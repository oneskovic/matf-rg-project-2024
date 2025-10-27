#define GLM_ENABLE_EXPERIMENTAL
#include "glm/detail/type_quat.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/gtx/quaternion.hpp"
#include "spdlog/spdlog.h"
#include "AnimationController.h"

#include <engine/platform/PlatformController.hpp>

void LeafAnimator::animate(float delta_time) {
    m_t += delta_time;
    if (m_settled) {
        return;
    }
    // We animate a falling leaf using a linear function combined with a sine curve to get a fake swaying effect

    float x =
        m_start_pos.x
        + m_wind_speed * m_t * m_wind_dir.x
        + m_sway_amp * std::sin(m_sway_freq * m_t + m_sway_phase);

    float z =
        m_start_pos.z
        + m_wind_speed * m_t * m_wind_dir.y
        + m_sway_amp * std::cos(m_sway_freq * m_t + m_sway_phase);

    // Falling down with variable lift to make it look like the leaf doesn't fall straight down
    float y_raw =
        m_start_pos.y
        - m_fall_speed * m_t
        - 0.5f * m_fall_accel * m_t * m_t
        + m_lift_amp * std::sin(m_lift_freq * m_t + m_lift_phase);

    float groundY = 0.0f;
    if (y_raw <= groundY) {
        y_raw = groundY;

        // Damp all rotation when the leaf hits the ground
        m_lift_amp       = damp(m_lift_amp,       0.0f, 2.5f, delta_time);
        m_spin_rate      = damp(m_spin_rate,      0.0f, 2.5f, delta_time);
        m_pitch_amp      = damp(m_pitch_amp,      0.0f, 2.5f, delta_time);
        m_roll_amp       = damp(m_roll_amp,       0.0f, 2.5f, delta_time);

        // once everything is zero mark as settled and don't animate anymore
        if (almost_zero(m_spin_rate) &&
            almost_zero(m_pitch_amp) &&
            almost_zero(m_roll_amp))
        {
            m_settled = true;
        }
    }

    glm::vec3 pos(x, y_raw, z);

    // Animate rotation around each axis in a similar way
    float yaw =
        m_yaw0 +
        m_spin_rate * m_t;

    float pitch =
        m_pitch_base +
        m_pitch_amp * std::sin(m_pitch_freq * m_t + m_pitch_phase);

    float roll =
        m_roll_base +
        m_roll_amp * std::sin(m_roll_freq * m_t + m_roll_phase);

    // Convert to quaternions
    glm::quat qYaw   = glm::angleAxis(yaw,   glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat qRoll  = glm::angleAxis(roll,  glm::vec3(0.0f, 0.0f, 1.0f));

    glm::quat qFinal = qYaw * qPitch * qRoll;

    // Don't animate position if the leaf is at ground level
    if (!almost_zero(pos.y - groundY)) {
        m_model->position = pos;
    }
    m_model->rotation_mat = glm::toMat4(qFinal);
}

float LeafAnimator::damp(float value, float target, float k, float dt) {
    float dv = target - value;
    return value + dv * (1.0f - std::exp(-k * dt));
}

bool LeafAnimator::almost_zero(float v) {
    return std::abs(v) < 0.01f;
}

void AnimationController::initialize() {
    m_last_update_time_seconds = get<engine::platform::PlatformController>()->get_time();
}
void AnimationController::update() {
    float now = get<engine::platform::PlatformController>()->get_time();
    float delta_time = now - m_last_update_time_seconds;
    for (auto& animator : m_animators) {
        animator->animate(delta_time);
    }
    m_last_update_time_seconds = now;
}
void AnimationController::animate_leaf(const std::shared_ptr<SceneModel>& model, std::mt19937& rng) {
    auto animator = std::make_shared<LeafAnimator>(model, rng);
    m_animators.push_back(animator);
}
