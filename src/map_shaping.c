#include "map_shaping.h"
#include <math.h>
#include <stdio.h>
#include <float.h>

static inline double lerp(double a, double b, double t) {
    return a * (1.0 - t) + b * t;
}

static inline double clamp(double value, double min_val, double max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

void shape_island(MapData* map, double mix_factor, IslandShapeType shape_type) {
    if (!map || !map->elevation) {
        fprintf(stderr, "Error: Cannot shape island on NULL map.\n");
        return;
    }

    mix_factor = clamp(mix_factor, 0.0, 1.0);
    if (mix_factor == 0.0) {
        printf("Island shaping mix factor is 0.0, skipping.\n");
        return;
    }

    printf("Applying island shape (mix=%.2f, type=%d)...\n", mix_factor, shape_type);

    int width = map->width;
    int height = map->height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double nx = (2.0 * x / (width - 1.0)) - 1.0;
            double ny = (2.0 * y / (height - 1.0)) - 1.0;
            if (width == 1) nx = 0.0;
            if (height == 1) ny = 0.0;

            double d = 0.0;
            switch (shape_type) {
                case ISLAND_SHAPE_SQUARE:
                    nx = clamp(nx, -1.0, 1.0);
                    ny = clamp(ny, -1.0, 1.0);
                    d = 1.0 - (1.0 - nx * nx) * (1.0 - ny * ny);
                    break;
                case ISLAND_SHAPE_RADIAL:
                default:
                    d = sqrt(nx * nx + ny * ny);
                    break;
            }
            d = clamp(d, 0.0, 2.0);
            double target_elevation = 1.0 - clamp(d, 0.0, 1.0);
            double current_elevation = map->elevation[y][x];
            map->elevation[y][x] = lerp(current_elevation, target_elevation, mix_factor);
            map->elevation[y][x] = clamp(map->elevation[y][x], 0.0, 1.0);
        }
    }

    printf("Island shaping complete.\n");
}

void apply_terraces(MapData* map, int num_levels) {
    if (!map || !map->elevation) {
        fprintf(stderr, "Error: Cannot apply terraces to NULL map.\n");
        return;
    }
    if (num_levels < 2) {
        fprintf(stderr, "Warning: Terracing with less than 2 levels requested (%d). Setting to 2.\n", num_levels);
        num_levels = 2;
    }

    printf("Applying terracing with %d levels...\n", num_levels);

    double levels_minus_one = (double)(num_levels - 1);

    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            double e = map->elevation[y][x];
            double scaled_e = e * levels_minus_one;
            double rounded_level = round(scaled_e);
            double terraced_e = rounded_level / levels_minus_one;
            map->elevation[y][x] = clamp(terraced_e, 0.0, 1.0);
        }
    }

    printf("Terracing complete.\n");
}
