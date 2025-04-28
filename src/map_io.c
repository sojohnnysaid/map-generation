#include "map_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef struct { unsigned char r, g, b; } RGBColor;

// Enhanced color palette with more gradations
const RGBColor COLOR_DEEP_OCEAN      = { 26,  42,  95};
const RGBColor COLOR_OCEAN           = { 36,  61, 139};
const RGBColor COLOR_SHALLOW_OCEAN   = { 67, 101, 165};
const RGBColor COLOR_LAKE_WATER      = { 93, 173, 226};
const RGBColor COLOR_BEACH           = {247, 220, 111};
const RGBColor COLOR_FOREST_GREEN    = { 39, 174,  96};
const RGBColor COLOR_JUNGLE_GREEN    = { 46, 204, 113};
const RGBColor COLOR_GRASS           = { 88, 214, 141};
const RGBColor COLOR_SAVANNAH        = {212, 172,  13};
const RGBColor COLOR_SHRUBLAND       = {241, 196,  15};
const RGBColor COLOR_TAIGA           = { 93, 173, 226};
const RGBColor COLOR_TUNDRA          = {169, 204, 227};
const RGBColor COLOR_SNOW            = {236, 240, 241};
const RGBColor COLOR_SCORCHED        = {192,  57,  43};
const RGBColor COLOR_BARE_ROCK       = {149, 165, 166};
const RGBColor COLOR_DESERT_SAND     = {210, 180, 140};
const RGBColor COLOR_MOUNTAIN_DARK   = {120, 120, 120};
const RGBColor COLOR_MOUNTAIN_LIGHT  = {180, 180, 180};

// Elevation thresholds (slightly modified for better gradients)
#define ELEV_DEEP_OCEAN      0.10
#define ELEV_OCEAN           0.15
#define ELEV_LAKE_MAX        0.18
#define ELEV_BEACH           0.18
#define ELEV_LOWLAND         0.30
#define ELEV_TROPICAL_MAX    0.40
#define ELEV_TEMPERATE_MAX   0.65
#define ELEV_BOREAL_MAX      0.85
#define ELEV_MOUNTAIN        0.90

// Moisture thresholds
#define MOIST_DESERT         0.20
#define MOIST_GRASS_SAVANNAH 0.40
#define MOIST_WOODLAND_SHRUB 0.70

// Lighting parameters
#define LIGHT_DIR_X          0.7
#define LIGHT_DIR_Y          0.3
#define LIGHT_DIR_Z          0.8
#define AMBIENT_LIGHT        0.3
#define DIFFUSE_STRENGTH     0.6
#define SPECULAR_STRENGTH    0.2
#define SPECULAR_POWER       4.0

// ANSI color codes for console output
#define ANSI_BG_DEEP_BLUE    "\x1b[48;5;17m"  // darker blue
#define ANSI_BG_LAKE_WATER   "\x1b[46m"
#define ANSI_BG_MEDIUM_BLUE  "\x1b[44m"   // medium blue for shallower ocean
#define ANSI_BG_SAND         "\x1b[103m"
#define ANSI_BG_FOREST_GREEN "\x1b[42m"
#define ANSI_BG_JUNGLE_GREEN "\x1b[102m"
#define ANSI_BG_GRASS        "\x1b[42m"
#define ANSI_BG_SAVANNAH     "\x1b[43m"
#define ANSI_BG_SHRUBLAND    "\x1b[103m"
#define ANSI_BG_TAIGA        "\x1b[46m"
#define ANSI_BG_TUNDRA       "\x1b[106m"
#define ANSI_BG_SNOW         "\x1b[107m"
#define ANSI_BG_SCORCHED     "\x1b[41m"
#define ANSI_BG_BARE_ROCK    "\x1b[100m"
#define ANSI_BG_DESERT_SAND  "\x1b[43m"
#define ANSI_BG_SHALLOW_OCEAN "\x1b[104m"
#define ANSI_RESET           "\x1b[0m"

// Utility functions
static inline double clamp_io(double value, double min_val, double max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

static inline RGBColor blend_colors(RGBColor a, RGBColor b, double t) {
    t = clamp_io(t, 0.0, 1.0);
    RGBColor result;
    result.r = (unsigned char)(a.r * (1.0 - t) + b.r * t);
    result.g = (unsigned char)(a.g * (1.0 - t) + b.g * t);
    result.b = (unsigned char)(a.b * (1.0 - t) + b.b * t);
    return result;
}

// Function to apply lighting based on normal vectors
static RGBColor apply_lighting(RGBColor base_color, Vector3 normal, double height_factor) {
    // Normalize light direction vector
    double light_len = sqrt(LIGHT_DIR_X*LIGHT_DIR_X + LIGHT_DIR_Y*LIGHT_DIR_Y + LIGHT_DIR_Z*LIGHT_DIR_Z);
    double light_x = LIGHT_DIR_X / light_len;
    double light_y = LIGHT_DIR_Y / light_len;
    double light_z = LIGHT_DIR_Z / light_len;
    
    // Calculate diffuse lighting component (dot product of normal and light direction)
    double diffuse = normal.x * light_x + normal.y * light_y + normal.z * light_z;
    diffuse = clamp_io(diffuse, 0.0, 1.0);
    
    // Simplified specular component (for high points like mountains)
    double specular = pow(diffuse, SPECULAR_POWER) * height_factor * SPECULAR_STRENGTH;
    
    // Calculate final lighting factor
    double lighting = AMBIENT_LIGHT + diffuse * DIFFUSE_STRENGTH + specular;
    
    // Apply lighting to color
    RGBColor lit_color;
    lit_color.r = (unsigned char)clamp_io(base_color.r * lighting, 0, 255);
    lit_color.g = (unsigned char)clamp_io(base_color.g * lighting, 0, 255);
    lit_color.b = (unsigned char)clamp_io(base_color.b * lighting, 0, 255);
    
    return lit_color;
}

void print_map_text(const MapData* map, double latitude_temp_factor) {
    if (!map || !map->elevation || !map->moisture) { return; }

    printf("--- Map (%dx%d) ---\n", map->width, map->height);
    int width = map->width;
    int height = map->height;

    for (int y = 0; y < height; y += 2) { // Skip every other row for more square-looking output
        double latitude_norm = (double)y / (height > 1 ? height - 1 : 1);
        double dist_from_equator = fabs(latitude_norm - 0.5) * 2.0;

        for (int x = 0; x < width; x += 2) { // Skip every other column for more square-looking output
            double e_orig = map->elevation[y][x];
            double m = map->moisture[y][x];
            double e_effective = clamp_io(e_orig + latitude_temp_factor * dist_from_equator, 0.0, 1.0);
            const char* bg_color_code;

            if (e_effective < ELEV_DEEP_OCEAN) {
                bg_color_code = ANSI_BG_DEEP_BLUE;
            }
            else if (e_effective < ELEV_OCEAN) {
                bg_color_code = ANSI_BG_DEEP_BLUE;
            }
            else if (e_effective < ELEV_BEACH) {
                 bg_color_code = ANSI_BG_LAKE_WATER;
            }
            else if (e_effective > ELEV_MOUNTAIN) {
                bg_color_code = ANSI_BG_SNOW;  // Mountain peaks
            }
            else if (e_effective > ELEV_BOREAL_MAX) { 
                if (m < MOIST_DESERT) bg_color_code = ANSI_BG_SCORCHED; 
                else if (m < MOIST_GRASS_SAVANNAH) bg_color_code = ANSI_BG_BARE_ROCK; 
                else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_TUNDRA; 
                else bg_color_code = ANSI_BG_SNOW;
            }
            else if (e_effective > ELEV_TEMPERATE_MAX) { 
                if (m < MOIST_DESERT) bg_color_code = ANSI_BG_SAVANNAH; 
                else if (m < MOIST_GRASS_SAVANNAH) bg_color_code = ANSI_BG_SHRUBLAND; 
                else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_TAIGA; 
                else bg_color_code = ANSI_BG_TAIGA; 
            }
            else if (e_effective > ELEV_TROPICAL_MAX) { 
                if (m < MOIST_DESERT) bg_color_code = ANSI_BG_SAVANNAH; 
                else if (m < MOIST_GRASS_SAVANNAH) bg_color_code = ANSI_BG_GRASS; 
                else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_FOREST_GREEN; 
                else bg_color_code = ANSI_BG_FOREST_GREEN; 
            }
            else { 
                if (m < MOIST_DESERT) bg_color_code = ANSI_BG_DESERT_SAND; 
                else if (m < MOIST_GRASS_SAVANNAH) bg_color_code = ANSI_BG_GRASS; 
                else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_JUNGLE_GREEN; 
                else bg_color_code = ANSI_BG_JUNGLE_GREEN; 
            }

            printf("%s  %s", bg_color_code, ANSI_RESET); // Use two spaces for more square-looking output
         }
         putchar('\n');
     }
     printf("--------------------\n");
}

int write_map_png(const MapData* map, const char* filename, double latitude_temp_factor) {
    if (!map || !map->elevation || !map->moisture) { return 1; }
    if (!filename) { return 1; }

    int width = map->width;
    int height = map->height;
    int channels = 3;
    unsigned char *pixel_data = malloc(width * height * channels * sizeof(unsigned char));
    if (!pixel_data) { return 1; }

    printf("Preparing pixel data for PNG file: %s\n", filename);

    // First pass - calculate normals if they haven't been calculated yet
    if (!map->normals || map->normals[0][0].z == 1.0) {
        // Temporary normals (not modifying const MapData)
        printf("Calculating temporary normals for rendering...\n");
        Vector3 **temp_normals = malloc(height * sizeof(Vector3*));
        if (!temp_normals) { 
            free(pixel_data);
            return 1; 
        }
        
        for (int y = 0; y < height; y++) {
            temp_normals[y] = malloc(width * sizeof(Vector3));
            if (!temp_normals[y]) {
                for (int i = 0; i < y; i++) free(temp_normals[i]);
                free(temp_normals);
                free(pixel_data);
                return 1;
            }
        }
        
        // Calculate normals
        const double height_scale = 5.0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int x_prev = (x > 0) ? (x - 1) : (width - 1);
                int x_next = (x < width - 1) ? (x + 1) : 0;
                int y_prev = (y > 0) ? (y - 1) : 0;
                int y_next = (y < height - 1) ? (y + 1) : (height - 1);
                
                double dx = (map->elevation[y][x_next] - map->elevation[y][x_prev]) * height_scale;
                double dy = (map->elevation[y_next][x] - map->elevation[y_prev][x]) * height_scale;
                
                double len = sqrt(dx*dx + dy*dy + 1.0);
                temp_normals[y][x].x = -dx / len;
                temp_normals[y][x].y = -dy / len;
                temp_normals[y][x].z = 1.0 / len;
            }
        }
        
        // Render with temporary normals
        for (int y = 0; y < height; y++) {
            double latitude_norm = (double)y / (height > 1 ? height - 1 : 1);
            double dist_from_equator = fabs(latitude_norm - 0.5) * 2.0;

            for (int x = 0; x < width; x++) {
                double e_orig = map->elevation[y][x];
                double m = map->moisture[y][x];
                double e_effective = clamp_io(e_orig + latitude_temp_factor * dist_from_equator, 0.0, 1.0);
                RGBColor base_color;
                
                // Determine base color with improved gradients
                if (e_effective < ELEV_DEEP_OCEAN) {
                    base_color = COLOR_DEEP_OCEAN;
                }
                else if (e_effective < ELEV_OCEAN) {
                    double t = (e_effective - ELEV_DEEP_OCEAN) / (ELEV_OCEAN - ELEV_DEEP_OCEAN);
                    base_color = blend_colors(COLOR_DEEP_OCEAN, COLOR_OCEAN, t);
                }
                else if (e_effective < ELEV_BEACH) {
                    double t = (e_effective - ELEV_OCEAN) / (ELEV_BEACH - ELEV_OCEAN);
                    base_color = blend_colors(COLOR_OCEAN, COLOR_SHALLOW_OCEAN, t);
                }
                else if (e_effective < ELEV_LOWLAND) {
                    // Land gradient near shore
                    double t = (e_effective - ELEV_BEACH) / (ELEV_LOWLAND - ELEV_BEACH);
                    
                    // Base color depends on biome
                    RGBColor land_color;
                    if (m < MOIST_DESERT) {
                        land_color = COLOR_DESERT_SAND;
                    } else if (m < MOIST_GRASS_SAVANNAH) {
                        land_color = COLOR_GRASS;
                    } else {
                        land_color = COLOR_JUNGLE_GREEN;
                    }
                    
                    base_color = blend_colors(COLOR_BEACH, land_color, t);
                }
                else if (e_effective > ELEV_MOUNTAIN) {
                    // Mountain peaks with snow
                    double t = (e_effective - ELEV_MOUNTAIN) / (1.0 - ELEV_MOUNTAIN);
                    base_color = blend_colors(COLOR_MOUNTAIN_LIGHT, COLOR_SNOW, t);
                }
                else if (e_effective > ELEV_BOREAL_MAX) {
                    // High elevation terrain
                    double t = (e_effective - ELEV_BOREAL_MAX) / (ELEV_MOUNTAIN - ELEV_BOREAL_MAX);
                    RGBColor high_color;
                    
                    if (m < MOIST_DESERT) {
                        high_color = COLOR_SCORCHED;
                    } else if (m < MOIST_GRASS_SAVANNAH) {
                        high_color = COLOR_BARE_ROCK;
                    } else if (m < MOIST_WOODLAND_SHRUB) {
                        high_color = COLOR_TUNDRA;
                    } else {
                        high_color = COLOR_SNOW;
                    }
                    
                    base_color = blend_colors(high_color, COLOR_MOUNTAIN_DARK, t);
                }
                else if (e_effective > ELEV_TEMPERATE_MAX) {
                    // Boreal zone with gradient
                    // Gradient factor (unused for now)
                    // double t = (e_effective - ELEV_TEMPERATE_MAX) / (ELEV_BOREAL_MAX - ELEV_TEMPERATE_MAX);
                    
                    RGBColor boreal_color;
                    if (m < MOIST_DESERT) {
                        boreal_color = COLOR_SAVANNAH;
                    } else if (m < MOIST_GRASS_SAVANNAH) {
                        boreal_color = COLOR_SHRUBLAND;
                    } else {
                        boreal_color = COLOR_TAIGA;
                    }
                    
                    base_color = boreal_color;
                }
                else if (e_effective > ELEV_TROPICAL_MAX) {
                    // Temperate zone with gradient
                    // Gradient factor (unused for now)
                    // double t = (e_effective - ELEV_TROPICAL_MAX) / (ELEV_TEMPERATE_MAX - ELEV_TROPICAL_MAX);
                    
                    RGBColor temperate_color;
                    if (m < MOIST_DESERT) {
                        temperate_color = COLOR_SAVANNAH;
                    } else if (m < MOIST_GRASS_SAVANNAH) {
                        temperate_color = COLOR_GRASS;
                    } else {
                        temperate_color = COLOR_FOREST_GREEN;
                    }
                    
                    base_color = temperate_color;
                }
                else {
                    // Tropical/lowland zone
                    RGBColor tropical_color;
                    if (m < MOIST_DESERT) {
                        tropical_color = COLOR_DESERT_SAND;
                    } else if (m < MOIST_GRASS_SAVANNAH) {
                        tropical_color = COLOR_GRASS;
                    } else {
                        tropical_color = COLOR_JUNGLE_GREEN;
                    }
                    
                    base_color = tropical_color;
                }
                
                // Apply lighting based on normals for 3D effect
                double height_factor = clamp_io((e_effective - ELEV_BEACH) / (1.0 - ELEV_BEACH), 0.0, 1.0);
                RGBColor final_color = apply_lighting(base_color, temp_normals[y][x], height_factor);
                
                // Add river overlay if needed
                if (map->is_river[y][x]) {
                    final_color = blend_colors(final_color, COLOR_LAKE_WATER, 0.7);
                }
                
                int index = (y * width + x) * channels;
                pixel_data[index + 0] = final_color.r;
                pixel_data[index + 1] = final_color.g;
                pixel_data[index + 2] = final_color.b;
            }
        }
        
        // Free temporary normals
        for (int y = 0; y < height; y++) {
            free(temp_normals[y]);
        }
        free(temp_normals);
    }
    else {
        // Use provided normals from the map
        for (int y = 0; y < height; y++) {
            double latitude_norm = (double)y / (height > 1 ? height - 1 : 1);
            double dist_from_equator = fabs(latitude_norm - 0.5) * 2.0;

            for (int x = 0; x < width; x++) {
                double e_orig = map->elevation[y][x];
                double m = map->moisture[y][x];
                double e_effective = clamp_io(e_orig + latitude_temp_factor * dist_from_equator, 0.0, 1.0);
                RGBColor base_color;
                
                // Determine base color with improved gradients
                if (e_effective < ELEV_DEEP_OCEAN) {
                    base_color = COLOR_DEEP_OCEAN;
                }
                else if (e_effective < ELEV_OCEAN) {
                    double t = (e_effective - ELEV_DEEP_OCEAN) / (ELEV_OCEAN - ELEV_DEEP_OCEAN);
                    base_color = blend_colors(COLOR_DEEP_OCEAN, COLOR_OCEAN, t);
                }
                else if (e_effective < ELEV_BEACH) {
                    double t = (e_effective - ELEV_OCEAN) / (ELEV_BEACH - ELEV_OCEAN);
                    base_color = blend_colors(COLOR_OCEAN, COLOR_SHALLOW_OCEAN, t);
                }
                else if (e_effective < ELEV_LOWLAND) {
                    // Land gradient near shore
                    double t = (e_effective - ELEV_BEACH) / (ELEV_LOWLAND - ELEV_BEACH);
                    
                    // Base color depends on biome
                    RGBColor land_color;
                    if (m < MOIST_DESERT) {
                        land_color = COLOR_DESERT_SAND;
                    } else if (m < MOIST_GRASS_SAVANNAH) {
                        land_color = COLOR_GRASS;
                    } else {
                        land_color = COLOR_JUNGLE_GREEN;
                    }
                    
                    base_color = blend_colors(COLOR_BEACH, land_color, t);
                }
                else if (e_effective > ELEV_MOUNTAIN) {
                    // Mountain peaks with snow
                    double t = (e_effective - ELEV_MOUNTAIN) / (1.0 - ELEV_MOUNTAIN);
                    base_color = blend_colors(COLOR_MOUNTAIN_LIGHT, COLOR_SNOW, t);
                }
                else if (e_effective > ELEV_BOREAL_MAX) {
                    // High elevation terrain
                    double t = (e_effective - ELEV_BOREAL_MAX) / (ELEV_MOUNTAIN - ELEV_BOREAL_MAX);
                    RGBColor high_color;
                    
                    if (m < MOIST_DESERT) {
                        high_color = COLOR_SCORCHED;
                    } else if (m < MOIST_GRASS_SAVANNAH) {
                        high_color = COLOR_BARE_ROCK;
                    } else if (m < MOIST_WOODLAND_SHRUB) {
                        high_color = COLOR_TUNDRA;
                    } else {
                        high_color = COLOR_SNOW;
                    }
                    
                    base_color = blend_colors(high_color, COLOR_MOUNTAIN_DARK, t);
                }
                else if (e_effective > ELEV_TEMPERATE_MAX) {
                    // Boreal zone with gradient
                    // Gradient factor (unused for now)
                    // double t = (e_effective - ELEV_TEMPERATE_MAX) / (ELEV_BOREAL_MAX - ELEV_TEMPERATE_MAX);
                    
                    RGBColor boreal_color;
                    if (m < MOIST_DESERT) {
                        boreal_color = COLOR_SAVANNAH;
                    } else if (m < MOIST_GRASS_SAVANNAH) {
                        boreal_color = COLOR_SHRUBLAND;
                    } else {
                        boreal_color = COLOR_TAIGA;
                    }
                    
                    base_color = boreal_color;
                }
                else if (e_effective > ELEV_TROPICAL_MAX) {
                    // Temperate zone with gradient
                    // Gradient factor (unused for now)
                    // double t = (e_effective - ELEV_TROPICAL_MAX) / (ELEV_TEMPERATE_MAX - ELEV_TROPICAL_MAX);
                    
                    RGBColor temperate_color;
                    if (m < MOIST_DESERT) {
                        temperate_color = COLOR_SAVANNAH;
                    } else if (m < MOIST_GRASS_SAVANNAH) {
                        temperate_color = COLOR_GRASS;
                    } else {
                        temperate_color = COLOR_FOREST_GREEN;
                    }
                    
                    base_color = temperate_color;
                }
                else {
                    // Tropical/lowland zone
                    RGBColor tropical_color;
                    if (m < MOIST_DESERT) {
                        tropical_color = COLOR_DESERT_SAND;
                    } else if (m < MOIST_GRASS_SAVANNAH) {
                        tropical_color = COLOR_GRASS;
                    } else {
                        tropical_color = COLOR_JUNGLE_GREEN;
                    }
                    
                    base_color = tropical_color;
                }
                
                // Apply lighting based on normals for 3D effect
                double height_factor = clamp_io((e_effective - ELEV_BEACH) / (1.0 - ELEV_BEACH), 0.0, 1.0);
                RGBColor final_color = apply_lighting(base_color, map->normals[y][x], height_factor);
                
                // Add river overlay if needed
                if (map->is_river[y][x]) {
                    final_color = blend_colors(final_color, COLOR_LAKE_WATER, 0.7);
                }
                
                int index = (y * width + x) * channels;
                pixel_data[index + 0] = final_color.r;
                pixel_data[index + 1] = final_color.g;
                pixel_data[index + 2] = final_color.b;
            }
        }
    }

    printf("Writing map to PNG file: %s\n", filename);
    int success = stbi_write_png(filename, width, height, channels, pixel_data, width * channels);
    free(pixel_data);

    if (success) { printf("PNG file write complete.\n"); return 0; }
    else { fprintf(stderr, "Error writing PNG file using stb_image_write.\n"); return 1; }
}
