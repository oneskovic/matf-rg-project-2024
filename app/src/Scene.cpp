#include "Scene.h"

#include "engine/graphics/GraphicsController.hpp"
using namespace engine::core;
using namespace engine::graphics;

void Scene::AddModel(const std::shared_ptr<engine::resources::SceneModel> &model) {
    models.push_back(model);
}

void Scene::AddLight(const PointLight &light) {
    staticPointLights.push_back(light);
}

void Scene::AddLight(const DirectionalLight &light) {
    directionalLights.push_back(light);
}
void Scene::SetAmbientLight(const AmbientLight &light) {
    ambient_light = light;
}

void Scene::RenderModels() const {
    engine::resources::Shader * main_shader = SetupMainShader();

    for (const auto& model : models) {
        auto model_matrix = glm::translate(glm::mat4(1.0f), model->position);
        model_matrix = model_matrix * model->rotation_mat;
        model_matrix = glm::scale(model_matrix, glm::vec3(model->scale, model->scale, model->scale));

        main_shader->set_mat4("model", model_matrix);
        main_shader->set_float("uTile", model->texture_tile_factor);
        main_shader->set_bool("useEmissive", model->is_emissive);
        model->model->draw(main_shader);
    }
}

void Scene::RenderLights() const {
    auto lightShader = Controller::get<engine::resources::ResourcesController>()->shader("light");
    auto graphics_controller = Controller::get<GraphicsController>();
    lightShader->use();

    auto cube = Controller::get<engine::resources::ResourcesController>()->model("cube");

    auto all_point_lights = GetAllPointLights();
    for (const auto& light : all_point_lights) {
        if (!light.render_debug_cube) {
            continue;
        }
        auto model = glm::translate(glm::mat4(1.0f), light.position);
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

        lightShader->set_mat4("model", model);
        lightShader->set_mat4("view", graphics_controller->camera()->view_matrix());
        lightShader->set_mat4("projection", graphics_controller->projection_matrix());
        lightShader->set_vec3("lightColor", light.color);

        cube->draw(lightShader);
    }
}

std::vector<Scene::PointLight> Scene::GetAllPointLights() const {
    auto all_point_lights = staticPointLights;
    // A hack to not have to update both the light position and the model position instead someone can move an emissive model and it will move the light as well
    for (const auto& model : models) {
        if (model->is_emissive) {
            PointLight point_light = {model->emissive_color, model->position, false};
            all_point_lights.push_back(point_light);
        }
    }
    return all_point_lights;
}

engine::resources::Shader *Scene::SetupMainShader() const {
    auto shader = Controller::get<engine::resources::ResourcesController>()->shader("triangle");
    auto graphics_controller = Controller::get<GraphicsController>();

    shader->use();

    shader->set_mat4("projection", graphics_controller->projection_matrix());
    shader->set_mat4("view", graphics_controller->camera()->view_matrix());

    // ambient light
    shader->set_vec3("ambientLight", ambient_light.color);

    // directional light
    if (!directionalLights.empty()) {
        const auto& d = directionalLights[0];
        shader->set_vec3("dirLightDir",   glm::normalize(d.direction));
        shader->set_vec3("dirLightColor", d.color);
    } else {
        shader->set_vec3("dirLightDir",   glm::vec3(0.0f, -1.0f, 0.0f));
        shader->set_vec3("dirLightColor", glm::vec3(0.0f));
    }

    auto all_point_lights = GetAllPointLights();
    int numPointLights = static_cast<int>(all_point_lights.size());
    if (numPointLights > MaxPointLights) {
        throw std::runtime_error("Too many PointLights");
    }
    shader->set_int("uNumPointLights", numPointLights);

    for (int i = 0; i < numPointLights; i++) {
        const auto& pl = all_point_lights[i];
        shader->set_vec3("uPointLights[" + std::to_string(i) + "].position", pl.position);
        shader->set_vec3("uPointLights[" + std::to_string(i) + "].color", pl.color);
    }

    // Set light attenuation params
    shader->set_float("pointKc", lightAttenuationParams.constant);
    shader->set_float("pointKl", lightAttenuationParams.linear);
    shader->set_float("pointKq", lightAttenuationParams.quadratic);

    return shader;
}