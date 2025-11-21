#include "main.h"
#include <math.h>

#define SCREEN_CENTER_X 64
#define SCREEN_CENTER_Y 32
#define HORIZON_LENGTH 50

// Dibuja un horizonte artificial basado en pitch y roll
void draw_horizon(float pitch, float roll) {
    // SIN limitar ángulos - rango completo

    // Normalizar pitch para display vertical (wrap around)
    // Escala: 90° = 32 pixels (altura media de pantalla)
    float pitch_normalized = pitch;
    while (pitch_normalized > 180.0f) pitch_normalized -= 360.0f;
    while (pitch_normalized < -180.0f) pitch_normalized += 360.0f;

    // Calcular desplazamiento vertical (0.35 pixels por grado)
    int y_offset = (int)(pitch_normalized * 0.35f);

    // Normalizar roll
    float roll_normalized = roll;
    while (roll_normalized > 180.0f) roll_normalized -= 360.0f;
    while (roll_normalized < -180.0f) roll_normalized += 360.0f;

    // Calcular inclinación de la línea por roll
    float roll_rad = roll_normalized * 3.14159f / 180.0f;

    // Puntos de la línea del horizonte
    int x1 = SCREEN_CENTER_X - HORIZON_LENGTH/2;
    int x2 = SCREEN_CENTER_X + HORIZON_LENGTH/2;
    int y1 = SCREEN_CENTER_Y - y_offset - (int)((HORIZON_LENGTH/2) * tan(roll_rad));
    int y2 = SCREEN_CENTER_Y - y_offset + (int)((HORIZON_LENGTH/2) * tan(roll_rad));

    // Limitar a los bordes de la pantalla para evitar dibujar fuera
    if (y1 < 0) y1 = 0;
    if (y1 >= SSD1306_HEIGHT) y1 = SSD1306_HEIGHT - 1;
    if (y2 < 0) y2 = 0;
    if (y2 >= SSD1306_HEIGHT) y2 = SSD1306_HEIGHT - 1;

    // Dibujar línea del horizonte (blanca gruesa)
    ssd1306_Line(x1, y1, x2, y2, White);
    if (y1 > 0 && y2 > 0) {
        ssd1306_Line(x1, y1-1, x2, y2-1, White);
    }
    if (y1 < SSD1306_HEIGHT-1 && y2 < SSD1306_HEIGHT-1) {
        ssd1306_Line(x1, y1+1, x2, y2+1, White);
    }

    // Dibujar indicador central (referencia fija)
    ssd1306_Line(SCREEN_CENTER_X - 10, SCREEN_CENTER_Y, SCREEN_CENTER_X - 3, SCREEN_CENTER_Y, White);
    ssd1306_Line(SCREEN_CENTER_X + 3, SCREEN_CENTER_Y, SCREEN_CENTER_X + 10, SCREEN_CENTER_Y, White);
    ssd1306_DrawPixel(SCREEN_CENTER_X, SCREEN_CENTER_Y, White);
}

void draw_angle_bars(float pitch, float roll) {
    char buffer[16];

    // Normalizar ángulos al rango -180 a 180
    float pitch_normalized = pitch;
    while (pitch_normalized > 180.0f) pitch_normalized -= 360.0f;
    while (pitch_normalized < -180.0f) pitch_normalized += 360.0f;

    float roll_normalized = roll;
    while (roll_normalized > 180.0f) roll_normalized -= 360.0f;
    while (roll_normalized < -180.0f) roll_normalized += 360.0f;

    // ==================== BARRA DE PITCH (Y) ====================
    // Mapear -180 a 180 grados a la altura de la barra (28 pixels)
    // 180° = 28 pixels de altura
    int pitch_bar_height = (int)((pitch_normalized / 180.0f) * 28.0f);
    uint8_t pitch_y_start = 32 - abs(pitch_bar_height)/2;

    if (pitch_bar_height > 0) {
        ssd1306_FillRectangle(120, pitch_y_start, 125, pitch_y_start + pitch_bar_height, White);
    } else if (pitch_bar_height < 0) {
        ssd1306_FillRectangle(120, pitch_y_start + pitch_bar_height, 125, pitch_y_start, White);
    }
    ssd1306_DrawRectangle(119, 18, 126, 46, White);

    // Línea central de referencia (0°)
    ssd1306_Line(119, 32, 126, 32, White);

    // ==================== BARRA DE ROLL (X) ====================
    int roll_bar_height = (int)((roll_normalized / 180.0f) * 28.0f);
    uint8_t roll_y_start = 32 - abs(roll_bar_height)/2;

    if (roll_bar_height > 0) {
        ssd1306_FillRectangle(2, roll_y_start, 7, roll_y_start + roll_bar_height, White);
    } else if (roll_bar_height < 0) {
        ssd1306_FillRectangle(2, roll_y_start + roll_bar_height, 7, roll_y_start, White);
    }
    ssd1306_DrawRectangle(1, 18, 8, 46, White);

    // Línea central de referencia (0°)
    ssd1306_Line(1, 32, 8, 32, White);

    // ==================== VALORES NUMÉRICOS ====================
    // Valor X arriba - alineado a la izquierda
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("X", Font_6x8, White);
    sprintf(buffer, "%.0f", roll_normalized);
    ssd1306_SetCursor(0, 8);
    ssd1306_WriteString(buffer, Font_6x8, White);

    // Valor Y arriba - alineado a la derecha
    ssd1306_SetCursor(122, 0);
    ssd1306_WriteString("Y", Font_6x8, White);
    sprintf(buffer, "%.0f", pitch_normalized);

    // Ajustar posición según el ancho del número
    uint8_t x_pos = 122;
    if (pitch_normalized < 0) x_pos -= 6;  // Espacio para el signo negativo
    if (fabs(pitch_normalized) >= 100.0f) x_pos -= 12;  // 3 dígitos
    else if (fabs(pitch_normalized) >= 10.0f) x_pos -= 6;  // 2 dígitos

    ssd1306_SetCursor(x_pos, 8);
    ssd1306_WriteString(buffer, Font_6x8, White);
}

void draw_bubble_level(float roll, float pitch) {
    char buffer[32];

    // Centro del nivel
    #define BUBBLE_CENTER_X 64
    #define BUBBLE_CENTER_Y 32
    #define BUBBLE_RADIUS 20
    #define DOT_RADIUS 3

    // Normalizar ángulos
    float roll_normalized = roll;
    while (roll_normalized > 180.0f) roll_normalized -= 360.0f;
    while (roll_normalized < -180.0f) roll_normalized += 360.0f;

    float pitch_normalized = pitch;
    while (pitch_normalized > 180.0f) pitch_normalized -= 360.0f;
    while (pitch_normalized < -180.0f) pitch_normalized += 360.0f;

    // Dibujar círculo exterior
    ssd1306_DrawCircle(BUBBLE_CENTER_X, BUBBLE_CENTER_Y, BUBBLE_RADIUS, White);

    // Dibujar círculos concéntricos (marcadores de 45° y 90°)
    ssd1306_DrawCircle(BUBBLE_CENTER_X, BUBBLE_CENTER_Y, BUBBLE_RADIUS/2, White);

    // Dibujar cruz de referencia
    ssd1306_Line(BUBBLE_CENTER_X - BUBBLE_RADIUS, BUBBLE_CENTER_Y,
                 BUBBLE_CENTER_X + BUBBLE_RADIUS, BUBBLE_CENTER_Y, White);
    ssd1306_Line(BUBBLE_CENTER_X, BUBBLE_CENTER_Y - BUBBLE_RADIUS,
                 BUBBLE_CENTER_X, BUBBLE_CENTER_Y + BUBBLE_RADIUS, White);

    // Calcular posición del punto (burbuja)
    // Mapear ±180° al radio del círculo
    float scale = (float)BUBBLE_RADIUS / 180.0f;
    int bubble_x = BUBBLE_CENTER_X + (int)(roll_normalized * scale);
    int bubble_y = BUBBLE_CENTER_Y + (int)(pitch_normalized * scale);

    // Limitar la burbuja al círculo
    int dx = bubble_x - BUBBLE_CENTER_X;
    int dy = bubble_y - BUBBLE_CENTER_Y;
    float distance = sqrt(dx*dx + dy*dy);

    if (distance > BUBBLE_RADIUS - DOT_RADIUS) {
        float angle = atan2(dy, dx);
        bubble_x = BUBBLE_CENTER_X + (int)((BUBBLE_RADIUS - DOT_RADIUS) * cos(angle));
        bubble_y = BUBBLE_CENTER_Y + (int)((BUBBLE_RADIUS - DOT_RADIUS) * sin(angle));
    }

    // Dibujar la burbuja
    ssd1306_FillCircle(bubble_x, bubble_y, DOT_RADIUS, White);

    // Mostrar valores numéricos debajo con etiquetas X/Y
    ssd1306_SetCursor(2, 54);
    sprintf(buffer, "X:%.0f Y:%.0f", roll_normalized, pitch_normalized);
    ssd1306_WriteString(buffer, Font_6x8, White);
}


void draw_bar_graph(float pitch, float roll) {
    char buffer[16];

    // Normalizar ángulos
    float pitch_normalized = pitch;
    while (pitch_normalized > 180.0f) pitch_normalized -= 360.0f;
    while (pitch_normalized < -180.0f) pitch_normalized += 360.0f;

    float roll_normalized = roll;
    while (roll_normalized > 180.0f) roll_normalized -= 360.0f;
    while (roll_normalized < -180.0f) roll_normalized += 360.0f;

    // Título
    ssd1306_SetCursor(30, 0);
    ssd1306_WriteString("MPU6050", Font_7x10, White);

    // ==================== EJE Y (Pitch) ====================
    ssd1306_SetCursor(0, 15);
    ssd1306_WriteString("Y:", Font_7x10, White);
    sprintf(buffer, "%.0f", pitch_normalized);
    ssd1306_SetCursor(15, 15);
    ssd1306_WriteString(buffer, Font_7x10, White);

    // Barra pitch: mapear -180 a 180 sobre 60 pixels (±30 desde el centro)
    int pitch_width = (int)((pitch_normalized / 180.0f) * 60.0f);
    if (pitch_width > 60) pitch_width = 60;
    if (pitch_width < -60) pitch_width = -60;

    if (pitch_width >= 0) {
        ssd1306_FillRectangle(64, 17, 64 + pitch_width, 23, White);
    } else {
        ssd1306_FillRectangle(64 + pitch_width, 17, 64, 23, White);
    }

    // Marco y marcadores
    ssd1306_DrawRectangle(4, 17, 124, 23, White);
    ssd1306_Line(64, 16, 64, 24, White);  // Centro (0°)
    ssd1306_Line(34, 18, 34, 22, White);  // -90°
    ssd1306_Line(94, 18, 94, 22, White);  // +90°

    // ==================== EJE X (Roll) ====================
    ssd1306_SetCursor(0, 35);
    ssd1306_WriteString("X:", Font_7x10, White);
    sprintf(buffer, "%.0f", roll_normalized);
    ssd1306_SetCursor(15, 35);
    ssd1306_WriteString(buffer, Font_7x10, White);

    // Barra roll
    int roll_width = (int)((roll_normalized / 180.0f) * 60.0f);
    if (roll_width > 60) roll_width = 60;
    if (roll_width < -60) roll_width = -60;

    if (roll_width >= 0) {
        ssd1306_FillRectangle(64, 37, 64 + roll_width, 43, White);
    } else {
        ssd1306_FillRectangle(64 + roll_width, 37, 64, 43, White);
    }

    // Marco y marcadores
    ssd1306_DrawRectangle(4, 37, 124, 43, White);
    ssd1306_Line(64, 36, 64, 44, White);  // Centro (0°)
    ssd1306_Line(34, 38, 34, 42, White);  // -90°
    ssd1306_Line(94, 38, 94, 42, White);  // +90°

    // Leyenda pequeña
    ssd1306_SetCursor(3, 50);
    ssd1306_WriteString("-180", Font_6x8, White);
    ssd1306_SetCursor(56, 50);
    ssd1306_WriteString("0", Font_6x8, White);
    ssd1306_SetCursor(98, 50);
    ssd1306_WriteString("+180", Font_6x8, White);
}


// Historial de valores (scrolling graph)
#define HISTORY_SIZE 128
static float pitch_history[HISTORY_SIZE] = {0};
static float roll_history[HISTORY_SIZE] = {0};
static uint8_t history_index = 0;

void draw_scrolling_graph(float pitch, float roll) {
    // Normalizar ángulos
    float pitch_normalized = pitch;
    while (pitch_normalized > 180.0f) pitch_normalized -= 360.0f;
    while (pitch_normalized < -180.0f) pitch_normalized += 360.0f;

    float roll_normalized = roll;
    while (roll_normalized > 180.0f) roll_normalized -= 360.0f;
    while (roll_normalized < -180.0f) roll_normalized += 360.0f;

    // Guardar nuevo valor
    pitch_history[history_index] = pitch_normalized;
    roll_history[history_index] = roll_normalized;
    history_index = (history_index + 1) % HISTORY_SIZE;

    // Dibujar título y valores actuales
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("Y:", Font_6x8, White);
    char buf[16];
    sprintf(buf, "%.0f", pitch_normalized);
    ssd1306_SetCursor(12, 0);
    ssd1306_WriteString(buf, Font_6x8, White);

    ssd1306_SetCursor(90, 0);
    ssd1306_WriteString("X:", Font_6x8, White);
    sprintf(buf, "%.0f", roll_normalized);
    ssd1306_SetCursor(102, 0);
    ssd1306_WriteString(buf, Font_6x8, White);

    // Líneas de referencia
    ssd1306_Line(0, 16, 127, 16, White);  // +90°
    ssd1306_Line(0, 32, 127, 32, White);  // 0°
    ssd1306_Line(0, 48, 127, 48, White);  // -90°

    // Etiquetas de escala
    ssd1306_SetCursor(0, 10);
    ssd1306_WriteString("180", Font_6x8, White);
    ssd1306_SetCursor(0, 29);
    ssd1306_WriteString("0", Font_6x8, White);
    ssd1306_SetCursor(0, 50);
    ssd1306_WriteString("-180", Font_6x8, White);

    // Dibujar historial
    for (int i = 0; i < HISTORY_SIZE - 1; i++) {
        int idx = (history_index + i) % HISTORY_SIZE;
        int next_idx = (history_index + i + 1) % HISTORY_SIZE;

        // Mapear -180 a 180 a pixeles 10 a 54 (44 pixels de altura)
        int y1_pitch = 32 - (int)(pitch_history[idx] * 22.0f / 180.0f);
        int y2_pitch = 32 - (int)(pitch_history[next_idx] * 22.0f / 180.0f);

        // Limitar a pantalla
        if (y1_pitch < 10) y1_pitch = 10;
        if (y1_pitch > 54) y1_pitch = 54;
        if (y2_pitch < 10) y2_pitch = 10;
        if (y2_pitch > 54) y2_pitch = 54;

        // Dibujar pitch (línea continua)
        if (i >= 25) {  // Empezar después de la zona de texto
            ssd1306_Line(i, y1_pitch, i+1, y2_pitch, White);
        }
    }
}

/*
 * mpu_display.c
 *
 *  Created on: Nov 17, 2025
 *      Author: Samuel
 */


