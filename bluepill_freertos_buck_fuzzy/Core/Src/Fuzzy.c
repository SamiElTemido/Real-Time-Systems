#include "main.h" // Asegúrate de incluir main.h para tipos de datos o math.h
#include "Fuzzy.h"
const float OUTPUT_WEIGHTS[5] = {-1.0f, -0.5f, 0.0f, 0.5f, 1.0f};
// --- FUNCIONES DE MEMBRESÍA TRIANGULARES ---
// Calcula la pertenencia a un conjunto triangular definido por (centro, ancho)
// El rango de entrada x siempre se asume normalizado [-1, 1]
float trimf(float x, float center) {
    float width = 0.5f; // Ancho base para 5 conjuntos equidistantes

    // Lógica para bordes (Saturación en los extremos)
    if (center == -1.0f && x <= -1.0f) return 1.0f; // Saturación Negativa
    if (center == 1.0f  && x >= 1.0f)  return 1.0f; // Saturación Positiva

    float val = 1.0f - (fabs(x - center) / width);
    return (val > 0.0f) ? val : 0.0f;
}

// --- MOTOR DE INFERENCIA DIFUSA ---
float Fuzzy_Compute(float error, float d_error) {

    // 1. NORMALIZACIÓN (Escalar entradas a -1...1)
    float e_norm = error / FUZZY_ERR_MAX;
    float de_norm = d_error / FUZZY_DERR_MAX;

    // Saturación de seguridad
    if (e_norm > 1.0f) e_norm = 1.0f; else if (e_norm < -1.0f) e_norm = -1.0f;
    if (de_norm > 1.0f) de_norm = 1.0f; else if (de_norm < -1.0f) de_norm = -1.0f;

    // 2. FUZZIFICACIÓN (Calcular mu para cada conjunto)
    // Índices: 0=NB, 1=NS, 2=ZE, 3=PS, 4=PB
    float mu_e[5], mu_de[5];
    float centers[5] = {-1.0f, -0.5f, 0.0f, 0.5f, 1.0f};

    for(int i=0; i<5; i++) {
        mu_e[i]  = trimf(e_norm, centers[i]);
        mu_de[i] = trimf(de_norm, centers[i]);
    }

    // 3. REGLAS E INFERENCIA (Matriz 5x5)
    // Base de reglas diagonal típica para control de error:
    // Si E es negativo y dE es negativo -> Salida Negativa Grande (Necesitamos bajar mucho)
    // Tabla de indices de salida:
    //      NB  NS  ZE  PS  PB  (dE)
    // NB [ NB  NB  NB  NS  ZE ]
    // NS [ NB  NB  NS  ZE  PS ]
    // ZE [ NB  NS  ZE  PS  PB ]
    // PS [ NS  ZE  PS  PB  PB ]
    // PB [ ZE  PS  PB  PB  PB ] (E)

    const int RULE_TABLE[5][5] = {
        {0, 0, 0, 1, 2}, // E = NB
        {0, 0, 1, 2, 3}, // E = NS
        {0, 1, 2, 3, 4}, // E = ZE (Diagonal principal ideal)
        {1, 2, 3, 4, 4}, // E = PS
        {2, 3, 4, 4, 4}  // E = PB
    };

    float num = 0.0f;
    float den = 0.0f;

    // Iterar por todas las reglas activas
    for (int r = 0; r < 5; r++) {      // Fila (Error)
        if (mu_e[r] == 0) continue;    // Optimización: si mu es 0, saltar fila

        for (int c = 0; c < 5; c++) {  // Columna (dError)
            if (mu_de[c] == 0) continue;

            // Operador AND (Minimo)
            float firing_strength = (mu_e[r] < mu_de[c]) ? mu_e[r] : mu_de[c];

            // Obtener el índice de salida de la tabla
            int out_idx = RULE_TABLE[r][c];

            // Acumular para defuzzificación (Centroide / Altura ponderada)
            num += firing_strength * OUTPUT_WEIGHTS[out_idx];
            den += firing_strength;
        }
    }

    // 4. DEFUZZIFICACIÓN Y ESCALADO
    float output_norm = 0.0f;
    if (den != 0.0f) {
        output_norm = num / den;
    }

    // Desnormalizar usando tu parámetro de salida máxima (500)
    return output_norm * FUZZY_OUT_MAX;
}
