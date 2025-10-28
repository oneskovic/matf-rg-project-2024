#include "engine/graphics/MSAAHandler.hpp"

#include "engine/graphics/OpenGL.hpp"
#include "engine/util/Errors.hpp"
#include "engine/resources/ResourcesController.hpp"
#include "engine/core/Controller.hpp"
#include <engine/util/Utils.hpp>
#include <filesystem>
#include <glad/glad.h>
#include <stb_image.h>

namespace engine::graphics {
int MSAAHandler::init_msaa(int scr_width, int scr_height) {
    this->m_scr_width = scr_width;
    this->m_scr_height = scr_height;

    m_screen_shader = core::Controller::get<resources::ResourcesController>()->shader("msaa");
    m_bright_shader = core::Controller::get<resources::ResourcesController>()->shader("bloom_bright");
    m_blur_shader = core::Controller::get<resources::ResourcesController>()->shader("bloom_blur");


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
    CHECKED_GL_CALL(glGenVertexArrays, 1, &m_quad_vao);
    CHECKED_GL_CALL(glGenBuffers, 1, &quadVBO);
    CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
    CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, quadVBO);
    CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    CHECKED_GL_CALL(glEnableVertexAttribArray, 0);
    CHECKED_GL_CALL(glVertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    CHECKED_GL_CALL(glEnableVertexAttribArray, 1);
    CHECKED_GL_CALL(glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // configure MSAA framebuffer
    // --------------------------

    CHECKED_GL_CALL(glGenFramebuffers, 1, &m_framebuffer);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_framebuffer);
    // create a multisampled color attachment texture
    unsigned int textureColorBufferMultiSampled;
    CHECKED_GL_CALL(glGenTextures, 1, &textureColorBufferMultiSampled);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    CHECKED_GL_CALL(glTexImage2DMultisample, GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F, scr_width, scr_height, GL_TRUE);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D_MULTISAMPLE, 0);
    CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
    // create a (also multisampled) renderbuffer object for depth and stencil attachments
    unsigned int rbo;
    CHECKED_GL_CALL(glGenRenderbuffers, 1, &rbo);
    CHECKED_GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, rbo);
    CHECKED_GL_CALL(glRenderbufferStorageMultisample, GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, scr_width, scr_height);
    CHECKED_GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, 0);
    CHECKED_GL_CALL(glFramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (CHECKED_GL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw util::EngineError(util::EngineError::Type::OpenGLError, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
    }
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);

    // configure second post-processing framebuffer
    CHECKED_GL_CALL(glGenFramebuffers, 1, &m_intermediate_fbo);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_intermediate_fbo);
    // create a color attachment texture
    CHECKED_GL_CALL(glGenTextures, 1, &m_screen_texture);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_screen_texture);
    CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA16F, scr_width, scr_height, 0, GL_RGBA, GL_FLOAT, nullptr);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_screen_texture, 0); // we only need a color buffer

    if (CHECKED_GL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw util::EngineError(util::EngineError::Type::OpenGLError, "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!");
    }
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);

    init_bloom();
    return m_framebuffer;
}

void MSAAHandler::init_bloom() {
    // Bright-pass FBO and texture
    CHECKED_GL_CALL(glGenFramebuffers, 1, &m_bright_fbo);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_bright_fbo);

    CHECKED_GL_CALL(glGenTextures, 1, &m_bright_tex);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_bright_tex);
    CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA16F, m_scr_width, m_scr_height, 0, GL_RGBA, GL_FLOAT, nullptr);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_bright_tex, 0);
    RG_GUARANTEE(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE, "brightFBO incomplete");

    // Ping-pong FBOs
    CHECKED_GL_CALL(glGenFramebuffers, 2, m_ping_fbo);
    CHECKED_GL_CALL(glGenTextures, 2, m_ping_tex);
    for (int i = 0; i < 2; ++i) {
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_ping_fbo[i]);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_ping_tex[i]);
        CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA16F, m_scr_width, m_scr_height, 0, GL_RGBA, GL_FLOAT, nullptr);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ping_tex[i], 0);
        RG_GUARANTEE(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE, "pingFBO incomplete");
    }

    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

void MSAAHandler::bloom_bright_pass() const {
    CHECKED_GL_CALL(glDisable, GL_DEPTH_TEST);
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_bright_fbo);
    CHECKED_GL_CALL(glViewport, 0, 0, m_scr_width, m_scr_height);
    CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT);

    CHECKED_GL_CALL(glDisable, GL_FRAMEBUFFER_SRGB);

    m_bright_shader->use();
    m_bright_shader->set_int("hdrTex", 0);
    m_bright_shader->set_float("threshold", m_bloom_threshold);

    CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_screen_texture);
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 6);
}

void MSAAHandler::bloom_blur_passes() {
    bool horizontal = true;
    GLuint src = m_bright_tex;

    m_blur_shader->use();
    m_blur_shader->set_int("image", 0);
    for (int i = 0; i < m_blur_passes; i++) {
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_ping_fbo[horizontal]);
        m_blur_shader->set_int("horizontal", horizontal ? 1 : 0);

        CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
        CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, src);
        CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 6);

        src = m_ping_tex[horizontal];
        horizontal = !horizontal;
    }

    m_last_bloom_texture = src;
}


void MSAAHandler::msaa_redirect() const {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_framebuffer);
    CHECKED_GL_CALL(glClearColor, 0.1f, 0.1f, 0.1f, 1.0f);
    CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECKED_GL_CALL(glEnable, GL_DEPTH_TEST);
}

void MSAAHandler::blit_framebuffer_to_intermediate() const {
    CHECKED_GL_CALL(glBindFramebuffer, GL_READ_FRAMEBUFFER, m_framebuffer);
    CHECKED_GL_CALL(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, m_intermediate_fbo);
    CHECKED_GL_CALL(glBlitFramebuffer,
                    0, 0, m_scr_width, m_scr_height,
                    0, 0, m_scr_width, m_scr_height,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void MSAAHandler::msaa_draw() const {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
    CHECKED_GL_CALL(glClearColor, 1.0f, 1.0f, 1.0f, 1.0f);
    CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT);
    CHECKED_GL_CALL(glDisable, GL_DEPTH_TEST);
    CHECKED_GL_CALL(glDisable, GL_FRAMEBUFFER_SRGB);

    m_screen_shader->use();
    // set hdr params
    m_screen_shader->set_int("screenTexture", 0);
    m_screen_shader->set_float("exposure", m_exposure);
    m_screen_shader->set_float("gamma", m_gamma);
    // set bloom params
    m_screen_shader->set_int("bloomTex", 1);
    m_screen_shader->set_float("bloomIntensity", m_bloom_intensity);

    CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_screen_texture);
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE1);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_last_bloom_texture);
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 6);
}
}
