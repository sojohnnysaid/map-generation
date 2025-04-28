#include "hydrology.h"
#include <stdio.h>
#include <stdlib.h> // For rand(), malloc(), free()
#include <float.h>  // For DBL_MAX

// Helper struct to store coordinates
typedef struct {
    int x;
    int y;
} Point;

// Find the lowest neighbour (including diagonals)
// Returns Point{-1, -1} if current cell is lowest or on boundary error
static Point find_downhill_neighbour(const MapData* map, int x, int y) {
    Point lowest = {-1, -1};
    double min_elev = map->elevation[y][x];

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue; // Skip self

            int nx = x + dx;
            int ny = y + dy;

            // Check boundaries
            if (nx >= 0 && nx < map->width && ny >= 0 && ny < map->height) {
                if (map->elevation[ny][nx] < min_elev) {
                    min_elev = map->elevation[ny][nx];
                    lowest.x = nx;
                    lowest.y = ny;
                }
            }
        }
    }
    return lowest;
}


void generate_rivers(MapData* map, int num_rivers, int min_length, int max_length) {
    if (!map || !map->elevation || !map->is_river) return;

    int width = map->width;
    int height = map->height;
    int rivers_generated = 0;
    int attempts = 0;
    const int max_attempts = num_rivers * 10; // Try harder to find start points

    // Define water level slightly above ocean level for termination condition
    const double water_level = 0.18; // Use a value >= ELEV_BEACH from map_io.c

    printf("Generating up to %d rivers (min_len=%d, max_len=%d)...\n",
           num_rivers, min_length, max_length);

    // Temporary storage for the current river path
    Point* path = malloc(max_length * sizeof(Point));
    if (!path) {
        perror("Failed to allocate memory for river path");
        return;
    }

    while (rivers_generated < num_rivers && attempts < max_attempts) {
        attempts++;

        // 1. Find a random starting point on land
        int startX = rand() % width;
        int startY = rand() % height;

        // Ensure start point is suitable land (not water, not too low)
        if (map->elevation[startY][startX] <= water_level || map->is_river[startY][startX]) {
            continue; // Try another random point
        }

        // 2. Trace path downhill
        int path_len = 0;
        int currX = startX;
        int currY = startY;
        bool reached_water = false;

        while (path_len < max_length) {
            // Check if already river or water before adding to path
            if (map->is_river[currY][currX] || map->elevation[currY][currX] <= water_level) {
                 if (map->elevation[currY][currX] <= water_level) {
                     reached_water = true; // Mark as successful termination if it hits water
                 }
                 break; // Stop if we hit existing river or water body
            }

            path[path_len].x = currX;
            path[path_len].y = currY;
            path_len++;

            Point next = find_downhill_neighbour(map, currX, currY);

            if (next.x == -1) { // Reached a pit or flat area
                break;
            }

            // Move to the next point
            currX = next.x;
            currY = next.y;
        }

        // 3. Mark path as river if long enough
        if (path_len >= min_length) {
            rivers_generated++;
            // printf("  Generated river %d (length %d)\n", rivers_generated, path_len);
            for (int i = 0; i < path_len; ++i) {
                map->is_river[path[i].y][path[i].x] = true;
                // Optional: Slightly lower elevation to carve visually?
                // map->elevation[path[i].y][path[i].x] *= 0.99; // Example
            }
        }
    }

    free(path); // Free temporary path storage
    printf("River generation complete. Generated %d rivers after %d attempts.\n", rivers_generated, attempts);
}
