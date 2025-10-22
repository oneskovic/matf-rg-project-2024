#include "engine/graphics/MSAAHandler.hpp"

#include "engine/graphics/OpenGL.hpp"
#include "engine/util/Errors.hpp"
#include "engine/resources/ResourcesController.hpp"
#include "engine/core/Controller.hpp"
#include <engine/util/Utils.hpp>
#include <filesystem>
#include <glad/glad.h>
#include <iostream>
#include <stb_image.h>

namespace engine::graphics {
int MSAAHandler::init_msaa(int SCR_WIDTH, int SCR_HEIGHT) {
    this->SCR_WIDTH = SCR_WIDTH;
    this->SCR_HEIGHT = SCR_HEIGHT;

    screenShader = core::Controller::get<resources::ResourcesController>()->shader("msaa");
    brightShader = core::Controller::get<resources::ResourcesController>()->shader("bloom_bright");
    blurShader = core::Controller::get<resources::ResourcesController>()->shader("bloom_blur");


    float quadVertices[] = {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    // setup screen VAO
    unsigned int quadVBO;
    CHECKED_GL_CALL(glGenVertexArrays, 1, &quadVAO);
    CHECKED_GL_CALL(glGenBuffers, 1, &quadVBO);
    CHECKED_GL_CALL(glBindVertexArray, quadVAO);
    CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, quadVBO);
    CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    CHECKED_GL_CALL(glEnableVertexAttribArray, 0);
    CHECKED_GL_CALL(glVertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    CHECKED_GL_CALL(glEnableVertexAttribArray, 1);
    CHECKED_GL_CALL(glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // configure MSAA framebuffer
    // --------------------------

    CHECKED_GL_CALL(glGenFramebuffers, 1, &framebuffer);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, framebuffer);
    // create a multisampled color attachment texture
    unsigned int textureColorBufferMultiSampled;
    CHECKED_GL_CALL(glGenTextures, 1, &textureColorBufferMultiSampled);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    CHECKED_GL_CALL(glTexImage2DMultisample, GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D_MULTISAMPLE, 0);
    CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
    // create a (also multisampled) renderbuffer object for depth and stencil attachments
    unsigned int rbo;
    CHECKED_GL_CALL(glGenRenderbuffers, 1, &rbo);
    CHECKED_GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, rbo);
    CHECKED_GL_CALL(glRenderbufferStorageMultisample, GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    CHECKED_GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, 0);
    CHECKED_GL_CALL(glFramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (CHECKED_GL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw util::EngineError(util::EngineError::Type::OpenGLError, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
    }
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);

    // configure second post-processing framebuffer
    CHECKED_GL_CALL(glGenFramebuffers, 1, &intermediateFBO);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, intermediateFBO);
    // create a color attachment texture
    CHECKED_GL_CALL(glGenTextures, 1, &screenTexture);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, screenTexture);
    CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0); // we only need a color buffer

    if (CHECKED_GL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw util::EngineError(util::EngineError::Type::OpenGLError, "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!");
    }
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);

    init_bloom();
    return framebuffer;
}

void MSAAHandler::init_bloom() {
    // Bright-pass FBO and texture
    CHECKED_GL_CALL(glGenFramebuffers, 1, &brightFBO);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, brightFBO);

    CHECKED_GL_CALL(glGenTextures, 1, &brightTex);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, brightTex);
    CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brightTex, 0);
    RG_GUARANTEE(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE, "brightFBO incomplete");

    // Ping-pong FBOs
    CHECKED_GL_CALL(glGenFramebuffers, 2, pingFBO);
    CHECKED_GL_CALL(glGenTextures, 2, pingTex);
    for (int i = 0; i < 2; ++i) {
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, pingFBO[i]);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, pingTex[i]);
        CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingTex[i], 0);
        RG_GUARANTEE(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE, "pingFBO incomplete");
    }

    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

void MSAAHandler::bloom_bright_pass() const {
    CHECKED_GL_CALL(glDisable, GL_DEPTH_TEST);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, brightFBO);
    CHECKED_GL_CALL(glViewport, 0, 0, SCR_WIDTH, SCR_HEIGHT);
    CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT);

    CHECKED_GL_CALL(glDisable, GL_FRAMEBUFFER_SRGB);

    brightShader->use();
    brightShader->set_int("hdrTex", 0);
    brightShader->set_float("threshold", bloomThreshold);

    CHECKED_GL_CALL(glBindVertexArray, quadVAO);
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, screenTexture);
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 6);
}

void MSAAHandler::bloom_blur_passes() {
    bool horizontal = true;
    GLuint src = brightTex;

    blurShader->use();
    blurShader->set_int("image", 0);
    for (int i = 0; i < blurPasses; i++) {
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, pingFBO[horizontal]);
        blurShader->set_int("horizontal", horizontal ? 1 : 0);

        CHECKED_GL_CALL(glBindVertexArray, quadVAO);
        CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, src);
        CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 6);

        src = pingTex[horizontal];
        horizontal = !horizontal;
    }

    lastBloomTexture = src;
}


void MSAAHandler::msaa_redirect() const {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, framebuffer);
    CHECKED_GL_CALL(glClearColor, 0.1f, 0.1f, 0.1f, 1.0f);
    CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECKED_GL_CALL(glEnable, GL_DEPTH_TEST);
}

void MSAAHandler::blit_framebuffer_to_intermediate() const {
    CHECKED_GL_CALL(glBindFramebuffer, GL_READ_FRAMEBUFFER, framebuffer);
    CHECKED_GL_CALL(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, intermediateFBO);
    CHECKED_GL_CALL(glBlitFramebuffer,
                    0, 0, SCR_WIDTH, SCR_HEIGHT,
                    0, 0, SCR_WIDTH, SCR_HEIGHT,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void MSAAHandler::msaa_draw() const {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
    CHECKED_GL_CALL(glClearColor, 1.0f, 1.0f, 1.0f, 1.0f);
    CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT);
    CHECKED_GL_CALL(glDisable, GL_DEPTH_TEST);
    CHECKED_GL_CALL(glDisable, GL_FRAMEBUFFER_SRGB);

    screenShader->use();
    // set hdr params
    screenShader->set_int("screenTexture", 0);
    screenShader->set_float("exposure", exposure);
    screenShader->set_float("gamma", gamma);
    // set bloom params
    screenShader->set_int("bloomTex", 1);
    screenShader->set_float("bloomIntensity", bloomIntensity);

    CHECKED_GL_CALL(glBindVertexArray, quadVAO);
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, screenTexture);
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE1);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, lastBloomTexture);
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 6);
}
}
