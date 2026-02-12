#include "mario/systems/RenderSystems.hpp"
#include "mario/ecs/Components.hpp"
#include "mario/resources/AssetManager.hpp"
#include "mario/helpers/Constants.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

namespace mario::systems::render {

    void update_camera(EntityManager& registry, Camera& camera, float dt, float viewport_w, float viewport_h, EntityID target) {
        camera.set_viewport(viewport_w, viewport_h);
        camera.update(dt);

        if (target == 0) return;

        auto pos_opt = registry.get_component<PositionComponent>(target);
        auto size_opt = registry.get_component<SizeComponent>(target);
        if (pos_opt && size_opt) {
            const float center_x = pos_opt->get().x + size_opt->get().width * 0.5f;
            const float center_y = pos_opt->get().y + size_opt->get().height * 0.5f;
            camera.set_target(center_x, center_y);
        }
    }

    void init_camera(EntityManager& registry, Camera& camera, float viewport_w, float viewport_h, EntityID target,
                      float initial_offset_x, float initial_offset_y) {
        camera.set_viewport(viewport_w, viewport_h);
        if (target == 0) return;

        auto pos_opt = registry.get_component<PositionComponent>(target);
        auto size_opt = registry.get_component<SizeComponent>(target);
        if (pos_opt && size_opt) {
            const float center_x = pos_opt->get().x + size_opt->get().width * 0.5f;
            const float center_y = pos_opt->get().y + size_opt->get().height * 0.5f;
            const float target_x = center_x - viewport_w * 0.5f;
            const float target_y = center_y - viewport_h * 0.5f;

            camera.set_target(center_x, center_y);
            camera.set_position(target_x + initial_offset_x, target_y + initial_offset_y);
        }
    }

    void update_animations(EntityManager& registry, float dt) {
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with_all<AnimationComponent, SpriteComponent>(entities);

        for (auto entity : entities) {
            auto anim_opt = registry.get_component<AnimationComponent>(entity);
            auto sprite_opt = registry.get_component<SpriteComponent>(entity);
            auto vel_opt = registry.get_component<VelocityComponent>(entity);
            auto input_opt = registry.get_component<PlayerInputComponent>(entity);

            if (!anim_opt || !sprite_opt) continue;

            auto& anim = anim_opt->get();
            auto& sprite = sprite_opt->get();

            AnimationComponent::State next_state = anim.current_state;

            if (vel_opt && input_opt) {
                auto& vel = vel_opt->get();
                auto& input = input_opt->get();

                if (input.jump_count > 0) {
                    next_state = AnimationComponent::State::Jump;
                } else if (std::abs(vel.vx) > 0.1f) {
                    next_state = AnimationComponent::State::Run;
                } else {
                    next_state = AnimationComponent::State::Idle;
                }

                if (vel.vx > 0.1f) anim.flip_x = true;
                else if (vel.vx < -0.1f) anim.flip_x = false;
            }

            if (next_state != anim.current_state) {
                anim.current_state = next_state;
                anim.current_frame = 0;
                anim.frame_timer = 0.0f;
                anim.is_dirty = true;

                using namespace mario::constants;
                if (next_state == AnimationComponent::State::Idle) {
                    sprite.texture_id = PLAYER_IDLE_ID;
                    anim.frame_count = 1;
                } else if (next_state == AnimationComponent::State::Run) {
                    sprite.texture_id = PLAYER_RUN_ID;
                    anim.frame_count = PLAYER_RUN_FRAMES;
                } else if (next_state == AnimationComponent::State::Jump) {
                    sprite.texture_id = PLAYER_JUMP_ID;
                    anim.frame_count = PLAYER_JUMP_FRAMES;
                }
            }

            anim.frame_timer += dt;
            if (anim.frame_timer >= anim.frame_duration) {
                anim.frame_timer = 0.0f;
                anim.current_frame = (anim.current_frame + 1) % anim.frame_count;
                anim.is_dirty = true;
            }

            if (anim.is_dirty) {
                const int fw = mario::constants::PLAYER_FRAME_WIDTH;
                const int fh = fw; // assuming square frames
                int left = anim.current_frame * fw;
                int top = 0;
                int width = anim.flip_x ? fw : -fw;
                sprite.texture_rect = sf::IntRect({anim.flip_x ? left : left + fw, top}, {width, fh});
                anim.is_dirty = false;
            }
        }
    }

    void render_sprites(Renderer& renderer, const Camera& camera, EntityManager& registry, AssetManager& assets) {
        renderer.set_camera(camera.x(), camera.y());
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with<SpriteComponent, PositionComponent, SizeComponent>(entities);

        for (auto entity : entities) {
            auto sprite_opt = registry.get_component<SpriteComponent>(entity);
            auto pos_opt = registry.get_component<PositionComponent>(entity);
            auto size_opt = registry.get_component<SizeComponent>(entity);

            if (sprite_opt && pos_opt && size_opt) {
                auto& sprite = sprite_opt->get();
                auto& pos = pos_opt->get();
                auto& size = size_opt->get();

                if (sprite.texture_id != -1) {
                    auto tex = assets.get_texture(sprite.texture_id);
                    if (tex) {
                        const float draw_w = (sprite.render_size.x > 0.0f) ? sprite.render_size.x : size.width;
                        const float draw_h = (sprite.render_size.y > 0.0f) ? sprite.render_size.y : size.height;
                        renderer.draw_sprite(*tex, pos.x + sprite.render_offset.x, pos.y + sprite.render_offset.y,
                                             draw_w, draw_h, sprite.texture_rect);
                    }
                } else {
                    if (sprite.shape == SpriteComponent::Shape::Rectangle) {
                        renderer.draw_rect(pos.x, pos.y, size.width, size.height, sprite.color);
                    }
                }
            }
        }
    }

    void render_backgrounds(EntityManager& registry, Renderer& renderer, AssetManager& assets, const Camera& camera) {
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with<BackgroundComponent>(entities);

        // Sort backgrounds by parallax to render further layers first.
        std::sort(entities.begin(), entities.end(), [&](EntityID a, EntityID b) {
            auto bg_a = registry.get_component<BackgroundComponent>(a);
            auto bg_b = registry.get_component<BackgroundComponent>(b);
            if (!bg_a || !bg_b) return false;
            return bg_a->get().parallax < bg_b->get().parallax;
        });

        for (auto entity : entities) {
            auto bg_opt = registry.get_component<BackgroundComponent>(entity);
            if (!bg_opt) continue;
            auto& bg = bg_opt->get();

            auto tex = assets.get_mutable_texture(bg.texture_id);
            if (!tex) continue;

            if (tex->isRepeated() != bg.repeat) {
                tex->setRepeated(bg.repeat);
            }

            const auto viewport = renderer.viewport_size();
            const float vw = viewport.x;
            const float vh = viewport.y;
            const float tw = static_cast<float>(tex->getSize().x);
            const float th = static_cast<float>(tex->getSize().y);

            float scaleX = vw / tw;
            float scaleY = vh / th;

            if (bg.preserve_aspect) {
                float scale = (bg.scale_mode == BackgroundComponent::ScaleMode::Fit)
                    ? std::min(scaleX, scaleY) : std::max(scaleX, scaleY);
                scaleX = scaleY = scale;
            }

            scaleX *= bg.scale_multiplier;
            scaleY *= bg.scale_multiplier;

            sf::Sprite s(*tex);
            s.setScale({scaleX, scaleY});

            // Parallax factor: 0 = follows camera, 1 = static background
            float x = -camera.x() * (1.0f - bg.parallax) + bg.offset_x;
            s.setPosition({x, bg.offset_y});

            renderer.window().draw(s);
        }
    }

    void render_clouds(Renderer& renderer, const Camera& camera, AssetManager& assets, EntityManager& registry) {
        using namespace mario::constants;
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with<CloudComponent>(entities);

        // Depth sort by layer: Big -> Medium -> Small.
        std::sort(entities.begin(), entities.end(), [&](EntityID a, EntityID b) {
            auto ca = registry.get_component<CloudComponent>(a);
            auto cb = registry.get_component<CloudComponent>(b);
            if (!ca || !cb) return false;
            return static_cast<int>(ca->get().layer) < static_cast<int>(cb->get().layer);
        });

        sf::RenderWindow& window = renderer.window();
        const sf::View old_view = window.getView();
        window.setView(window.getDefaultView());

        for (auto entity : entities) {
            auto cloud_opt = registry.get_component<CloudComponent>(entity);
            if (!cloud_opt) continue;
            auto& cloud = cloud_opt->get();

            auto tex = assets.get_texture(cloud.texture_id);
            if (!tex) continue;

            sf::Sprite sprite(*tex);
            sprite.setScale({cloud.scale, cloud.scale});

            float pos_x = cloud.x - camera.x() * CLOUD_PARALLAX;
            sprite.setPosition({pos_x, cloud.y});
            window.draw(sprite);
        }

        window.setView(old_view);
    }

    void render_debug(EntityManager& registry, Renderer& renderer, const Camera& camera) {
        // Render hitboxes, entity centers, etc.
    }
}

