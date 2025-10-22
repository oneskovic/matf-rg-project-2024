#ifndef MATF_RG_PROJECT_MSAAHANDLER_H
#define MATF_RG_PROJECT_MSAAHANDLER_H
#include "engine/resources/ResourcesController.hpp"
#include <engine/resources/Shader.hpp>

namespace engine::graphics {
class MSAAHandler {
public:
    int init_msaa(int SCR_WIDTH, int SCR_HEIGHT);
    void init_bloom();
    void bloom_bright_pass() const;
    void bloom_blur_passes();
    void msaa_redirect() const;
    void blit_framebuffer_to_intermediate() const;
    void msaa_draw() const;

private:
    int SCR_WIDTH, SCR_HEIGHT;
    // For hdr
    float exposure = 0.8f;
    float gamma = 2.2f;

    // For msaa
    unsigned int framebuffer;
    unsigned int intermediateFBO;
    unsigned int quadVAO;
    unsigned int screenTexture;
    resources::Shader *screenShader;

    // For bloom
    unsigned int pingFBO[2];
    unsigned int pingTex[2];
    unsigned int brightFBO, brightTex;
    resources::Shader *brightShader, *blurShader;
    float bloomThreshold = 1.0f;
    float bloomIntensity = 0.7f;
    int blurPasses = 8;
    unsigned int lastBloomTexture;
};
}

#endif//MATF_RG_PROJECT_MSAAHANDLER_H
