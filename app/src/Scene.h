#pragma once
#include "engine/resources/Model.hpp"
#include "glm/vec3.hpp"
#include "engine/resources/ResourcesController.hpp"
#include <vector>

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
    };

    struct DirectionalLight : Light {
        glm::vec3 direction;
    };

    class Model {
    public:
        Model(float scale, glm::vec3 position, engine::resources::Model* model, glm::mat4 rotation_mat = glm::mat4(1.0f), float textureTileFactor = 1)
            : scale(scale)
            , position(position)
            , rotation_mat(rotation_mat)
            , model(model),
            textureTileFactor(textureTileFactor){
        }
        float scale;
        glm::vec3 position;
        glm::mat4 rotation_mat;
        engine::resources::Model* model;
        float textureTileFactor = 1;
    };

    void AddModel(const std::shared_ptr<Model> &model);
    void AddLight(const PointLight &light);
    void AddLight(const DirectionalLight &light);

    void SetAmbientLight(const AmbientLight &light);
    void MoveMovableLight(const glm::vec3 &position_delta);

    void RenderModels() const;
    void RenderLights() const;

private:
    engine::resources::Shader * SetupMainShader() const;

    static constexpr int MaxPointLights = 16;

    PointLight movable_light = {glm::vec3(0.67f, 0.55f, 0.18f), glm::vec3(1.0f,0.5f,0.0f)};
    AmbientLight ambient_light = {glm::vec3(0.01f, 0.01f, 0.01f)};
    std::vector<PointLight> staticPointLights;
    std::vector<DirectionalLight> directionalLights;
    std::vector<std::shared_ptr<Model>> models;

    LightAttenuation lightAttenuationParams = {1.0f, 0.09f, 0.032f};
};