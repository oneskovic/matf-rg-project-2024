#include "MainController.hpp"
#include <engine/core/Controller.hpp>
#include <engine/core/Engine.hpp>
#include <memory>


class MyApp final : public engine::core::App {
protected:
    void app_setup() override {
        auto controller = register_controller<MainController>();
        controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
    }
};


int main(int argc, char** argv) {
    return std::make_unique<MyApp>()->run(argc, argv);
}
