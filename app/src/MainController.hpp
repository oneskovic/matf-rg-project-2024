#pragma once
#include "Scene.h"
#include "engine/graphics/MSAAHandler.hpp"
#include "engine/resources/Model.hpp"
#include "engine/resources/Shader.hpp"
#include <engine/core/Controller.hpp>
#include <memory>
#include <random>
#include <engine/resources/SceneModel.hpp>

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
    engine::resources::Model *get_random_leaf_model();

    void generate_trees(int n);
    void update() override;

private:
    void generate_leaves_around_tree(float tree_x, float tree_y, int n);
    void generate_random_leaf_piles(int n);
    void make_random_falling_leaf(glm::vec3 start_pos);
    void make_random_falling_leaves();

    void update_camera();
    void update_light();
    void render_skybox();
    std::unique_ptr<Scene> scene;
    std::unique_ptr<engine::graphics::MSAAHandler> msaa_handler;
    std::mt19937 rng;
    std::vector<std::shared_ptr<engine::resources::SceneModel>> trees;

    // random scene generation params
    float leaf_spawn_prob_per_tick = 0.001;
};