#ifndef HYDROLOGY_H
#define HYDROLOGY_H

#include "map_data.h"

// Generates rivers on the map by tracing downhill paths.
// Modifies map->elevation along river paths.
void generate_rivers(MapData* map,
                     int num_rivers,              // Number of rivers to attempt generating
                     int min_length,              // Minimum path length to count as a river
                     int max_length,              // Maximum path length (safety break)
                     double start_elevation_min); // Minimum elevation for a river source

#endif // HYDROLOGY_H
