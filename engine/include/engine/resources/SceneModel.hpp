#pragma once
#include <engine/resources/Model.hpp>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
namespace engine::resources {
    class SceneModel {
    public:
        SceneModel(float scale, glm::vec3 position, Model* model, glm::mat4 rotation_mat = glm::mat4(1.0f), float texture_tile_factor = 1, bool is_emissive = false, glm::vec3 emissive_color = glm::vec3(0))
            : scale(scale)
            , position(position)
            , rotation_mat(rotation_mat)
            , model(model)
            , texture_tile_factor(texture_tile_factor)
            , is_emissive(is_emissive)
            , emissive_color(emissive_color){
        }
        float scale;
        glm::vec3 position;
        glm::mat4 rotation_mat;
        Model* model;
        float texture_tile_factor = 1;
        bool is_emissive;
        glm::vec3 emissive_color;
    };
}