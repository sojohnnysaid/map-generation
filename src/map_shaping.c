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

// --- New Implementation: Apply Continent Mask ---
void apply_continent_mask(MapData* map, double** continent_map, int width, int height, double land_threshold) {
    if (!map || !map->elevation || !continent_map) {
        fprintf(stderr, "Error: Cannot apply continent mask with NULL inputs.\n");
        return;
    }
    if (width <= 0 || height <= 0) {
         fprintf(stderr, "Error: Invalid dimensions for continent mask.\n");
         return;
    }

    printf("Applying continent mask (land threshold = %.2f)...\n", land_threshold);

    // Define how deep the ocean should be forced
    const double ocean_depth_target = 0.05; // Force below beach level

    for (int y = 0; y < height; y++) {
        // Check row validity (optional but safer)
        if (!map->elevation[y] || !continent_map[y]) {
             fprintf(stderr, "Error: Row %d is NULL in elevation or continent map.\n", y);
             continue;
        }
        for (int x = 0; x < width; x++) {
            double continent_val = continent_map[y][x];
            double current_elev = map->elevation[y][x];

            // If continent mask value is below threshold, force elevation down
            if (continent_val < land_threshold) {
                // Option 1: Force to a specific depth
                map->elevation[y][x] = ocean_depth_target;

                // Option 2: Blend towards depth based on how far below threshold
                // double blend_factor = 1.0 - (continent_val / land_threshold); // 0 at threshold, 1 at 0
                // map->elevation[y][x] = lerp(current_elev, ocean_depth_target, blend_factor * blend_factor); // Stronger blend further out
            } else {
                // Option 3: On land, maybe slightly boost elevation? Optional.
                // map->elevation[y][x] = current_elev * 1.05; // Example boost
            }

            // Ensure final elevation is clamped (important if boosting/blending)
            map->elevation[y][x] = clamp(map->elevation[y][x], 0.0, 1.0);
        }
    }

    printf("Continent mask application complete.\n");
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
