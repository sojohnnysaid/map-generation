#ifndef MAP_SHAPING_H
#define MAP_SHAPING_H

#include "map_data.h"

typedef enum {
    ISLAND_SHAPE_SQUARE,
    ISLAND_SHAPE_RADIAL
} IslandShapeType;

void shape_island(MapData* map, double mix_factor, IslandShapeType shape_type);

// --- New Function: Apply Terracing ---
// Rounds elevation values to create n distinct levels.
void apply_terraces(MapData* map, int num_levels);
// -----------------------------------

#endif // MAP_SHAPING_H
