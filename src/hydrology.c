#include "hydrology.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <stdbool.h> // Make sure bool is included

typedef struct {
    int x;
    int y;
} Point;

// --- Simple Queue Implementation for BFS ---
typedef struct {
    Point* points;
    int capacity;
    int size;
    int head;
    int tail;
} PointQueue;

PointQueue* create_queue(int capacity) {
    PointQueue* q = malloc(sizeof(PointQueue));
    if (!q) return NULL;
    q->points = malloc(capacity * sizeof(Point));
    if (!q->points) { free(q); return NULL; }
    q->capacity = capacity;
    q->size = 0;
    q->head = 0;
    q->tail = -1;
    return q;
}

void destroy_queue(PointQueue* q) {
    if (!q) return;
    free(q->points);
    free(q);
}

bool is_empty(PointQueue* q) {
    return q->size == 0;
}

bool is_full(PointQueue* q) {
    return q->size == q->capacity;
}

void enqueue(PointQueue* q, Point p) {
    if (is_full(q)) return; // Or resize
    q->tail = (q->tail + 1) % q->capacity;
    q->points[q->tail] = p;
    q->size++;
}

Point dequeue(PointQueue* q) {
    Point p = {-1, -1}; // Invalid point
    if (is_empty(q)) return p;
    p = q->points[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->size--;
    return p;
}
// --- End Queue Implementation ---


static Point find_downhill_neighbour(const MapData* map, int x, int y) {
   // ... (Implementation from previous step remains the same) ...
    Point best_neighbour = {x, y};
    double min_elevation = map->elevation[y][x];
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < map->width && ny >= 0 && ny < map->height) {
                double neighbour_elevation = map->elevation[ny][nx];
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
    // ... (Implementation from previous step remains the same) ...
     if (!map || !map->elevation) return;
     printf("Generating rivers (attempting %d)...\n", num_rivers);
     // srand() should ideally be called once in main
     int rivers_generated = 0;
     int width = map->width;
     int height = map->height;
     const double WATER_LEVEL_THRESHOLD = 0.18; // Use ELEV_BEACH
     for (int i = 0; i < num_rivers; ++i) {
         Point path[max_length];
         int path_len = 0;
         Point current_pos = {-1, -1};
         int start_attempts = 0;
         const int max_start_attempts = width * height / 10;
         while (start_attempts < max_start_attempts) { /* Find start */
             int sx = rand() % width; int sy = rand() % height;
             if (map->elevation[sy][sx] >= start_elevation_min) {
                 current_pos.x = sx; current_pos.y = sy; break;
             } start_attempts++;
         }
         if (current_pos.x == -1) continue;
         path[path_len++] = current_pos;
         while (path_len < max_length) { /* Trace path */
             Point next_pos = find_downhill_neighbour(map, current_pos.x, current_pos.y);
             if (next_pos.x == current_pos.x && next_pos.y == current_pos.y) break;
             if (map->elevation[next_pos.y][next_pos.x] < WATER_LEVEL_THRESHOLD) {
                 path[path_len++] = next_pos; break;
             }
             bool cycle = false; for(int k=0; k<path_len; ++k) if(path[k].x==next_pos.x && path[k].y==next_pos.y) cycle=true; if(cycle) break;
             path[path_len++] = next_pos; current_pos = next_pos;
         }
         if (path_len >= min_length) { /* Carve river */
             rivers_generated++;
             for (int j = 0; j < path_len; ++j) {
                 int px = path[j].x; int py = path[j].y;
                 map->elevation[py][px] = fmax(WATER_LEVEL_THRESHOLD * 0.8, map->elevation[py][px] * 0.90);
             }
         }
     }
     printf("River generation complete (%d rivers carved).\n", rivers_generated);
}


void fill_lakes(MapData* map, double ocean_level) {
    if (!map || !map->elevation) return;

    int width = map->width;
    int height = map->height;
    printf("Filling lakes (Ocean Level = %.4f)...\n", ocean_level);

    bool** visited = malloc(height * sizeof(bool*));
    if (!visited) { perror("Failed to allocate visited rows"); return; }
    for (int y = 0; y < height; ++y) {
        visited[y] = calloc(width, sizeof(bool)); // Use calloc to initialize to false
        if (!visited[y]) {
            perror("Failed to allocate visited columns");
            for(int i=0; i < y; ++i) free(visited[i]);
            free(visited);
            return;
        }
    }

    PointQueue* q = create_queue(width * height); // Max possible size needed
    PointQueue* pit_cells = create_queue(width * height); // Store cells in the current pit
    if (!q || !pit_cells) {
        fprintf(stderr, "Failed to create queues for lake filling.\n");
        // Cleanup visited array
        for(int y=0; y < height; ++y) free(visited[y]);
        free(visited);
        destroy_queue(q); // Safe to call on NULL
        destroy_queue(pit_cells);
        return;
    }


    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (visited[y][x]) continue; // Skip already processed cells

            double current_elevation = map->elevation[y][x];
            visited[y][x] = true;

            // Start BFS only if this cell is potentially part of an inland depression
            // (It must be below ocean level, otherwise it drains naturally)
            if (current_elevation < ocean_level) {

                Point start_point = {x, y};
                double min_spill_point = DBL_MAX; // Lowest point found on the rim
                bool reached_ocean = false;
                int current_pit_size = 0;

                // Reset queues for this potential pit
                q->size = 0; q->head = 0; q->tail = -1;
                pit_cells->size = 0; pit_cells->head = 0; pit_cells->tail = -1;

                enqueue(q, start_point);
                enqueue(pit_cells, start_point); // Add start to pit list


                // BFS to find extent of depression and minimum spill point
                while (!is_empty(q)) {
                    Point current = dequeue(q);

                    // Check 8 neighbours
                    for (int dy = -1; dy <= 1; ++dy) {
                        for (int dx = -1; dx <= 1; ++dx) {
                            if (dx == 0 && dy == 0) continue;

                            int nx = current.x + dx;
                            int ny = current.y + dy;

                            // Check bounds
                            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                                if (!visited[ny][nx]) {
                                    visited[ny][nx] = true; // Mark neighbour visited
                                    double neighbour_elevation = map->elevation[ny][nx];

                                    if (neighbour_elevation < ocean_level) {
                                        // Part of the same potential depression, add to queue
                                        enqueue(q, (Point){nx, ny});
                                        enqueue(pit_cells, (Point){nx, ny}); // Add to pit list
                                    } else {
                                        // Hit the rim (or land above ocean level)
                                        // This neighbour is a potential spill point
                                        if (neighbour_elevation < min_spill_point) {
                                            min_spill_point = neighbour_elevation;
                                        }
                                    }
                                }
                                // Check if neighbour *is* ocean (special case of rim)
                                // This check is subtle - if any explored cell is *adjacent* to ocean
                                // it means the depression connects to the ocean. We might need
                                // to pre-mark ocean cells as visited?
                                // Let's simplify: if min_spill_point remains DBL_MAX or goes below ocean,
                                // assume it connects or is ocean itself.


                            } else {
                                // Hit the map edge, assume this connects to the ocean
                                reached_ocean = true;
                                // Technically, need to handle edge cases better if map isn't island-shaped
                            }
                        } // end dx loop
                    } // end dy loop
                } // end while BFS queue not empty

                // Decide whether to fill the pit
                // Fill if we didn't reach the map edge/ocean and found a valid spill point > current elevation
                if (!reached_ocean && min_spill_point < DBL_MAX && min_spill_point > current_elevation) {
                    // Fill all cells in pit_cells up to min_spill_point
                    // printf("Filling pit starting at (%d, %d) to level %.4f (%d cells)\n", x, y, min_spill_point, pit_cells->size);
                    while (!is_empty(pit_cells)) {
                        Point pit_cell = dequeue(pit_cells);
                        // Only raise elevation if it's currently below the spill point
                        if (map->elevation[pit_cell.y][pit_cell.x] < min_spill_point) {
                             map->elevation[pit_cell.y][pit_cell.x] = min_spill_point;
                        }
                    }
                } else {
                    // This wasn't a contained pit, just dequeue remaining pit cells
                     while (!is_empty(pit_cells)) { (void)dequeue(pit_cells); }
                }

            } // end if potential pit start
        } // end x loop
    } // end y loop


    // Cleanup
    for(int y=0; y < height; ++y) free(visited[y]);
    free(visited);
    destroy_queue(q);
    destroy_queue(pit_cells);

    printf("Lake filling complete.\n");
}
