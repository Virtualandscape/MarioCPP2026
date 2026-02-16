# Architecture du Projet Mario

## Vue d'ensemble

Le projet suit une **architecture ECS (Entity Component System)** combinée avec un **gestionnaire d'états (State Pattern)**. Voici comment les composants interagissent :

```
┌─────────────────────────────────────────────────────────────────┐
│                          MAIN LOOP                              │
│  Entrée : main() → Game::initialize() → Game::run()             │
└─────────────────────────────┬───────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                     Game (Core Application)                     │
│  • Owns: Renderer, InputManager, AssetManager, EntityManager    │
│  • Manages: Scene Stack (push/pop scenes)                       │
│  • Fixed timestep loop, frame throttling                        │
└─────────────────────────────┬───────────────────────────────────┘
                              │
          ┌───────────────────┼───────────────────┐
          │                   │                   │
          ▼                   ▼                   ▼
      ┌────────┐         ┌───────────┐      ┌──────────────┐
      │Renderer│         │InputMgr   │      │AssetManager  │
      │(SFML)  │         │(Keyboard) │      │(Textures)    │
      └────────┘         └───────────┘      └──────────────┘
          │                   │                   │
          └───────────────────┼───────────────────┘
                              │
                              ▼
                    ┌──────────────────────┐
                    │  EntityManager (ECS) │
                    │  • Create entities   │
                    │  • Add/Get components│
                    │  • Query by component│
                    └──────────────────────┘
                              │
          ┌───────────────────┼───────────────────┐
          │                   │                   │
          ▼                   ▼                   ▼
    ┌──────────────┐   ┌──────────────┐   ┌──────────────┐
    │Scene         │   │        Scene |   │   Scene      │
    │(Abstract)    │   │(Impl 1)      │   │(Impl 2)      │
    │ on_enter()   │   └──────────────┘   └──────────────┘
    │ on_exit()    │
    │ update(dt)   │
    │ render()     │
    └──────────────┘
          │
          ▼
    ┌──────────────────────────────────────┐
    │ PlayScene                            │
    │ • Owns: Level, Systems (Physics,     │
    │   Collision, etc.)                   │
    │ • Updates/Renders active level       │
    └──────────────────────────────────────┘
```

---

## Détail des Composants Principaux

### 1. **Game** (Cœur de l'Application)
- **Fichier** : `include/mario/core/Game.hpp`
- **Responsabilités** :
  - Initialise et gère la boucle principale (fixed timestep)
  - Détient les gestionnaires globaux (Renderer, InputManager, AssetManager, EntityManager)
  - Gère une pile d'états (State Stack pattern)
  - Appelle `update()` et `render()` sur la scène active
  
- **Relations** :
  - Possède (owns) : `Renderer`, `InputManager`, `AssetManager`, `EntityManager`
  - Gère : Stack de `Scene`

---

### 2. **Scene** (Pattern d'État)
- **Fichier** : `include/mario/core/Scene.hpp`
- **Responsabilités** :
  - Interface abstraite pour toutes les scènes du jeu
  - Définit le cycle de vie : `on_enter()`, `on_exit()`, `update(dt)`, `render()`
  
- **Implémentations** :
  - `PlayScene` / `WorldScene` : Gère la gameplay active
  - `MenuScene` : Menu principal
  - etc.

---

### 3. **Renderer** (Rendu)
- **Fichier** : `include/mario/render/Renderer.hpp`
- **Responsabilités** :
  - Gère la fenêtre SFML
  - Gère la caméra et les transformations de vue
  - Offre des méthodes de dessin : sprites, rectangles, texte, bbox debug
  - Gère le zoom et le parallax
  
- **Dépendances** :
  - SFML 3.0.2 (`sf::RenderWindow`, `sf::Texture`, `sf::Font`)

---

### 4. **InputManager** (Entrée Clavier)
- **Fichier** : `include/mario/input/InputManager.hpp`
- **Responsabilités** :
  - Poll les touches SFML
  - Traduit en actions abstraites : `MoveLeft`, `MoveRight`, `Jump`, `Escape`, `ToggleDebug`
  - Fournit `is_pressed(action)`
  
- **Utilité** :
  - Découplage entre entrée hardware et logic du jeu

---

### 5. **AssetManager** (Gestion des Ressources)
- **Fichier** : `include/mario/resources/AssetManager.hpp`
- **Responsabilités** :
  - Charge et met en cache les textures
  - Charge les sons
  - Libère les ressources
  
- **Utilité** :
  - Évite les chargements redondants
  - Smart pointers pour la gestion mémoire

---

### 6. **EntityManager** (ECS Core)
- **Fichier** : `include/mario/ecs/EntityManager.hpp`
- **Responsabilités** :
  - Crée les entités avec des IDs uniques
  - Ajoute/récupère des composants typés à une entité
  - Permet les requêtes d'entités par type de composant
  
- **Architecture ECS** :
  ```
  EntityID = uint32_t
  
  Entité = Collection de Composants
  
  Entity 1: [Transform, Sprite, Physics, PlayerControl]
  Entity 2: [Transform, Sprite, Physics, Enemy]
  Entity 3: [Transform, TileMap]
  ```

- **Composants Disponibles** :
  - `Transform` : position, vitesse, rotation
  - `Sprite` : rendu visuel
  - `Physics` : gravité, friction, vélocité
  - `Collision` : bounding box, réaction aux collisions
  - `PlayerControl` : entrée joueur
  - `Enemy` : logique IA ennemis
  - `Projectile` : munitions
  - etc.

---

### 7. **Level** (Gestion des Niveaux)
- **Fichier** : `include/mario/world/Level.hpp`
- **Responsabilités** :
  - Charge les tilemaps depuis des fichiers JSON
  - Gère la caméra du niveau
  - Stocke les spawn d'entités
  - Gère les couches de parallax
  - Gère les clouds
  
- **Relations** :
  - Détient : `TileMap`, `Camera`, `EntitySpawn[]`

---

### 8. **TileMap** (Grille de Tuiles)
- **Fichier** : `include/mario/world/TileMap.hpp`
- **Responsabilités** :
  - Charge et stocke la grille de tuiles
  - Parses les fichiers JSON de niveau
  - Fournit requêtes de collision : `is_solid(tx, ty)`
  - Gère les dimensions et le clamping
  
- **Format** :
  - `'0'` = tuile vide
  - `'1'` = tuile solide
  - `'G'` = spawn Goomba
  - `'K'` = spawn Koopa

---

### 9. **Camera** (Caméra de Jeu)
- **Fichier** : `include/mario/world/Camera.hpp`
- **Responsabilités** :
  - Suit le joueur (smooth follow)
  - Clamp les limites (ne pas sortir de la map)
  - Gère le viewport
  
- **Formule** :
  ```
  Chaque frame:
  camera_x = lerp(camera_x, target_x, follow_speed)
  camera_y = lerp(camera_y, target_y, follow_speed)
  ```

---

### 10. **Systèmes ECS** (Logic Métier)

Chaque système opère sur des entités avec des composants spécifiques :

#### **PhysicsSystem**
- Applique la gravité
- Met à jour la vélocité et position
- Intègre friction et accélération

#### **CollisionSystem**
- Détecte les collisions AABB
- Résout les pénétrations
- Déclenche les événements de collision

#### **PlayerControllerSystem**
- Lit les entrées
- Met à jour la vélocité du joueur
- Gère les sauts

#### **AnimationSystem**
- Met à jour les frames de sprite
- Gère les transitions d'animation

#### **SpriteRenderSystem**
- Dessine les sprites des entités
- Respecte l'ordre de layering (Z-order)

#### **EnemySystem**
- IA des ennemis (marche, patrouille)
- Détecte le joueur

#### **LevelSystem**
- Gère les transitions entre niveaux
- Détecte la victoire/défaite

#### **CameraSystem**
- Met à jour la caméra pour suivre le joueur
- Appelle `Renderer::set_camera()`

#### **BackgroundSystem** & **CloudSystem**
- Dessine les couches de parallax
- Gère les clouds animés

#### **DebugDrawSystem**
- Dessine les bounding boxes si debug activé

#### **HUD**
- Affiche les vies, pièces, score, timer

---

## Flux de Données

### Cycle de Jeu (Game::run())

```
1. Initialisation
   ├─ Game::initialize()
   ├─ Current Scene::on_enter()
   └─ AssetManager charge les assets

2. Boucle Principale (Fixed Timestep)
   ├─ InputManager::poll()     [Lit clavier]
   │
   ├─ Scene::update(dt)    [Met à jour logic]
   │  ├─ PhysicsSystem::update()
   │  ├─ CollisionSystem::update()
   │  ├─ PlayerControllerSystem::update()
   │  ├─ EnemySystem::update()
   │  ├─ AnimationSystem::update()
   │  ├─ CameraSystem::update()
   │  ├─ LevelSystem::handle_transitions()
   │  └─ ...
   │
   └─ Renderer::begin_frame()
      ├─ BackgroundSystem::render()
      ├─ SpriteRenderSystem::render()    [Dessine entités]
      ├─ DebugDrawSystem::render()       [If debug on]
      ├─ HUD::render()
      └─ Renderer::end_frame()

3. Fin
   ├─ Current Scene::on_exit()
   └─ Shutdown resources
```

---

## Exemple: Cycle d'une Entité Joueur

```cpp
// 1. Création (dans PlayScene)
EntityID player = entity_manager.create_entity();
entity_manager.add_component<Transform>(player, {x: 5, y: 10, ...});
entity_manager.add_component<Sprite>(player, {texture_id: PLAYER_SPRITE, ...});
entity_manager.add_component<Physics>(player, {velocity: {0,0}, ...});
entity_manager.add_component<PlayerControl>(player, {...});

// 2. Update (PhysicsSystem)
auto physics = entity_manager.get_component<Physics>(player);
physics->velocity.y += gravity * dt;
physics->position += physics->velocity * dt;

// 3. Render (SpriteRenderSystem)
auto sprite = entity_manager.get_component<Sprite>(player);
auto transform = entity_manager.get_component<Transform>(player);
renderer.draw_sprite(sprite->texture_id, transform->x, transform->y);
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
| `Game::run()` | Boucle principale |
| `Scene::update()` | Logic par frame |
| `EntityManager` | Accès aux entités |
| `Renderer::end_frame()` | Rendu final SFML |

---

Voilà un schéma complet de votre architecture ! 🎮

