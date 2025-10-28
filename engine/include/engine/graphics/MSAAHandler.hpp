#ifndef MATF_RG_PROJECT_MSAAHANDLER_H
#define MATF_RG_PROJECT_MSAAHANDLER_H
#include "engine/resources/ResourcesController.hpp"

namespace engine::graphics {
    class MSAAHandler {
    public:
        int init_msaa(int scr_width, int scr_height);

        void init_bloom();

        void bloom_bright_pass() const;

        void bloom_blur_passes();

        void msaa_redirect() const;

        void blit_framebuffer_to_intermediate() const;

        void msaa_draw() const;

    private:
        int m_scr_width, m_scr_height;
        // For hdr
        float m_exposure = 0.8f;
        float m_gamma    = 2.2f;

        // For msaa
        unsigned int m_framebuffer;
        unsigned int m_intermediate_fbo;
        unsigned int m_quad_vao;
        unsigned int m_screen_texture;
        resources::Shader *m_screen_shader;

        // For bloom
        unsigned int m_ping_fbo[2];
        unsigned int m_ping_tex[2];
        unsigned int m_bright_fbo, m_bright_tex;
        resources::Shader *m_bright_shader, *m_blur_shader;
        float m_bloom_threshold = 1.0f;
        float m_bloom_intensity = 0.7f;
        int m_blur_passes       = 8;
        unsigned int m_last_bloom_texture;
    };
}

#endif//MATF_RG_PROJECT_MSAAHANDLER_H
