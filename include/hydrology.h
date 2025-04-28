#ifndef HYDROLOGY_H
#define HYDROLOGY_H

#include "map_data.h"
#include <stdbool.h> // Make sure bool is available

void generate_rivers(MapData* map,
                     int num_rivers,
                     int min_length,
                     int max_length,
                     double start_elevation_min);

// --- New Function: Fill Lakes ---
// Identifies and fills depressions (pits) in the terrain.
// Modifies map->elevation to create flat lake surfaces.
void fill_lakes(MapData* map, double ocean_level);
// --------------------------------

#endif // HYDROLOGY_H
