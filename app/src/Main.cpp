#include "AnimationController.h"
#include "MainController.hpp"
#include <engine/core/Controller.hpp>
#include <engine/core/Engine.hpp>
#include <memory>


class MyApp final : public engine::core::App {
protected:
    void app_setup() override {
        auto anim_controller = register_controller<AnimationController>();
        anim_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
        auto main_controller = register_controller<MainController>();
        main_controller->after(anim_controller);
    }
};


int main(int argc, char** argv) {
    return std::make_unique<MyApp>()->run(argc, argv);
}
