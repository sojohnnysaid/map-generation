#ifndef MAP_DATA_H
#define MAP_DATA_H

typedef struct {
    int width;
    int height;
    double **elevation; // Using double for elevation [0.0, 1.0]
    // Add moisture later if needed: double **moisture;
} MapData;

// Function prototypes
MapData* create_map(int width, int height);
void destroy_map(MapData* map);
void fill_map_constant(MapData* map, double value); // Helper to fill grid

// --- New Declaration ---
// Applies the pow(elevation, exponent) function to the map's elevation data
void redistribute_map(MapData* map, double exponent);
// -----------------------

#endif // MAP_DATA_H
