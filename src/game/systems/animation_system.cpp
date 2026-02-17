#include "Zia/game/systems/AnimationSystem.hpp"
#include "Zia/engine/ecs/components/AnimationComponent.hpp"
#include "Zia/engine/ecs/components/SpriteComponent.hpp"
#include "Zia/engine/ecs/components/VelocityComponent.hpp"
#include "Zia/engine/ecs/components/PlayerControllerComponent.hpp"
#include "Zia/game/helpers/Constants.hpp"
#include <cmath>

namespace zia {

void AnimationSystem::update(zia::engine::IEntityManager& registry, float dt) const {
    static thread_local std::vector<EntityID> entities;
    registry.get_entities_with_all<AnimationComponent, SpriteComponent>(entities);

    for (auto entity : entities) {
        // Get components as optionals of reference_wrapper — avoid raw pointers.
        auto anim_opt = registry.get_component<AnimationComponent>(entity);
        auto sprite_opt = registry.get_component<SpriteComponent>(entity);
        auto vel_opt = registry.get_component<VelocityComponent>(entity);
        auto ctrl_opt = registry.get_component<PlayerControllerComponent>(entity);

        // If mandatory components are missing, skip the entity.
        if (!anim_opt || !sprite_opt) continue;

        // Extract references from optionals for convenient access.
        AnimationComponent& anim = anim_opt->get();
        SpriteComponent& sprite = sprite_opt->get();

        AnimationComponent::State next_state = anim.current_state;

        // Prefer controller-driven animation state when available
        if (ctrl_opt) {
            PlayerControllerComponent& ctrl = ctrl_opt->get();
            next_state = ctrl.requested_state;

            // Update flip from a controller facing a direction
            bool old_flip = anim.flip_x;
            anim.flip_x = ctrl.facing_right;
            if (old_flip != anim.flip_x) anim.needs_rect_update = true;
        }
        // Fallback: derive state from velocity if no controller present
        else if (vel_opt) {
            VelocityComponent& vel = vel_opt->get();
            if (std::abs(vel.vx) > 0.1f) {
                next_state = AnimationComponent::State::Run;
            } else {
                next_state = AnimationComponent::State::Idle;
            }

            // Update flip based on velocity
            bool old_flip = anim.flip_x;
            if (vel.vx > 0.1f) anim.flip_x = true;
            else if (vel.vx < -0.1f) anim.flip_x = false;
            else anim.flip_x = true; // default
            if (old_flip != anim.flip_x) anim.needs_rect_update = true;
        }

        // If currently playing a one-shot celebrate animation, preserve it until it finishes
        if (anim.is_one_shot) {
            // Do not override next_state from controller/velocity while celebrating
            next_state = anim.current_state;
        }

        // If there are queued one-shot plays and we're not currently playing one, start Celebrate now.
        if (!anim.is_one_shot && anim.one_shot_queue > 0) {
            // Consume one queued play and request the Celebrate state for this frame.
            anim.one_shot_queue -= 1;
            next_state = AnimationComponent::State::Celebrate;
        }

        // State transition logic
        if (next_state != anim.current_state) {
            anim.current_state = next_state;
            anim.current_frame = 0;
            anim.frame_timer = 0.0f;
            anim.needs_rect_update = true;

            // Clear any previous one-shot flag when a new explicit state is requested
            anim.is_one_shot = false;
            // Mark that we just started this state so we don't advance frames immediately this tick
            anim.just_started = true;

            // Set state-specific values
            switch (anim.current_state) {
                case AnimationComponent::State::Idle:
                    sprite.texture_id = constants::PLAYER_IDLE_ID;
                    anim.frame_count = 1;
                    anim.frame_duration = 1.0f;
                    break;
                case AnimationComponent::State::Run:
                    sprite.texture_id = constants::PLAYER_RUN_ID;
                    anim.frame_count = constants::PLAYER_RUN_FRAMES;
                    anim.frame_duration = constants::PLAYER_FRAME_DURATION;
                    break;
                case AnimationComponent::State::Jump:
                    sprite.texture_id = constants::PLAYER_JUMP_ID;
                    anim.frame_count = constants::PLAYER_JUMP_FRAMES;
                    anim.frame_duration = constants::PLAYER_FRAME_DURATION;
                    break;
                case AnimationComponent::State::Celebrate:
                    // Celebrate is a one-shot animation: use dedicated texture and frame count.
                    sprite.texture_id = constants::PLAYER_CELEBRATE_ID;
                    anim.frame_count = constants::PLAYER_CELEBRATE_FRAMES;
                    anim.frame_duration = constants::PLAYER_FRAME_DURATION;
                    // Ensure the engine knows this is a one-shot unless already set by caller
                    anim.is_one_shot = true;
                    break;
            }
            // We've just transitioned — avoid advancing frames or finishing the animation in the same tick.
            continue;
        }

        // Update frame timer
        anim.frame_timer += dt;
        if (anim.frame_timer >= anim.frame_duration) {
            anim.frame_timer -= anim.frame_duration;
            // If we just started this state this tick, skip advancing frames this tick but clear the flag.
            if (anim.just_started) {
                anim.just_started = false;
                // do not advance frame this tick; wait for next tick
                anim.needs_rect_update = true;
            } else {
                // For one-shot animations, advance until the last frame then stop and restore Idle.
                if (anim.is_one_shot) {
                    if (anim.current_frame + 1 < anim.frame_count) {
                        anim.current_frame = anim.current_frame + 1;
                        anim.needs_rect_update = true;
                    } else {
                        // Reached the last frame of the one-shot. Finish the animation and restore Idle visuals.
                        // If there are queued extra plays, start Celebrate again; otherwise restore Idle
                        if (anim.one_shot_queue > 0) {
                            anim.one_shot_queue -= 1;
                             // Restart celebrate animation
                             anim.current_frame = 0;
                             anim.frame_timer = 0.0f;
                             anim.frame_count = constants::PLAYER_CELEBRATE_FRAMES;
                             anim.frame_duration = constants::PLAYER_FRAME_DURATION;
                             anim.needs_rect_update = true;
                             anim.is_one_shot = true;
                             sprite.texture_id = constants::PLAYER_CELEBRATE_ID;
                         } else {
                            anim.is_one_shot = false;
                            anim.current_frame = 0;
                            anim.needs_rect_update = true;
                            anim.current_state = AnimationComponent::State::Idle;
                            sprite.texture_id = constants::PLAYER_IDLE_ID;
                            anim.frame_count = 1;
                            anim.frame_duration = 1.0f;
                            anim.one_shot_queue = 0;
                         }
                     }
                 } else {
                     // Normal looping animations use modulo to wrap frames.
                     anim.current_frame = (anim.current_frame + 1) % anim.frame_count;
                     anim.needs_rect_update = true;
                 }
             }
         }

         // Update the sprite's texture_rect based on the current frame and flip state.
         if (anim.needs_rect_update) {
             int left = anim.current_frame * constants::PLAYER_FRAME_WIDTH;
             int top = 0;
             int width = constants::PLAYER_FRAME_WIDTH;
             int height = constants::PLAYER_FRAME_HEIGHT;

             // We use the absolute frame size in texture_rect and let the Renderer handle flipping.
             sprite.texture_rect = sf::IntRect({left, top}, {width, height});

             // To signal flipping to the renderer, we could use negative width in texture_rect
             // as Renderer::draw_sprite currently checks for texture_rect.size.x < 0
             if (anim.flip_x) {
                 sprite.texture_rect.position.x += width;
                 sprite.texture_rect.size.x = -width;
             }

             anim.needs_rect_update = false;
         }
     }
 }

 } // namespace Zia
