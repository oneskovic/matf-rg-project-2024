#include "MainController.hpp"

#include "engine/graphics/GraphicsController.hpp"
#include "engine/platform/PlatformController.hpp"
#include "engine/resources/ResourcesController.hpp"
#include <engine/graphics/OpenGL.hpp>

using namespace engine::core;
using namespace engine::graphics;

void MainController::initialize() {
    OpenGL::enable_depth_testing();
    light_position = glm::vec3(2.0f, 2.0f, 2.0f);
    light_color = glm::vec3(1.0f, 1.0f, 1.0f);
}

void MainController::begin_draw() {
    OpenGL::clear_buffers();
}

void MainController::draw() {
    auto bunny = get<engine::resources::ResourcesController>()->model("bunny");
    shader = get<engine::resources::ResourcesController>()->shader("triangle");
    auto graphics_controller = get<GraphicsController>();

    shader->use();
    shader->set_mat4("projection", graphics_controller->projection_matrix());
    shader->set_mat4("view", graphics_controller->camera()->view_matrix());
    auto model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    shader->set_mat4("model", model);

    shader->set_vec3("objectColor",  glm::vec3(1.0f, 0.5f, 0.3f));
    shader->set_vec3("ambientColor", glm::vec3(0.25f, 0.25f, 0.25f));

    // Directional light
    shader->set_vec3("dirLightDir",  glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f)));
    shader->set_vec3("dirLightColor",glm::vec3(1.0f, 1.0f, 1.0f));

    // Point light
    shader->set_vec3("pointLightPos",  light_position);
    shader->set_vec3("pointLightColor", light_color);

    // Attenuation
    shader->set_float("pointKc", 1.0f);
    shader->set_float("pointKl", 0.09f);
    shader->set_float("pointKq", 0.032f);

    bunny->draw(shader);
    render_light();
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
    camera->rotate_camera(mouse.dx, mouse.dy);
    camera->zoom(mouse.scroll);
}
void MainController::update_light() {
    auto platform = get<engine::platform::PlatformController>();
    float dt = platform->dt();
    if (platform->key(engine::platform::KEY_UP)
                .state() == engine::platform::Key::State::Pressed) {
        light_position += glm::vec3(0,1,0)*dt;
                }
    if (platform->key(engine::platform::KEY_DOWN)
                .state() == engine::platform::Key::State::Pressed) {
        light_position += glm::vec3(0,-1,0)*dt;
                }
    if (platform->key(engine::platform::KEY_LEFT)
                .state() == engine::platform::Key::State::Pressed) {
        light_position += glm::vec3(1,0,0)*dt;
                }
    if (platform->key(engine::platform::KEY_RIGHT)
                .state() == engine::platform::Key::State::Pressed) {
        light_position += glm::vec3(-1,0,0)*dt;
                }
}
void MainController::render_light() {
    auto cube = get<engine::resources::ResourcesController>()->model("cube");
    auto lightShader = get<engine::resources::ResourcesController>()->shader("light");
    auto graphics_controller = get<GraphicsController>();

    auto model = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
    model = glm::translate(model, light_position);

    lightShader->use();
    lightShader->set_mat4("model", model);
    lightShader->set_mat4("view", graphics_controller->camera()->view_matrix());
    lightShader->set_mat4("projection", graphics_controller->projection_matrix());
    lightShader->set_vec3("lightColor", light_color);
    cube->draw(lightShader);
}


void MainController::end_draw() {
    auto platform = get<engine::platform::PlatformController>();
    platform->swap_buffers();
}
