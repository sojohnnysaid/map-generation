#ifndef MAP_DATA_H
#define MAP_DATA_H

#include <stdbool.h> // Needed for bool

// Forward declaration
typedef struct NoiseState NoiseState;

typedef struct {
    double x;
    double y;
    double z;
} Vector3;

typedef struct {
    int width;
    int height;
    double **elevation;
    double **moisture;
    bool **is_river;
    Vector3 **normals;  // Added normals for 3D effect
    double **detail;    // Added high-frequency detail layer
} MapData;

MapData* create_map(int width, int height);
void destroy_map(MapData* map);
void redistribute_map(MapData* map, double exponent);
void calculate_normals(MapData* map);
void generate_detail_map(MapData* map, NoiseState* noise, int seed);

#endif // MAP_DATA_H
