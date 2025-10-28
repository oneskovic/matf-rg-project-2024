#include "MainController.hpp"

#include "AnimationController.h"
#include "engine/graphics/Camera.hpp"
#include "engine/graphics/GraphicsController.hpp"
#include "engine/platform/PlatformController.hpp"
#include "engine/resources/ResourcesController.hpp"
#include "spdlog/spdlog.h"

#include <engine/graphics/OpenGL.hpp>
#include <memory>

using namespace engine::core;
using namespace engine::graphics;

void MainController::initialize() {
    OpenGL::enable_depth_testing();
    scene = std::make_unique<Scene>();

    // Add lights to the scene
    // Scene::PointLight point_light = {glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(3.0f, 0.5f, 2.75f)};
    // scene->AddLight(point_light);
    // Scene::PointLight point_light2 = {glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.5f, -2.75f)};
    // scene->AddLight(point_light2);
    Scene::DirectionalLight directional_light = {glm::vec3(0.5f, 0.5f, 0.5f), glm::normalize(glm::vec3(0.5f, 1.0f, 0.5f))};
    scene->AddLight(directional_light);

    // Add models to the scene

    glm::vec3 lamp_pos = glm::vec3(3.0f,0,3.0f);
    auto lamp_post_model = get<ResourcesController>()->model("lamp_post");
    auto lamp_post = std::make_shared<SceneModel>(0.1, lamp_pos,  lamp_post_model, glm::mat4(1),1,true, glm::vec3(0.67f, 0.55f, 0.18f));
    scene->AddModel(lamp_post);

    glm::vec3 tent_pos = glm::vec3(2.0f,0,2.0f);
    auto tent_model = get<ResourcesController>()->model("tent");
    auto tent = std::make_shared<SceneModel>(0.25, tent_pos,  tent_model, glm::mat4(1),1,false);
    scene->AddModel(tent);

    glm::vec3 lantern_pos = glm::vec3(1.5f,0.2,1.5f);
    auto lantern_model = get<ResourcesController>()->model("lantern");
    lantern = std::make_shared<SceneModel>(0.3, lantern_pos,  lantern_model, glm::mat4(1),1,true, glm::vec3(0.67f, 0.55f, 0.18f));
    scene->AddModel(lantern);

    glm::vec3 ground_pos = glm::vec3(0.0f, -0.01f, 0.0f);
    auto ground_model = get<ResourcesController>()->model("ground");
    auto ground = std::make_shared<SceneModel>(10, ground_pos, ground_model, glm::mat4(1), 10);
    scene->AddModel(ground);

    msaa_handler = std::make_unique<MSAAHandler>();
    msaa_handler->init_msaa(800, 600);
    rng = std::mt19937(42);
    generate_trees(tree_count);
    generate_rocks(rock_count);
    generate_random_leaf_piles(leaf_pile_count);
    make_random_falling_leaves();

    // Setup the camera
    auto camera = get<GraphicsController>()->camera();
    camera->Position = glm::vec3(1.67, 0.6, 4);
}

void MainController::begin_draw() {
    OpenGL::clear_buffers();
    msaa_handler->msaa_redirect();
}

void MainController::draw() {
    scene->RenderModels();
    scene->RenderLights();
    render_skybox();
}

glm::vec2 MainController::random_point_in_ring(float center_x, float center_y, float radius_inner, float radius_outer) {
    glm::vec2 center = glm::vec2(center_x, center_y);
    glm::vec2 point;
    std::uniform_real_distribution dist_x(center_x-radius_outer, center_x+radius_outer);
    std::uniform_real_distribution dist_y(center_y-radius_outer, center_y+radius_outer);
    do {
        point = glm::vec2(dist_x(rng), dist_y(rng));
    } while (glm::distance(point, center) > radius_outer && glm::distance(point, center) < radius_inner);
    return point;
}

Model *MainController::get_random_leaf_model() {
    int different_leaves = 3;
    std::uniform_int_distribution dist_type(1, different_leaves);
    int leaf_model_index = dist_type(rng);
    Model *leaf_model = get<ResourcesController>()->model("leaf" + std::to_string(leaf_model_index));
    return leaf_model;
}

void MainController::generate_leaves_around_tree(float tree_x, float tree_z, int n)
{
    for (int i = 0; i < n; i++) {
        glm::vec2 leaf_pos = random_point_in_ring(tree_x, tree_z, 0.1f, 0.4f);
        auto leaf_model = get_random_leaf_model();
        auto leaf = std::make_shared<SceneModel>(0.1,glm::vec3(leaf_pos.x,0.01f,leaf_pos.y),  leaf_model);
        scene->AddModel(leaf);
    }
}

void MainController::make_random_falling_leaf(glm::vec3 start_pos) {
    auto leaf_model = get_random_leaf_model();
    auto leaf = std::make_shared<SceneModel>(0.1, start_pos, leaf_model);
    scene->AddModel(leaf);
    auto animation_controller = get<AnimationController>();
    animation_controller->animate_leaf(leaf, rng);
}

void MainController::make_random_falling_leaves() {
    std::uniform_int_distribution n_leaves_dist(0, max_leaves_per_tree);
    std::uniform_real_distribution start_height_dist(3.0f, 6.0f);
    for (const auto& tree: trees) {
        int n = n_leaves_dist(rng);
        for (int i = 0; i < n; i++) {
            glm::vec2 start_pos_xz = random_point_in_ring(tree->position.x, tree->position.z, 0.1f, 1.0f);
            glm::vec3 start_pos = glm::vec3(start_pos_xz.x, start_height_dist(rng), start_pos_xz.y);
            make_random_falling_leaf(start_pos);
        }
    }
}

void MainController::generate_random_leaf_piles(int n) {
    float min = -5.0f;
    float max = 5.0f;
    int differentLeafPiles = 2;

    std::uniform_real_distribution dist_pos(min, max);
    std::uniform_int_distribution dist_type(1, differentLeafPiles);

    for (int i = 0; i < n; i++) {
        float x = dist_pos(rng);
        float z = dist_pos(rng);

        int model_index = dist_type(rng);
        Model *model = get<ResourcesController>()->model("leaf_group" + std::to_string(model_index));
        auto model_ptr = std::make_shared<SceneModel>(0.1,glm::vec3(x,0.01,z),  model);
    }
}

void MainController::generate_trees(int n) {
    int differentTrees = 9;

    std::uniform_real_distribution dist_pos(object_min_pos, object_max_pos);
    std::uniform_int_distribution dist_type(1, differentTrees);

    for (int i = 0; i < n; i++) {
        float x = dist_pos(rng);
        float z = dist_pos(rng);

        int tree_model_index = dist_type(rng);
        Model *tree_model = get<ResourcesController>()->model("tree" + std::to_string(tree_model_index));
        auto tree = std::make_shared<SceneModel>(1,glm::vec3(x,0,z),  tree_model);
        scene->AddModel(tree);
        trees.push_back(tree);

        generate_leaves_around_tree(x,z,30);
    }
}

void MainController::generate_rocks(int n) {
    int differentRocks = 5;

    std::uniform_real_distribution dist_pos(object_min_pos, object_max_pos);
    std::uniform_int_distribution dist_type(1, differentRocks);

    for (int i = 0; i < n; i++) {
        float x = dist_pos(rng);
        float z = dist_pos(rng);

        int rock_model_index = dist_type(rng);
        Model *model = get<ResourcesController>()->model("rock" + std::to_string(rock_model_index));
        auto rock = std::make_shared<SceneModel>(1,glm::vec3(x,0,z),  model);
        scene->AddModel(rock);
    }
}

void MainController::update() {
    update_camera();
    update_light();
    std::uniform_real_distribution should_spawn_leaves_dist(0.0f, 1.0f);
    if (should_spawn_leaves_dist(rng) <= leaf_spawn_prob_per_tick) {
        make_random_falling_leaves();
    }
}

void MainController::update_camera() {
    auto platform = get<engine::platform::PlatformController>();
    auto camera = get<GraphicsController>()->camera();
    float dt = platform->dt();
    if (platform->key(engine::platform::KEY_W)
                .state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(Camera::Movement::FORWARD, dt);
    }
    if (platform->key(engine::platform::KEY_S)
                .state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(Camera::Movement::BACKWARD, dt);
    }
    if (platform->key(engine::platform::KEY_A)
                .state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(Camera::Movement::LEFT, dt);
    }
    if (platform->key(engine::platform::KEY_D)
                .state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(Camera::Movement::RIGHT, dt);
    }
    auto mouse = platform->mouse();

    float dx = std::clamp(mouse.dx, -10.0f, 10.0f);
    float dy = std::clamp(mouse.dy, -10.0f, 10.0f);
    camera->rotate_camera(dx, dy);
    camera->zoom(mouse.scroll);
}
void MainController::update_light() {
    auto platform = get<engine::platform::PlatformController>();
    float dt = platform->dt();
    if (platform->key(engine::platform::KEY_UP)
                .state() == engine::platform::Key::State::Pressed) {
        lantern->position += glm::vec3(0,0,1)*dt;
                }
    if (platform->key(engine::platform::KEY_DOWN)
                .state() == engine::platform::Key::State::Pressed) {
        lantern->position += glm::vec3(0,0,-1)*dt;
                }
    if (platform->key(engine::platform::KEY_LEFT)
                .state() == engine::platform::Key::State::Pressed) {
        lantern->position += glm::vec3(1,0,0)*dt;
                }
    if (platform->key(engine::platform::KEY_RIGHT)
                .state() == engine::platform::Key::State::Pressed) {
        lantern->position += glm::vec3(-1,0,0)*dt;
                }
}

void MainController::render_skybox() {
    auto shader = get<ResourcesController>()->shader("skybox");
    auto skybox_cube = get<ResourcesController>()->skybox("skybox");
    get<GraphicsController>()->draw_skybox(shader, skybox_cube);
}

void MainController::end_draw() {
    msaa_handler->blit_framebuffer_to_intermediate();
    msaa_handler->bloom_bright_pass();
    msaa_handler->bloom_blur_passes();
    msaa_handler->msaa_draw();
    auto platform = get<engine::platform::PlatformController>();
    platform->swap_buffers();
}
