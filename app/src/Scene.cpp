
#include "Scene.h"

#include "engine/core/App.hpp"
#include "engine/graphics/GraphicsController.hpp"
using namespace engine::core;
using namespace engine::graphics;

void Scene::AddModel(const Model &model) {
    models.push_back(model);
}
void Scene::AddLight(const PointLight &light) {
    pointLights.push_back(light);
}
void Scene::AddLight(const DirectionalLight &light) {
    directionalLights.push_back(light);
}
void Scene::RenderModels() const {
    engine::resources::Shader * main_shader = SetupMainShader();
    for (const auto& model : models) {
        auto model_matrix =
            glm::scale(
                glm::translate(glm::mat4(1.0f), model.position),
                glm::vec3(model.scale,model.scale,model.scale));
        main_shader->set_mat4("model", model_matrix);
        main_shader->set_float("uTile", model.textureTileFactor);
        //main_shader->set_int("uDiffuseMap", 0);
        model.model->draw(main_shader);
    }
}
void Scene::RenderLights() const {
    auto lightShader = Controller::get<engine::resources::ResourcesController>()->shader("light");
    auto graphics_controller = Controller::get<GraphicsController>();
    lightShader->use();

    for (const auto& light : pointLights) {
        auto cube = Controller::get<engine::resources::ResourcesController>()->model("cube");

        auto model = glm::translate(glm::mat4(1.0f), light.position);
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

        lightShader->set_mat4("model", model);
        lightShader->set_mat4("view", graphics_controller->camera()->view_matrix());
        lightShader->set_mat4("projection", graphics_controller->projection_matrix());
        lightShader->set_vec3("lightColor", light.color);
        cube->draw(lightShader);
    }
}
engine::resources::Shader *Scene::SetupMainShader() const {
    auto shader = Controller::get<engine::resources::ResourcesController>()->shader("triangle");
    auto graphics_controller = Controller::get<GraphicsController>();

    shader->use();
    shader->set_float("uTile", 1.0f);

    shader->set_mat4("projection", graphics_controller->projection_matrix());
    shader->set_mat4("view", graphics_controller->camera()->view_matrix());

    shader->set_vec3("objectColor",  glm::vec3(1.0f, 1.0f, 1.0f));
    shader->set_vec3("ambientColor", glm::vec3(0.05f, 0.05f, 0.05f));

    // Directional light
    auto light_direction = directionalLights[0].direction;
    auto light_color = directionalLights[0].color;
    shader->set_vec3("dirLightDir",  glm::normalize(light_direction));
    shader->set_vec3("dirLightColor",light_color);

    // Point light
    auto light_position = pointLights[0].position;
    auto point_light_color = pointLights[0].color;
    shader->set_vec3("pointLightPos", light_position);
    shader->set_vec3("pointLightColor", point_light_color);

    // Attenuation
    shader->set_float("pointKc", lightAttenuationParams.constant);
    shader->set_float("pointKl", lightAttenuationParams.linear);
    shader->set_float("pointKq", lightAttenuationParams.quadratic);

    return shader;
}