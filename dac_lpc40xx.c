/***************************************************************************//**
 * \file    dac_lpc40xx.c
 *
 * \brief   Funciones de manejo del DAC del LPC40xx.
 */

#include <LPC407x_8x_177x_8x.h>
#include "dac_lpc40xx.h"
#include "error.h"

/***************************************************************************//**
 * \brief
 */
void dac_inicializar(void)
{
    LPC_IOCON->P0_26 = (1u << 16) | 2;  /* FUNC = 2 => función DAC_OUT
                                         * ADMODE = 0 => selecciona modo analógico
                                         * DACEN = 1 => bit 16 a 1 => activa el DAC
                                         */
}

/***************************************************************************//**
 * \brief
 */
void dac_convertir(uint16_t dato_a_convertir)
{
    ASSERT(dato_a_convertir < 1024, "dato_a_convertir debe estar entre 0 y 1023");
    
    LPC_DAC->CR = dato_a_convertir << 6;
}
