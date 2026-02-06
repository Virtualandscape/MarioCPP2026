#include "mario/systems/AnimationSystem.hpp"
#include "mario/ecs/components/AnimationComponent.hpp"
#include "mario/ecs/components/SpriteComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"
#include "mario/ecs/components/PlayerInputComponent.hpp"
#include "mario/helpers/Constants.hpp"
#include <cmath>

namespace mario {

void AnimationSystem::update(EntityManager& registry, float dt) const {
    static thread_local std::vector<EntityID> entities;
    registry.get_entities_with_all<AnimationComponent, SpriteComponent>(entities);

    for (auto entity : entities) {
        auto* anim = registry.get_component<AnimationComponent>(entity);
        auto* sprite = registry.get_component<SpriteComponent>(entity);
        auto* vel = registry.get_component<VelocityComponent>(entity);
        auto* input = registry.get_component<PlayerInputComponent>(entity);

        if (!anim || !sprite) continue;

        AnimationComponent::State next_state = anim->current_state;

        // Logic for player animation switching
        if (vel && input) {
            // Check if jumping (assuming jump_count > 0 means in air for now, 
            // though a grounded check would be better if available)
            if (input->jump_count > 0) {
                next_state = AnimationComponent::State::Jump;
            } else if (std::abs(vel->vx) > 0.1f) {
                next_state = AnimationComponent::State::Run;
            } else {
                next_state = AnimationComponent::State::Idle;
            }

            // Update flip: assume true means face right, false means face left
            bool old_flip = anim->flip_x;
            if (vel->vx > 0.1f) anim->flip_x = true;
            else if (vel->vx < -0.1f) anim->flip_x = false;
            else anim->flip_x = true; // Default to facing right when stopped

            if (old_flip != anim->flip_x) anim->is_dirty = true;
        }

        // State transition logic
        if (next_state != anim->current_state) {
            anim->current_state = next_state;
            anim->current_frame = 0;
            anim->frame_timer = 0.0f;
            anim->is_dirty = true;

            // Set state specific values
            switch (anim->current_state) {
                case AnimationComponent::State::Idle:
                    sprite->texture_id = constants::PLAYER_IDLE_ID;
                    anim->frame_count = 1;
                    anim->frame_duration = 1.0f;
                    break;
                case AnimationComponent::State::Run:
                    sprite->texture_id = constants::PLAYER_RUN_ID;
                    anim->frame_count = constants::PLAYER_RUN_FRAMES;
                    anim->frame_duration = constants::PLAYER_FRAME_DURATION;
                    break;
                case AnimationComponent::State::Jump:
                    sprite->texture_id = constants::PLAYER_JUMP_ID;
                    anim->frame_count = constants::PLAYER_JUMP_FRAMES;
                    anim->frame_duration = constants::PLAYER_FRAME_DURATION;
                    break;
            }
        }

        // Update frame timer
        anim->frame_timer += dt;
        if (anim->frame_timer >= anim->frame_duration) {
            anim->frame_timer -= anim->frame_duration;
            anim->current_frame = (anim->current_frame + 1) % anim->frame_count;
            anim->is_dirty = true;
        }

            // Update texture rect if dirty
        if (anim->is_dirty) {
            int left = anim->current_frame * constants::PLAYER_FRAME_WIDTH;
            int top = 0;
            int width = constants::PLAYER_FRAME_WIDTH;
            int height = constants::PLAYER_FRAME_HEIGHT;

            // In SFML 3, sf::IntRect uses {position, size}
            // We use the absolute frame size in texture_rect, and let the Renderer handle flipping.
            sprite->texture_rect = sf::IntRect({left, top}, {width, height});
            
            // To signal flipping to the renderer, we could use negative width in texture_rect
            // as Renderer::draw_sprite currently checks for texture_rect.size.x < 0
            if (anim->flip_x) {
                sprite->texture_rect.position.x += width;
                sprite->texture_rect.size.x = -width;
            }

            anim->is_dirty = false;
        }
    }
}

} // namespace mario
