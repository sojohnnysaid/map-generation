#ifndef MAP_DATA_H
#define MAP_DATA_H

typedef struct {
    int width;
    int height;
    double **elevation;
    double **moisture;
} MapData;

MapData* create_map(int width, int height);
void destroy_map(MapData* map);
void redistribute_map(MapData* map, double exponent);

#endif // MAP_DATA_H
