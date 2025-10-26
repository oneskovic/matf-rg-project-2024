#pragma once
#include "Scene.h"


#include <engine/core/Controller.hpp>
#include <random>
class LeafAnimator {
public:
        LeafAnimator(const std::shared_ptr<Scene::Model>& model, std::mt19937& rng)
            : model(model)
            , start_pos(model->position)
            , t(0)
        {
            // Randomize phase
            std::uniform_real_distribution<float> phase_dist(-1,1);
            liftPhase = phase_dist(rng);
            pitchPhase = phase_dist(rng);
            swayPhase = phase_dist(rng);
            rollPhase = phase_dist(rng);
        }
        void animate(float delta_time);
private:
    static float damp(float value, float target, float k, float dt);
    static bool almost_zero(float v);

    std::shared_ptr<Scene::Model> model;
    glm::vec3 start_pos;
    float t;
    bool settled = false;

    glm::vec2 windDir        = glm::vec2(1.5f, 0.3f);
    float windSpeed          = 0.1f;

    float swayAmp            = 0.9f;
    float swayFreq           = 2.0f;
    float swayPhase          = 0.0f;

    float fallSpeed          = 0.1f;
    float fallAccel          = 0.1f;

    float liftAmp            = 0.01f;
    float liftFreq           = 7.0f;
    float liftPhase          = 0.0f;

    float yaw0               = 0.0f;
    float spinRate           = 0.8f;

    float pitchBase          = 0.4f;
    float rollBase           = 0.1f;
    float pitchAmp           = 0.6f;
    float rollAmp            = 0.4f;
    float pitchFreq          = 5.0f;
    float rollFreq           = 3.2f;
    float pitchPhase         = 0.0f;
    float rollPhase          = 0.0f;
};
class AnimationController final : public engine::core::Controller {
public:
    void initialize() override;
    void update() override;
    void animate_leaf(const std::shared_ptr<Scene::Model> &model, std::mt19937& rng);
private:
    std::vector<std::shared_ptr<LeafAnimator>> animators;
    float m_last_update_time_seconds = 0;
};