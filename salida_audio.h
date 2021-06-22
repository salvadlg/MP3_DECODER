/***************************************************************************//**
 * \file    salida_audio.h
 *
 * \brief   Funciones de salida de audio.
 */

#ifndef SALIDA_AUDIO_H
#define SALIDA_AUDIO_H

#include "tipos.h"

void salaud_habilitar(void);
void salaud_deshabilitar(void);
void salaud_esperar_fin_fragmento(void);
void salaud_encolar_bloque_muestras(int32_t *ptr_muestras_izquierda,
                                    int32_t *ptr_muestras_derecha,
                                    uint16_t longitud,
                                    uint16_t numero_canales);
void salaud_ajustar_tasa_muestreo(uint32_t sample_rate);
void salaud_inicializar(void);

#endif  /* SALIDA_AUDIO_H */
