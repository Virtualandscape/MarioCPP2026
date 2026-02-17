#ifndef QUADTREE_H
#define QUADTREE_H

// Forward compatibility wrapper: quadtree was moved to engine::spatial::Quadtree
#include "Zia/engine/spatial/Quadtree.hpp"

// Provide the old name in the global namespace as an alias to minimize changes.
// Note: we intentionally keep the old quadtree name for source compatibility.
namespace zia {
    using quadtree = engine::spatial::Quadtree;
}

#endif // QUADTREE_H

