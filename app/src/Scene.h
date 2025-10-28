#pragma once
#include "glm/vec3.hpp"
#include "engine/resources/ResourcesController.hpp"
#include <vector>
#include <engine/resources/SceneModel.hpp>

class Scene {
public:
    struct Light {
        glm::vec3 color;
    };

    struct LightAttenuation {
        float constant;
        float linear;
        float quadratic;
    };

    struct AmbientLight : Light {

    };

    struct PointLight : Light {
        glm::vec3 position;
        bool render_debug_cube = true;
    };

    struct DirectionalLight : Light {
        glm::vec3 direction;
    };

    void AddModel(const std::shared_ptr<engine::resources::SceneModel> &model);
    void AddLight(const PointLight &light);
    void AddLight(const DirectionalLight &light);

    void SetAmbientLight(const AmbientLight &light);

    void RenderModels() const;
    void RenderLights() const;

    std::vector<Scene::PointLight> GetAllPointLights() const;

private:
    engine::resources::Shader * SetupMainShader() const;

    static constexpr int MaxPointLights = 16;

    AmbientLight ambient_light = {glm::vec3(0.01f, 0.01f, 0.01f)};
    std::vector<PointLight> staticPointLights;
    std::vector<DirectionalLight> directionalLights;
    std::vector<std::shared_ptr<engine::resources::SceneModel>> models;

    LightAttenuation lightAttenuationParams = {1.0f, 0.09f, 0.032f};
};
