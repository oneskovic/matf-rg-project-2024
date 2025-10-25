#pragma once
#include "engine/graphics/MSAAHandler.hpp"
#include "engine/resources/Model.hpp"
#include "engine/resources/Shader.hpp"


#include <engine/core/Controller.hpp>
#include <memory>
#include <random>

class Shader;

struct TreeModel {
    glm::vec3 position;
    engine::resources::Model* model;
};

class MainController final : public engine::core::Controller {
public:
    MainController() = default;
    ~MainController() override = default;

protected:
    void initialize() override;
    void begin_draw() override;
    void end_draw() override;
    void draw() override;
    void generate_trees(int n);
    void render_trees();
    void update() override;

private:
    void update_camera();
    void update_light();
    void render_light();
    void render_skybox();
    glm::vec3 light_position;
    glm::vec3 light_color;
    glm::mat4 lamp_post_model_matrix;
    engine::resources::Shader *shader;
    engine::resources::Model *triangle_model;
    std::unique_ptr<engine::graphics::MSAAHandler> msaa_handler;
    std::vector<TreeModel> trees;
    std::mt19937 rng;
};