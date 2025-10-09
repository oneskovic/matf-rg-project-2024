#pragma once
#include "engine/resources/Model.hpp"
#include "engine/resources/Shader.hpp"


#include <engine/core/Controller.hpp>
#include <memory>

class Shader;
class Model;

class MainController final : public engine::core::Controller {
public:
    MainController() = default;
    ~MainController() override = default;

protected:
    void initialize() override;
    void begin_draw() override;
    void end_draw() override;
    void draw() override;
    void update() override;

private:
    void update_camera();
    void update_light();
    void render_light();
    glm::vec3 light_position;
    glm::vec3 light_color;
    engine::resources::Shader *shader;
    engine::resources::Model *triangle_model;
};
