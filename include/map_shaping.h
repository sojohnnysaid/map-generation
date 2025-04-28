#ifndef MAP_SHAPING_H
#define MAP_SHAPING_H

#include "map_data.h" // Needed for MapData pointer

// Removed IslandShapeType enum
// Removed shape_island function declaration

// --- New Function: Apply Continent Mask ---
// Modifies map->elevation based on values from a separate continent noise map.
// map:           Pointer to the main map data (contains elevation to modify)
// continent_map: A 2D array (double**) holding the low-frequency continent noise values [0, 1]
// width, height: Dimensions of the maps
// land_threshold: Value in continent_map above which is considered land potential
void apply_continent_mask(MapData* map, double** continent_map, int width, int height, double land_threshold);
// ----------------------------------------

// Applies terracing effect to map elevations.
void apply_terraces(MapData* map, int num_levels);

#endif // MAP_SHAPING_H
