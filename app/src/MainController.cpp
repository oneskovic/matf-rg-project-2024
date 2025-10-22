#include "MainController.hpp"

#include "engine/graphics/GraphicsController.hpp"
#include "engine/platform/PlatformController.hpp"
#include "engine/resources/ResourcesController.hpp"
#include "spdlog/spdlog.h"

#include <engine/graphics/OpenGL.hpp>

using namespace engine::core;
using namespace engine::graphics;

void MainController::initialize() {
    OpenGL::enable_depth_testing();
    light_position = glm::vec3(3.0f, 0.5f, 2.75f);
    light_color = glm::vec3(1.0f, 1.0f, 1.0f);
    lamp_post_model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(3,0,3)), glm::vec3(0.1,0.1,0.1));
    msaa_handler = std::make_unique<MSAAHandler>();
    msaa_handler->init_msaa(800, 600);
}

void MainController::begin_draw() {
    OpenGL::clear_buffers();
    msaa_handler->msaa_redirect();
}

void MainController::draw() {
    auto bunnyModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    auto bunny = get<engine::resources::ResourcesController>()->model("bunny");
    auto lampPost = get<engine::resources::ResourcesController>()->model("lamp_post");
    auto debug = get<engine::resources::ResourcesController>()->model("BallAndBox");
    shader = get<engine::resources::ResourcesController>()->shader("triangle");
    auto graphics_controller = get<GraphicsController>();

    shader->use();
    shader->set_mat4("projection", graphics_controller->projection_matrix());
    shader->set_mat4("view", graphics_controller->camera()->view_matrix());

    shader->set_vec3("objectColor",  glm::vec3(1.0f, 0.5f, 0.3f));
    shader->set_vec3("ambientColor", glm::vec3(0.05f, 0.05f, 0.05f));

    // Directional light
    shader->set_vec3("dirLightDir",  glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f)));
    shader->set_vec3("dirLightColor",glm::vec3(0.2f, 0.2f, 0.2f));

    // Point light
    shader->set_vec3("pointLightPos",  light_position);
    shader->set_vec3("pointLightColor", glm::vec3(1.0,0.0,0.0));

    // Attenuation
    shader->set_float("pointKc", 1.0f);
    shader->set_float("pointKl", 0.09f);
    shader->set_float("pointKq", 0.032f);

    shader->set_mat4("model", bunnyModelMatrix);
    bunny->draw(shader);

    shader->set_mat4("model", lamp_post_model_matrix);
    shader->set_int("uDiffuseMap", 0);
    lampPost->draw(shader);

    shader->set_mat4("model", glm::mat4(1.0f));
    shader->set_int("uDiffuseMap", 0);
    debug->draw(shader);

    render_light();
    render_skybox();
}

void MainController::update() {
    update_camera();
    update_light();
}

void MainController::update_camera() {
    auto platform = get<engine::platform::PlatformController>();
    auto camera = get<GraphicsController>()->camera();
    float dt = platform->dt();
    if (platform->key(engine::platform::KEY_W)
                .state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(Camera::Movement::FORWARD, dt);
    }
    if (platform->key(engine::platform::KEY_S)
                .state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(Camera::Movement::BACKWARD, dt);
    }
    if (platform->key(engine::platform::KEY_A)
                .state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(Camera::Movement::LEFT, dt);
    }
    if (platform->key(engine::platform::KEY_D)
                .state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(Camera::Movement::RIGHT, dt);
    }
    auto mouse = platform->mouse();

    float dx = std::clamp(mouse.dx, -10.0f, 10.0f);
    float dy = std::clamp(mouse.dy, -10.0f, 10.0f);
    camera->rotate_camera(dx, dy);
    camera->zoom(mouse.scroll);
}
void MainController::update_light() {
    auto platform = get<engine::platform::PlatformController>();
    float dt = platform->dt();
    if (platform->key(engine::platform::KEY_UP)
                .state() == engine::platform::Key::State::Pressed) {
        light_position += glm::vec3(0,0,1)*dt;
                }
    if (platform->key(engine::platform::KEY_DOWN)
                .state() == engine::platform::Key::State::Pressed) {
        light_position += glm::vec3(0,0,-1)*dt;
                }
    if (platform->key(engine::platform::KEY_LEFT)
                .state() == engine::platform::Key::State::Pressed) {
        light_position += glm::vec3(1,0,0)*dt;
                }
    if (platform->key(engine::platform::KEY_RIGHT)
                .state() == engine::platform::Key::State::Pressed) {
        light_position += glm::vec3(-1,0,0)*dt;
                }
    //spdlog::info("Light position: {} {} {}", light_position.x, light_position.y, light_position.z);
}
void MainController::render_light() {
    auto cube = get<engine::resources::ResourcesController>()->model("cube");
    auto lightShader = get<engine::resources::ResourcesController>()->shader("light");
    auto graphics_controller = get<GraphicsController>();

    auto model = glm::translate(glm::mat4(1.0f), light_position);
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

    lightShader->use();
    lightShader->set_mat4("model", model);
    lightShader->set_mat4("view", graphics_controller->camera()->view_matrix());
    lightShader->set_mat4("projection", graphics_controller->projection_matrix());
    lightShader->set_vec3("lightColor", light_color);
    cube->draw(lightShader);
}

void MainController::render_skybox() {
    auto shader = get<engine::resources::ResourcesController>()->shader("skybox");
    auto skybox_cube = get<engine::resources::ResourcesController>()->skybox("skybox");
    get<GraphicsController>()->draw_skybox(shader, skybox_cube);
}


void MainController::end_draw() {
    msaa_handler->blit_framebuffer_to_intermediate();
    msaa_handler->bloom_bright_pass();
    msaa_handler->bloom_blur_passes();
    msaa_handler->msaa_draw();
    auto platform = get<engine::platform::PlatformController>();
    platform->swap_buffers();
}
