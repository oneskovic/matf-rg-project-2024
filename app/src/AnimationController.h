#pragma once
#include <random>
#include <engine/core/Controller.hpp>
#include <engine/resources/SceneModel.hpp>

using namespace engine::resources;


class LeafAnimator {
public:
    LeafAnimator(const std::shared_ptr<SceneModel>& model, std::mt19937& rng)
        : m_model(model)
        , m_start_pos(model->position)
        , m_t(0)
    {
        // Randomize phase
        std::uniform_real_distribution<float> phase_dist(-1,1);
        m_lift_phase = phase_dist(rng);
        m_pitch_phase = phase_dist(rng);
        m_sway_phase = phase_dist(rng);
        m_roll_phase = phase_dist(rng);
    }
    void animate(float delta_time);
private:
    static float damp(float value, float target, float k, float dt);
    static bool almost_zero(float v);

    std::shared_ptr<SceneModel> m_model;
    glm::vec3 m_start_pos;
    float m_t;
    bool m_settled = false;

    glm::vec2 m_wind_dir        = glm::vec2(1.5f, 0.3f);
    float m_wind_speed          = 0.1f;

    float m_sway_amp            = 0.9f;
    float m_sway_freq           = 2.0f;
    float m_sway_phase          = 0.0f;

    float m_fall_speed          = 0.1f;
    float m_fall_accel          = 0.1f;

    float m_lift_amp            = 0.01f;
    float m_lift_freq           = 7.0f;
    float m_lift_phase          = 0.0f;

    float m_yaw0               = 0.0f;
    float m_spin_rate           = 0.8f;

    float m_pitch_base          = 0.4f;
    float m_roll_base           = 0.1f;
    float m_pitch_amp           = 0.6f;
    float m_roll_amp            = 0.4f;
    float m_pitch_freq          = 5.0f;
    float m_roll_freq           = 3.2f;
    float m_pitch_phase         = 0.0f;
    float m_roll_phase          = 0.0f;
};
class AnimationController final : public engine::core::Controller {
public:
    void initialize() override;
    void update() override;
    void animate_leaf(const std::shared_ptr<SceneModel> &model, std::mt19937& rng);
private:
    std::vector<std::shared_ptr<LeafAnimator>> m_animators;
    float m_last_update_time_seconds = 0;
};
