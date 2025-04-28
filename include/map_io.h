#ifndef MAP_IO_H
#define MAP_IO_H

#include "map_data.h"

// --- Updated Signatures ---
void print_map_text(const MapData* map, double latitude_temp_factor);
int write_map_png(const MapData* map, const char* filename, double latitude_temp_factor);
// ------------------------

#endif // MAP_IO_H
