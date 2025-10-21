#ifndef MATF_RG_PROJECT_MSAAHANDLER_H
#define MATF_RG_PROJECT_MSAAHANDLER_H
#include "engine/resources/ResourcesController.hpp"


#include <engine/resources/Shader.hpp>

namespace engine::graphics {
class MSAAHandler {
public:
    int init_msaa(int SCR_WIDTH, int SCR_HEIGHT);
    void msaa_redirect() const;
    void blit_framebuffer_to_intermediate() const;
    void msaa_draw() const;

private:
    int SCR_WIDTH, SCR_HEIGHT;
    unsigned int framebuffer;
    unsigned int intermediateFBO;
    unsigned int quadVAO;
    unsigned int screenTexture;
    resources::Shader *screenShader;

};
}

#endif//MATF_RG_PROJECT_MSAAHANDLER_H
