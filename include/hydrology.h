#ifndef HYDROLOGY_H
#define HYDROLOGY_H

#include "map_data.h"

// Generates rivers by tracing paths downhill.
// Modifies map->is_river flags.
void generate_rivers(MapData* map, int num_rivers, int min_length, int max_length);

// Note: Lake filling could be added here later

#endif // HYDROLOGY_H
