// Connects the platformer Game implementation to the reusable engine infrastructure.

#include "mario/core/Game.hpp"
#include "mario/core/MenuState.hpp"

#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>

#include <memory>

namespace mario {

Game::Game() = default;

Game::~Game() = default;

void Game::initialize() {
    _running = true;
}

void Game::shutdown() {
    _states.clear();
    _assets.unload_all();
    _entities.clear();
    _running = false;
}

void Game::run() {
    initialize();
    before_loop();
    main_loop();
    shutdown();
}

void Game::push_state(std::shared_ptr<GameState> state) {
    if (!state) {
        return;
    }
    if (const auto current = current_state()) {
        current->on_exit();
    }
    _states.push_back(std::move(state));
    _states.back()->on_enter();
}

void Game::pop_state() {
    if (_states.empty()) {
        return;
    }
    _states.back()->on_exit();
    _states.pop_back();
    if (_states.empty()) {
        _running = false;
    }
}

std::shared_ptr<GameState> Game::current_state() {
    if (_states.empty()) {
        return nullptr;
    }
    return _states.back();
}

Renderer &Game::renderer() {
    return _renderer;
}

InputManager &Game::input() {
    return _input;
}

AssetManager &Game::assets() {
    return _assets;
}

EntityManager &Game::entity_manager() {
    return _entities;
}

void Game::before_loop() {
    if (!current_state()) {
        push_state(std::make_shared<MenuState>(*this));
    }
}

void Game::main_loop() {
    sf::Clock clock;
    constexpr sf::Time target_frame_time = sf::seconds(1.0f / 60.0f);
    sf::Time previous_time = clock.getElapsedTime();

    while (_running && current_state()) {
        const sf::Time current_time = clock.getElapsedTime();
        const float dt = (current_time - previous_time).asSeconds();
        previous_time = current_time;

        const auto state = current_state();
        state->update(dt);
        state->render();

        const sf::Time elapsed = clock.getElapsedTime() - current_time;
        if (elapsed < target_frame_time) {
            sf::sleep(target_frame_time - elapsed);
        }
    }
}

} // namespace mario
