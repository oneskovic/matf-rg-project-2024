
#include "AnimationController.h"

#include "../../engine/libs/glfw/include/GLFW/glfw3.h"
#include "glm/detail/type_quat.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/gtx/quaternion.hpp"
#include "spdlog/spdlog.h"
void LeafAnimator::animate(float delta_time) {
    t += delta_time;
    if (settled) {
        return;
    }
    // We animate a falling leaf using a linear function combined with a sine curve to get a fake swaying effect

    float x =
        start_pos.x
        + windSpeed * t * windDir.x
        + swayAmp * std::sin(swayFreq * t + swayPhase);

    float z =
        start_pos.z
        + windSpeed * t * windDir.y
        + swayAmp * std::cos(swayFreq * t + swayPhase);

    // Falling down with variable lift to make it look like the leaf doesn't fall straight down
    float y_raw =
        start_pos.y
        - fallSpeed * t
        - 0.5f * fallAccel * t * t
        + liftAmp * std::sin(liftFreq * t + liftPhase);

    float groundY = 0.0f;
    if (y_raw <= groundY) {
        y_raw = groundY;

        // Damp all rotation when the leaf hits the ground
        liftAmp       = damp(liftAmp,       0.0f, 2.5f, delta_time);
        spinRate      = damp(spinRate,      0.0f, 2.5f, delta_time);
        pitchAmp      = damp(pitchAmp,      0.0f, 2.5f, delta_time);
        rollAmp       = damp(rollAmp,       0.0f, 2.5f, delta_time);

        // once everything is zero mark as settled and don't animate anymore
        if (almost_zero(spinRate) &&
            almost_zero(pitchAmp) &&
            almost_zero(rollAmp))
        {
            settled = true;
        }
    }

    glm::vec3 pos(x, y_raw, z);

    // Animate rotation around each axis in a similar way
    float yaw =
        yaw0 +
        spinRate * t;

    float pitch =
        pitchBase +
        pitchAmp * std::sin(pitchFreq * t + pitchPhase);

    float roll =
        rollBase +
        rollAmp * std::sin(rollFreq * t + rollPhase);

    // Convert to quaternions
    glm::quat qYaw   = glm::angleAxis(yaw,   glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat qRoll  = glm::angleAxis(roll,  glm::vec3(0.0f, 0.0f, 1.0f));

    glm::quat qFinal = qYaw * qPitch * qRoll;

    // Don't animate position if the leaf is at ground level
    if (!almost_zero(pos.y - groundY)) {
        model->position = pos;
    }
    model->rotation_mat = glm::toMat4(qFinal);
}

float LeafAnimator::damp(float value, float target, float k, float dt) {
    float dv = target - value;
    return value + dv * (1.0f - std::exp(-k * dt));
}

bool LeafAnimator::almost_zero(float v) {
    return std::abs(v) < 0.01f;
}

void AnimationController::initialize() {
    m_last_update_time_seconds = static_cast<float>(glfwGetTime());
}
void AnimationController::update() {
    float now = static_cast<float>(glfwGetTime());
    float delta_time = now - m_last_update_time_seconds;
    for (auto& animator : animators) {
        animator->animate(delta_time);
    }
    m_last_update_time_seconds = now;
}
void AnimationController::animate_leaf(const std::shared_ptr<Scene::Model>& model, std::mt19937& rng) {
    auto animator = std::make_shared<LeafAnimator>(model, rng);
    animators.push_back(animator);
}
