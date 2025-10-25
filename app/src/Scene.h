#pragma once
#include "engine/resources/Model.hpp"
#include "glm/vec3.hpp"
#include "engine/resources/ResourcesController.hpp"

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
    struct PointLight : Light {
        glm::vec3 position;
    };
    struct DirectionalLight : Light {
        glm::vec3 direction;
    };
    struct Model {
        float scale;
        glm::vec3 position;
        engine::resources::Model* model;
        float textureTileFactor = 1;
    };
    void AddModel(const Model &model);
    void AddLight(const PointLight &light);
    void AddLight(const DirectionalLight &light);
    void RenderModels() const;
    void RenderLights() const;
private:
    engine::resources::Shader * SetupMainShader() const;
    std::vector<PointLight> pointLights;
    std::vector<DirectionalLight> directionalLights;
    std::vector<Model> models;
    LightAttenuation lightAttenuationParams = {1, 0.09f, 0.032f};
};
