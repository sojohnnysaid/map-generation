#include "map_shaping.h"
#include <math.h>   // For sqrt, fabs, pow
#include <stdio.h>  // For printf

// Helper function for linear interpolation
static inline double lerp(double a, double b, double t) {
    return a * (1.0 - t) + b * t;
}

// Helper function: Clamp value between min and max
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

    // Clamp mix_factor to [0, 1] range
    mix_factor = clamp(mix_factor, 0.0, 1.0);

    if (mix_factor == 0.0) {
        printf("Island shaping mix factor is 0.0, skipping.\n");
        return; // No shaping needed
    }

    printf("Applying island shape (mix=%.2f, type=%d)...\n", mix_factor, shape_type);

    int width = map->width;
    int height = map->height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            // 1. Calculate normalized coordinates (-1 to +1 range)
            // Ensure floating point division
            double nx = (2.0 * x / (width - 1.0)) - 1.0;
            double ny = (2.0 * y / (height - 1.0)) - 1.0;
             // Handle edge case for 1-pixel wide/high maps to avoid division by zero
            if (width == 1) nx = 0.0;
            if (height == 1) ny = 0.0;


            // 2. Calculate distance 'd' (0 at center, >=1 at edges)
            double d = 0.0;
            switch (shape_type) {
                case ISLAND_SHAPE_SQUARE:
                    // d = 1 - (1-nx²) * (1-ny²)
                    // Ensure nx*nx doesn't exceed 1 if using floats slightly > 1
                    nx = clamp(nx, -1.0, 1.0);
                    ny = clamp(ny, -1.0, 1.0);
                    d = 1.0 - (1.0 - nx * nx) * (1.0 - ny * ny);
                    break;

                case ISLAND_SHAPE_RADIAL:
                default:
                    // Simple radial distance, normalized roughly to map corners
                    // Max distance is sqrt(1^2 + 1^2) = sqrt(2) at corners
                    d = sqrt(nx * nx + ny * ny);
                    // Optional: Normalize d to be closer to 1 at edge midpoints
                    // d /= sqrt(2.0); // Makes d=1 at corners, <1 elsewhere on edge
                    break; // Use raw distance for now, max sqrt(2)
            }
            // Clamp distance value just in case, ensure it's non-negative
            d = clamp(d, 0.0, 2.0); // Max can be sqrt(2) for radial, 1 for square

            // 3. Determine target elevation based on distance (1 - d, lower at edges)
            // We want elevation to be high (1.0) at center (d=0)
            // and low (0.0) at edges (d=1 for square, d=sqrt(2) for radial corner)
            // Let's try a simple gradient based on 'd' clamped to [0,1]
            double target_elevation = 1.0 - clamp(d, 0.0, 1.0);

            // Optionally make the gradient steeper
            // target_elevation = pow(target_elevation, 1.5); // Make edges drop off faster


            // 4. Interpolate current elevation towards target elevation
            double current_elevation = map->elevation[y][x];
            map->elevation[y][x] = lerp(current_elevation, target_elevation, mix_factor);

             // Ensure elevation remains valid [0, 1] after lerp (shouldn't happen if inputs are valid)
             map->elevation[y][x] = clamp(map->elevation[y][x], 0.0, 1.0);
        }
    }

    printf("Island shaping complete.\n");
}
