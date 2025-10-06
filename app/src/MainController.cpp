#include "MainController.hpp"

#include "engine/graphics/GraphicsController.hpp"
#include "engine/platform/PlatformController.hpp"
#include "engine/resources/ResourcesController.hpp"
#include <engine/graphics/OpenGL.hpp>

using namespace engine::core;
using namespace engine::graphics;

void MainController::initialize() {
    OpenGL::enable_depth_testing();
}

void MainController::begin_draw() {
    OpenGL::clear_buffers();
}

void MainController::draw() {
    triangle_model = get<engine::resources::ResourcesController>()->model("triangle");
    auto bunny = get<engine::resources::ResourcesController>()->model("bunny");
    shader = get<engine::resources::ResourcesController>()->shader("triangle");
    auto graphics_controller = get<GraphicsController>();

    shader->use();
    shader->set_mat4("projection", graphics_controller->projection_matrix());
    shader->set_mat4("view", graphics_controller->camera()->view_matrix());
    auto model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    shader->set_mat4("model", model);
    bunny->draw(shader);
}
void MainController::update() {
    update_camera();
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

void MainController::end_draw() {
    auto platform = get<engine::platform::PlatformController>();
    platform->swap_buffers();
}
