#ifndef MAP_IO_H
#define MAP_IO_H

#include "map_data.h"

void print_map_text(const MapData* map);

// --- Changed to PNG ---
// Writes the map biome colors to a PNG file.
// Returns 0 on success, non-zero on error.
int write_map_png(const MapData* map, const char* filename);
// -----------------------

#endif // MAP_IO_H
