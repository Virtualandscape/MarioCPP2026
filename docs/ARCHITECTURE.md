# Architecture du Projet Mario

## Vue d'ensemble

Le projet suit une **architecture ECS (Entity Component System)** combinée avec un **gestionnaire d'états (Scene Pattern)** et une **architecture en couches avec interfaces abstraites**. Voici comment les composants interagissent :

```
┌─────────────────────────────────────────────────────────────────┐
│                       main()                                    │
│    Crée Game → initialize() → run()                             │
└─────────────────────────────┬───────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│              Game (Wrapper Game-level)                          │
│  • Owns: engine::Application instance                           │
│  • Forwards scene management to Application                     │
│  • Provides game-level accessors                                │
└─────────────────────────────┬───────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│         engine::Application (Core Application Layer)            │
│  • Owns concrete: Renderer, InputManager, AssetManager, etc.    │
│  • Manages: Scene Stack (push/pop scenes)                       │
│  • Fixed timestep loop, frame throttling                        │
└─────────────────────────────┬───────────────────────────────────┘
                              │
          ┌───────────────────┼───────────────────┐
          │                   │                   │
          ▼                   ▼                   ▼
      ┌────────────┐     ┌──────────┐      ┌──────────────┐
      │IRenderer   │     │IInput    │      │IAssetManager │
      │(interface) │     │(interface)      │(interface)   │
      └────────────┘     └──────────┘      └──────────────┘
          │                   │                   │
          └───────────────────┼───────────────────┘
                              │
                    ┌─────────────────────┐
                    │ IEntityManager      │
                    │ (interface)         │
                    └─────────────────────┘
                              │
          ┌───────────────────┼───────────────────┐
          │                   │                   │
          ▼                   ▼                   ▼
    ┌──────────────┐   ┌────────────────┐   ┌──────────────┐
    │Scene         │   │   IScene       │   │   Scene      │
    │(game::Scene) │   │(engine::IScene)│   │(Impl: Play)  │
    │ on_enter()   │   │ on_enter()     │   └──────────────┘
    │ on_exit()    │   │ on_exit()      │
    │ update(dt)   │   │ update(dt)     │
    │ render()     │   │ render()       │
    │ is_running() │   │ is_running()   │
    └──────────────┘   └────────────────┘
          △                   △
          └───────┬───────────┘
                  │
         (Adapters conversionent)
                  │
                  ▼
    ┌──────────────────────────────────────┐
    │ PlayScene                            │
    │ • Owns: Level, Systems, HUD          │
    │ • Implements Scene (game-level)      │
    │ • Spawns/updates entities via ECS    │
    └──────────────────────────────────────┘
```

---

## Détail des Composants Principaux

### 1. **Game** (Wrapper Game-Level)
- **Fichier** : `include/mario/game/MarioGame.hpp`
- **Responsabilités** :
  - Thin wrapper autour d'`engine::Application`
  - Fournit une API game-level cohérente
  - Gère la pile de scènes game-level (Scene, pas IScene)
  - Adaptation de scènes game-level vers engine::IScene via adaptateurs
  
- **Relations** :
  - Possède : `std::unique_ptr<engine::Application>`
  - Gère : Stack de `std::shared_ptr<Scene>`

---

### 2. **engine::Application** (Couche Engine)
- **Fichier** : `include/mario/engine/Application.hpp`
- **Responsabilités** :
  - Boucle principale (fixed timestep, frame throttling)
  - Détient les gestionnaires concrètes (Renderer, InputManager, etc.)
  - Gère la pile d'états via interfaces (`IScene`)
  - Découplage entre engine et game via interfaces
  
- **Relations** :
  - Possède : Instances concrètes ou interfaces de tous les managers
  - Gère : Stack de `std::shared_ptr<IScene>`
  - Hooks : `before_loop()` (pour les sous-classes)

---

### 3. **Scene & IScene** (Pattern d'État)
- **Fichier Scene (Game-level)** : `include/mario/engine/Scene.hpp`
- **Fichier IScene (Engine interface)** : `include/mario/engine/IScene.hpp`
- **Responsabilités** :
  - Interface abstraite pour le cycle de vie des scènes
  - Définit : `on_enter()`, `on_exit()`, `update(dt)`, `render()`, `is_running()`
  
- **Implémentations** :
  - `PlayScene` : Gère la gameplay active (ECS systems, Level loading)
  - `MenuScene` : Menu principal
  - `PauseScene` : Écran de pause
  
- **Adaptateurs** : `engine::adapters::SceneAdapter` convertit `Scene` → `IScene`

---

### 4. **Interfaces Abstraites** (Inversion de Contrôle)
Le projet utilise des interfaces pour découpler la logique game de l'implémentation engine :

#### **IRenderer**
- **Fichier** : `include/mario/engine/IRenderer.hpp`
- Méthodes clés : `begin_frame()`, `end_frame()`, `draw_sprite()`, `draw_rect()`, `set_camera()`, etc.

#### **IInput**
- **Fichier** : `include/mario/engine/IInput.hpp`
- Méthodes clés : `poll()`, `is_pressed(Action)`

#### **IAssetManager**
- **Fichier** : `include/mario/engine/IAssetManager.hpp`
- Méthodes clés : `load_texture()`, `get_texture()`, `load_font()`, `finalize_decoded_images()`

#### **IEntityManager**
- **Fichier** : `include/mario/engine/IEntityManager.hpp`
- Méthodes clés : `create_entity()`, `add_component<T>()`, `get_component<T>()`, `get_entities_with<T>()`

#### **Adaptateurs** (`engine::adapters::`)
- `RendererAdapter` : Enveloppe `Renderer` → `IRenderer`
- `InputAdapter` : Enveloppe `InputManager` → `IInput`
- `AssetManagerAdapter` : Enveloppe `AssetManager` → `IAssetManager`
- `EntityManagerAdapter` : Enveloppe `EntityManager` → `IEntityManager`
- `SceneAdapter` : Enveloppe `Scene` → `IScene`

---

### 5. **Renderer** (Implémentation Rendu)
- **Fichier** : `include/mario/engine/render/Renderer.hpp`
- **Responsabilités** :
  - Gère la fenêtre SFML (`sf::RenderWindow`)
  - Gère caméra et transformations de vue
  - Offre méthodes de dessin : sprites, rectangles, texte, bbox debug
  - Gère le zoom et le clamping de caméra
  - Implémente `IRenderer`
  
- **Dépendances** :
  - SFML 3.0.2 (`sf::RenderWindow`, `sf::Texture`, `sf::Font`, `sf::Sprite`)

---

### 6. **InputManager** (Implémentation Entrée)
- **Fichier** : `include/mario/engine/input/InputManager.hpp`
- **Responsabilités** :
  - Poll les touches SFML
  - Traduit en actions abstraites : `MoveLeft`, `MoveRight`, `Jump`, `Escape`, `ToggleDebug`, etc.
  - Fournit `is_pressed(action)`
  - Implémente `IInput`
  
- **Utilité** :
  - Découplage entre entrée hardware et logique du jeu

---

### 7. **AssetManager** (Implémentation Gestion des Ressources)
- **Fichier** : `include/mario/engine/resources/AssetManager.hpp`
- **Responsabilités** :
  - Charge et met en cache les textures
  - Charge les polices (fonts)
  - Implémente `IAssetManager`
  - Support du chargement multi-threaded via `push_decoded_image()` / `finalize_decoded_images()`
  
- **Utilité** :
  - Évite les chargements redondants
  - Smart pointers pour la gestion mémoire
  - Thread-safe pour les assets décodés

---

### 8. **EntityManager** (ECS Core)
- **Fichier** : `include/mario/engine/ecs/EntityManager.hpp`
- **Responsabilités** :
  - Crée les entités avec des IDs uniques (`EntityID = uint32_t`)
  - Ajoute/récupère des composants typés à une entité (via `std::any` + `std::type_index`)
  - Permet les requêtes d'entités par type de composant
  - Implémente via `IEntityManager` (adaptatée)
  
- **Architecture ECS** :
  ```
  EntityID = uint32_t (1-based, 0 = invalid)
  
  Entité = Collection de Composants
  
  Entity 1: [PositionComponent, SpriteComponent, VelocityComponent, PlayerControllerComponent]
  Entity 2: [PositionComponent, SpriteComponent, VelocityComponent, EnemyComponent]
  Entity 3: [PositionComponent, SizeComponent, BackgroundComponent]
  ```

- **Composants Disponibles** :
  - `PositionComponent` : position (x, y)
  - `SizeComponent` : dimensions (width, height)
  - `VelocityComponent` : vélocité (vx, vy)
  - `SpriteComponent` : ID texture, rect, flips
  - `AnimationComponent` : frame index, frame timer, animation data
  - `CollisionInfoComponent` : bounding box, propriétés de collision
  - `PlayerControllerComponent` : input state, jump data
  - `EnemyComponent` : IA, direction, patrouille
  - `BackgroundComponent` : couche parallax, offsets
  - `CloudComponent` : vitesse de dérive, offset
  - `TypeComponent` : type d'entité (pour le debug/inspection)

---

### 9. **Level** (Gestion des Niveaux)
- **Fichier** : `include/mario/game/world/Level.hpp`
- **Responsabilités** :
  - Charge les tilemaps depuis des fichiers JSON (parsé via `JsonHelper`)
  - Gère la caméra du niveau (`Camera`)
  - Stocke les spawn d'entités (`EntitySpawn[]`)
  - Gère les couches de parallax et backgrounds
  - Gère les clouds animés
  - Fournit accès aux propriétés du niveau (largeur, hauteur, spawn positions)
  
- **Relations** :
  - Détient : `TileMap`, `Camera`, `std::vector<EntitySpawn>`
  - Chargement asynchrone via `AssetManager`

---

### 10. **TileMap** (Grille de Tuiles)
- **Fichier** : `include/mario/game/world/TileMap.hpp`
- **Responsabilités** :
  - Charge et stocke la grille de tuiles depuis JSON
  - Fournit requêtes de collision : `is_solid(tx, ty)`, `is_in_bounds(x, y)`
  - Gère les dimensions (largeur, hauteur)
  - Permet l'itération sur tuiles via des accesseurs
  
- **Données** :
  - Grid 2D : `std::vector<std::vector<char>>`
  - Chaque cellule est un `char` (caractère) représentant le type de tuile

---

### 11. **Camera** (Caméra de Jeu)
- **Fichier** : `include/mario/game/world/Camera.hpp`
- **Responsabilités** :
  - Suit la position du joueur (smooth follow)
  - Clamp les limites pour ne pas quitter la map
  - Gère le viewport (position X, Y)
  - Aligne les bords de la caméra
  
- **Formule** :
  ```
  Chaque frame:
  camera_x = lerp(camera_x, target_x, follow_speed)
  camera_y = lerp(camera_y, target_y, follow_speed)
  clamp(camera_x, min, max)
  clamp(camera_y, min, max)
  ```

---

### 12. **Systèmes ECS** (Logic Métier)

Chaque système opère sur des entités avec des composants spécifiques via les interfaces `IEntityManager` et `IRenderer`.

#### **PhysicsSystem**
- **Fichier** : `include/mario/game/systems/PhysicsSystem.hpp`
- Applique la gravité (`_gravity = 1200.0f * TILE_SCALE`)
- Met à jour la vélocité et position via intégration
- Intègre friction et accélération
- **Composants** : `PositionComponent`, `VelocityComponent`

#### **CollisionSystem**
- **Fichier** : `include/mario/game/systems/CollisionSystem.hpp`
- Détecte les collisions AABB (axis-aligned bounding box)
- Résout les pénétrations (push back)
- Déclenche les événements de collision
- **Composants** : `PositionComponent`, `CollisionInfoComponent`

#### **PlayerControllerSystem**
- **Fichier** : `include/mario/game/systems/PlayerControllerSystem.hpp`
- Lit les entrées via `IInput`
- Met à jour la vélocité du joueur
- Gère les sauts (hold/release mechanics)
- **Composants** : `PlayerControllerComponent`, `VelocityComponent`

#### **AnimationSystem**
- **Fichier** : `include/mario/game/systems/AnimationSystem.hpp`
- Met à jour l'index de frame en fonction du temps écoulé
- Gère les transitions d'animation
- Wraps les frames de sprite
- **Composants** : `AnimationComponent`, `SpriteComponent`

#### **SpriteRenderSystem**
- **Fichier** : `include/mario/game/systems/SpriteRenderSystem.hpp`
- Dessine les sprites des entités via `IRenderer`
- Respecte l'ordre de layering (Z-order)
- Support des flips (horizontal/vertical)
- **Composants** : `PositionComponent`, `SpriteComponent`

#### **EnemySystem**
- **Fichier** : `include/mario/game/systems/EnemySystem.hpp`
- IA des ennemis (marche, patrouille, direction)
- Détecte le joueur pour les interactions
- Gère les changements de direction
- **Composants** : `EnemyComponent`, `PositionComponent`, `VelocityComponent`

#### **LevelSystem**
- **Fichier** : `include/mario/game/systems/LevelSystem.hpp`
- Gère les transitions entre niveaux
- Détecte la victoire (atteindre exit)
- Détecte la défaite (chute du joueur)
- Spawne les entités du niveau
- **Composants** : Various, depending on win/lose conditions

#### **CameraSystem**
- **Fichier** : `include/mario/game/systems/CameraSystem.hpp`
- Met à jour la position de caméra pour suivre le joueur
- Appelle `IRenderer::set_camera()` avec les nouvelles coordonnées
- Gère le smooth follow et clamping
- **Composants** : `PositionComponent` (du joueur)

#### **BackgroundSystem**
- **Fichier** : `include/mario/game/systems/BackgroundSystem.hpp`
- Dessine les couches de parallax
- Applique des offsets basés sur la position de caméra
- Gère plusieurs couches de parallax
- **Composants** : `BackgroundComponent`

#### **CloudSystem**
- **Fichier** : `include/mario/game/systems/CloudSystem.hpp`
- Dessine les clouds animés
- Gère la dérive des clouds (vitesse lente et constante)
- **Composants** : `CloudComponent`, `PositionComponent`

#### **DebugDrawSystem**
- **Fichier** : `include/mario/game/systems/DebugDrawSystem.hpp`
- Dessine les bounding boxes des entités si debug activé
- Affiche des informations de collision
- Contrôlé par `IRenderer::is_debug_bboxes_enabled()`
- **Composants** : `CollisionInfoComponent`

#### **InspectorSystem**
- **Fichier** : `include/mario/game/systems/InspectorSystem.hpp`
- Affiche les informations d'entités (debug)
- Permet l'inspection et le contrôle en temps réel
- **Composants** : Various

#### **HUD** (Affichage Haut Écran)
- **Fichier** : `include/mario/game/ui/HUD.hpp`
- Affiche les vies, pièces, score
- Affiche le timer du niveau
- Rendu via `IRenderer::draw_text()`
- **Composants** : N/A (accès direct au Game state)

---

## Flux de Données

### Cycle de Jeu (engine::Application::run())

```
1. Initialisation
   ├─ Game::initialize()
   ├─ engine::Application::initialize()
   ├─ PlayScene::on_enter()
   └─ AssetManager charge les assets

2. Boucle Principale (Fixed Timestep)
   ├─ IInput::poll()     [Lit clavier via InputManager]
   │
   ├─ IScene::update(dt)    [Met à jour logic (PlayScene)]
   │  ├─ PhysicsSystem::update(IEntityManager, dt)
   │  ├─ CollisionSystem::update(IEntityManager, dt)
   │  ├─ PlayerControllerSystem::update(IEntityManager, dt, IInput)
   │  ├─ EnemySystem::update(IEntityManager, dt)
   │  ├─ AnimationSystem::update(IEntityManager, dt)
   │  ├─ CameraSystem::update(IEntityManager, IRenderer)
   │  ├─ LevelSystem::handle_transitions()
   │  └─ ...
   │
   └─ IRenderer::begin_frame()
      ├─ BackgroundSystem::render(IEntityManager, IRenderer)
      ├─ SpriteRenderSystem::render(IEntityManager, IRenderer)
      ├─ DebugDrawSystem::render(IEntityManager, IRenderer) [If debug on]
      ├─ HUD::render(IRenderer)
      └─ IRenderer::end_frame()

3. Fin
   ├─ Current IScene::on_exit()
   └─ engine::Application::shutdown() – Cleanup resources
```

---

## Exemple: Cycle d'une Entité Joueur

```cpp
// 1. Création (dans PlayScene::on_enter ou Level::spawn)
EntityID player = entity_manager.create_entity();
entity_manager.add_component<PositionComponent>(player, {x: 5.0f, y: 10.0f});
entity_manager.add_component<SpriteComponent>(player, {sprite_id: PLAYER_SPRITE, ...});
entity_manager.add_component<VelocityComponent>(player, {vx: 0.0f, vy: 0.0f});
entity_manager.add_component<PlayerControllerComponent>(player, {...});
entity_manager.add_component<CollisionInfoComponent>(player, {width: 32.0f, height: 32.0f});
entity_manager.add_component<AnimationComponent>(player, {frame_index: 0, ...});

// 2. Update via PhysicsSystem
auto vel = entity_manager.get_component<VelocityComponent>(player);
vel->vy += gravity * dt;  // Apply gravity
auto pos = entity_manager.get_component<PositionComponent>(player);
pos->x += vel->vx * dt;
pos->y += vel->vy * dt;

// 3. Collision Detection (CollisionSystem)
auto collision = entity_manager.get_component<CollisionInfoComponent>(player);
// Check against tiles and other entities...

// 4. Animation Update (AnimationSystem)
auto anim = entity_manager.get_component<AnimationComponent>(player);
anim->frame_timer += dt;
if (anim->frame_timer >= anim->frame_duration) {
    anim->frame_index = (anim->frame_index + 1) % anim->max_frames;
    anim->frame_timer = 0.0f;
}

// 5. Render (SpriteRenderSystem)
auto sprite = entity_manager.get_component<SpriteComponent>(player);
auto transform = entity_manager.get_component<PositionComponent>(player);
renderer.draw_sprite(sprite->sprite_id, transform->x, transform->y);
```

---

## Principes SOLID Appliqués

✅ **Single Responsibility** : Chaque système = une responsabilité  
✅ **Open/Closed** : Ajouter des systèmes sans modifier Game  
✅ **Liskov Substitution** : GameState polymorphe  
✅ **Interface Segregation** : Chaque système n'accès que ses composants  
✅ **Dependency Inversion** : Game inject Renderer/InputManager/etc.

---

## Architecture ECS Avantages

- **Réutilisabilité** : Composants combinables (Sprite + Transform = n'importe quel objet)
- **Flexibilité** : Ajouter comportement = ajouter composants + système
- **Performance** : Cache-friendly (composants du même type = mémoire contiguë)
- **Testabilité** : Systèmes indépendants = testables isolément

---

## Dépendances Externes

- **SFML 3.0.2** : Rendu, fenêtre, son
- **C++17** : Smart pointers, optional, std::any, std::variant

---

## Points d'Entrée Clés

| Fichier | Rôle |
|---------|------|
| `main.cpp` | Entrée du programme |
| `include/mario/game/MarioGame.hpp` | Game wrapper pour l'application |
| `include/mario/engine/Application.hpp` | Boucle principale et gestion des scènes |
| `include/mario/game/PlayScene.hpp` | Scène de gameplay |
| `include/mario/engine/ecs/EntityManager.hpp` | Accès aux entités et composants |
| `include/mario/engine/render/Renderer.hpp` | Rendu final SFML |
| `include/mario/engine/input/InputManager.hpp` | Gestion des entrées clavier |
| `include/mario/engine/resources/AssetManager.hpp` | Gestion des assets (textures, fonts) |

---

## Patterns & Principes Clés

### Dependency Injection
- `Application` reçoit les interfaces `IRenderer`, `IInput`, `IAssetManager`, `IEntityManager`
- Permet le remplacement d'implémentations pour les tests ou extensions

### Adapter Pattern
- `engine::adapters::SceneAdapter` convertit `Scene` → `IScene`
- Permet aux scènes game-level de s'exécuter sur le moteur engine

### Composition over Inheritance
- Entités = collections de composants, pas hiérarchies de classes
- Systèmes opèrent sur les composants, découplant la logique des structures de données

### Type-Erased Storage
- `EntityManager` utilise `std::any` + `std::type_index` pour un stockage polymorphe
- Composants de tout type `T` peuvent être ajoutés sans modification du manager

---

## Voilà un schéma complet de votre architecture ! 🎮

