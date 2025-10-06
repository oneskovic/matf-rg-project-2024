#include "MainController.hpp"

#include "engine/graphics/GraphicsController.hpp"
#include "engine/platform/PlatformController.hpp"
#include "engine/resources/ResourcesController.hpp"
#include <engine/graphics/OpenGL.hpp>

using namespace engine::core;
using namespace engine::graphics;

void MainController::initialize() {
    engine::graphics::OpenGL::enable_depth_testing();
}

void MainController::begin_draw() {
    engine::graphics::OpenGL::clear_buffers();
}

void MainController::draw() {
    triangle_model = get<engine::resources::ResourcesController>()->model("triangle");
    shader = get<engine::resources::ResourcesController>()->shader("triangle");
    auto graphics_controller = get<engine::graphics::GraphicsController>();

    shader->use();
    shader->set_mat4("projection", graphics_controller->projection_matrix());
    shader->set_mat4("view", graphics_controller->camera()->view_matrix());
    auto model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    shader->set_mat4("model", model);
    triangle_model->draw(shader);
}

void MainController::end_draw() {
    auto platform = get<engine::platform::PlatformController>();
    platform->swap_buffers();
}

