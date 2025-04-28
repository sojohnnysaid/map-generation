#ifndef MAP_SHAPING_H
#define MAP_SHAPING_H

#include "map_data.h"

// Enum to select distance calculation method
typedef enum {
    ISLAND_SHAPE_SQUARE,   // Square bump function from tutorial
    ISLAND_SHAPE_RADIAL    // Simple radial distance (sqrt(nx^2+ny^2))
                           // Euclidean^2 from tutorial seemed possibly incorrect? Let's use radial.
} IslandShapeType;

// Applies an island shape constraint to the map's elevation
// map:        Pointer to the map data
// mix_factor: How strongly to apply the shape (0.0 = no effect, 1.0 = full override)
// shape_type: Which distance function to use (ISLAND_SHAPE_SQUARE or ISLAND_SHAPE_RADIAL)
void shape_island(MapData* map, double mix_factor, IslandShapeType shape_type);


#endif // MAP_SHAPING_H
