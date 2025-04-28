#include "hydrology.h"
#include <stdio.h>
#include <stdlib.h> // For rand, srand
#include <float.h>  // For DBL_MAX
#include <math.h>   // For fmax
#include <time.h>   // For seeding srand

// Simple structure to hold coordinates
typedef struct {
    int x;
    int y;
} Point;

// Helper: Find the neighbour with the strictly lowest elevation
static Point find_downhill_neighbour(const MapData* map, int x, int y) {
    Point best_neighbour = {x, y}; // Default to current point (indicates no downhill)
    double min_elevation = map->elevation[y][x];

    // Check 8 neighbours (dx, dy are offsets)
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue; // Skip self

            int nx = x + dx;
            int ny = y + dy;

            // Check bounds
            if (nx >= 0 && nx < map->width && ny >= 0 && ny < map->height) {
                double neighbour_elevation = map->elevation[ny][nx];
                // Found a strictly lower neighbour
                if (neighbour_elevation < min_elevation) {
                    min_elevation = neighbour_elevation;
                    best_neighbour.x = nx;
                    best_neighbour.y = ny;
                }
            }
        }
    }
    return best_neighbour;
}


void generate_rivers(MapData* map, int num_rivers, int min_length, int max_length, double start_elevation_min) {
    if (!map || !map->elevation) return;

    printf("Generating rivers (attempting %d)...\n", num_rivers);
    srand((unsigned int)time(NULL)); // Seed random number generator

    int rivers_generated = 0;
    int width = map->width;
    int height = map->height;

    // Define water level threshold locally (could pass as param or get from map_io)
    // Using a value slightly above ocean to ensure flow terminates correctly
    const double WATER_LEVEL_THRESHOLD = 0.18; // Use ELEV_BEACH value

    for (int i = 0; i < num_rivers; ++i) {
        Point path[max_length]; // Static array for path storage per river
        int path_len = 0;
        Point current_pos = {-1, -1};

        // Try to find a suitable starting point
        int start_attempts = 0;
        const int max_start_attempts = width * height / 10; // Try roughly 10% of cells

        while (start_attempts < max_start_attempts) {
            int sx = rand() % width;
            int sy = rand() % height;
            if (map->elevation[sy][sx] >= start_elevation_min) {
                current_pos.x = sx;
                current_pos.y = sy;
                break;
            }
            start_attempts++;
        }

        if (current_pos.x == -1) {
            // printf("Failed to find suitable river start point after %d attempts.\n", max_start_attempts);
            continue; // Try next river
        }

        // Trace the path downhill
        path[path_len++] = current_pos;

        while (path_len < max_length) {
            Point next_pos = find_downhill_neighbour(map, current_pos.x, current_pos.y);

            // Check stopping conditions
            if (next_pos.x == current_pos.x && next_pos.y == current_pos.y) {
                // Stuck in a pit or flat area
                break;
            }
            if (map->elevation[next_pos.y][next_pos.x] < WATER_LEVEL_THRESHOLD) {
                // Reached water
                path[path_len++] = next_pos; // Add final water point
                break;
            }

            // Check for cycles (optional but good) - simplistic check
            bool cycle_detected = false;
            for(int k=0; k < path_len; ++k) {
                if(path[k].x == next_pos.x && path[k].y == next_pos.y) {
                    cycle_detected = true;
                    break;
                }
            }
            if (cycle_detected) break;


            // Add to path and continue
            path[path_len++] = next_pos;
            current_pos = next_pos;
        }

        // Process the generated path
        if (path_len >= min_length) {
            rivers_generated++;
            // printf("Carving river %d (length %d)\n", rivers_generated, path_len);
            // "Carve" the river by lowering elevation
            for (int j = 0; j < path_len; ++j) {
                int px = path[j].x;
                int py = path[j].y;
                // Lower elevation, ensuring it doesn't go below a base level
                double current_elev = map->elevation[py][px];
                // Make river slightly lower than original, but above deep ocean
                map->elevation[py][px] = fmax(WATER_LEVEL_THRESHOLD * 0.8, current_elev * 0.90);
                 // Could also slightly widen the river by affecting neighbours
            }
        }
    } // End loop for num_rivers

    printf("River generation complete (%d rivers carved).\n", rivers_generated);
}
