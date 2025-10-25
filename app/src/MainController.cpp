#include "MainController.hpp"

#include "engine/graphics/Camera.hpp"
#include "engine/graphics/GraphicsController.hpp"
#include "engine/platform/PlatformController.hpp"
#include "engine/resources/ResourcesController.hpp"
#include "spdlog/spdlog.h"

#include <engine/graphics/OpenGL.hpp>

using namespace engine::core;
using namespace engine::graphics;

void MainController::initialize() {
    OpenGL::enable_depth_testing();
    scene = std::make_unique<Scene>();

    // Add lights to the scene
    Scene::PointLight point_light = {glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(3.0f, 0.5f, 2.75f)};
    scene->AddLight(point_light);
    // Scene::PointLight point_light2 = {glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.5f, -2.75f)};
    // scene->AddLight(point_light2);
    Scene::DirectionalLight directional_light = {glm::vec3(0.5f, 0.5f, 0.5f), glm::normalize(glm::vec3(0.5f, 1.0f, 0.5f))};
    scene->AddLight(directional_light);

    // Add models to the scene

    glm::vec3 lamp_pos = glm::vec3(3.0f,0,3.0f);
    auto lampPost = get<engine::resources::ResourcesController>()->model("lamp_post");
    scene->AddModel({0.1, lamp_pos, lampPost});

    glm::vec3 ground_pos = glm::vec3(0.0f, -0.01f, 0.0f);
    auto ground = get<engine::resources::ResourcesController>()->model("ground");
    scene->AddModel({10.0, ground_pos, ground, 10.0});

    msaa_handler = std::make_unique<MSAAHandler>();
    msaa_handler->init_msaa(800, 600);
    rng = std::mt19937(42);
    generate_trees(10);
    generate_random_leaf_piles(15);
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

void MainController::generate_leaves_around_tree(float tree_x, float tree_z, int n)
{
    int different_leaves = 3;
    std::uniform_int_distribution dist_type(1, different_leaves);
    for (int i = 0; i < n; i++) {
        glm::vec2 leaf_pos = random_point_in_ring(tree_x, tree_z, 0.1f, 0.4f);
        int leaf_model_index = dist_type(rng);
        engine::resources::Model *leaf_model = get<engine::resources::ResourcesController>()->model("leaf" + std::to_string(leaf_model_index));
        scene->AddModel({0.1,glm::vec3(leaf_pos.x,0.01f,leaf_pos.y), leaf_model});
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
        engine::resources::Model *model = get<engine::resources::ResourcesController>()->model("leaf_group" + std::to_string(model_index));
        scene->AddModel({0.1,glm::vec3(x,0.01,z), model});

    }
}

void MainController::generate_trees(int n) {
    float min = -5.0f;
    float max = 5.0f;
    int differentTrees = 9;

    std::uniform_real_distribution dist_pos(min, max);
    std::uniform_int_distribution dist_type(1, differentTrees);

    for (int i = 0; i < n; i++) {
        float x = dist_pos(rng);
        float z = dist_pos(rng);

        int tree_model_index = dist_type(rng);
        engine::resources::Model *tree_model = get<engine::resources::ResourcesController>()->model("tree" + std::to_string(tree_model_index));
        scene->AddModel({1,glm::vec3(x,0,z), tree_model});

        generate_leaves_around_tree(x,z,30);
    }
}

void MainController::update() {
    update_camera();
    update_light();
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
        scene->MoveMovableLight(glm::vec3(0,0,1)*dt);
                }
    if (platform->key(engine::platform::KEY_DOWN)
                .state() == engine::platform::Key::State::Pressed) {
        scene->MoveMovableLight(glm::vec3(0,0,-1)*dt);
                }
    if (platform->key(engine::platform::KEY_LEFT)
                .state() == engine::platform::Key::State::Pressed) {
        scene->MoveMovableLight(glm::vec3(1,0,0)*dt);
                }
    if (platform->key(engine::platform::KEY_RIGHT)
                .state() == engine::platform::Key::State::Pressed) {
        scene->MoveMovableLight(glm::vec3(-1,0,0)*dt);
                }
    //spdlog::info("Light position: {} {} {}", light_position.x, light_position.y, light_position.z);
}

void MainController::render_skybox() {
    auto shader = get<engine::resources::ResourcesController>()->shader("skybox");
    auto skybox_cube = get<engine::resources::ResourcesController>()->skybox("skybox");
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
