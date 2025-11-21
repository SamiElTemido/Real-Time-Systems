#ifndef MPU_DISPLAY_H
#define MPU_DISPLAY_H

#include <stdint.h>

// Funciones de visualización
void draw_horizon(float pitch, float roll);
void draw_angle_bars(float pitch, float roll);
void draw_bubble_level(float roll, float pitch);
void draw_bar_graph(float pitch, float roll);
void draw_scrolling_graph(float pitch, float roll);

#endif // MPU_DISPLAY_H
