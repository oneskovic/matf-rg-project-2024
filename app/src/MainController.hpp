#pragma once
#include "Scene.h"
#include "engine/graphics/MSAAHandler.hpp"
#include "engine/resources/Model.hpp"
#include "engine/resources/Shader.hpp"
#include <engine/core/Controller.hpp>
#include <memory>
#include <random>

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
    glm::vec2 random_point_in_ring(float center_x, float center_y, float radius_inner, float radius_outer);
    void generate_leaves_around_tree(float tree_x, float tree_y, int n);
    void generate_random_leaf_piles(int n);
    void generate_trees(int n);
    void update() override;

private:
    void update_camera();
    void update_light();
    void render_skybox();
    std::unique_ptr<Scene> scene;
    std::unique_ptr<engine::graphics::MSAAHandler> msaa_handler;
    std::mt19937 rng;
};