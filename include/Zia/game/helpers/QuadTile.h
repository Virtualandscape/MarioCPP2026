#ifndef GAME_QUADTILE_H
#define GAME_QUADTILE_H

// QuadTile was moved to the engine spatial namespace; include the authoritative header.
#include "Zia/engine/spatial/QuadTile.h"

// Re-export into the zia global namespace for compatibility.
namespace zia {
    using ::QuadTile; // global struct from engine header is in the global namespace; keep compatibility.
}

#endif // GAME_QUADTILE_H
