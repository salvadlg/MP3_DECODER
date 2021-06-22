/***************************************************************************//**
 * \file    dac_lpc40xx.h
 *
 * \brief   Funciones de manejo del DAC del LPC40xx.
 */
 
#ifndef DAC_LPC40XX_H
#define DAC_LPC40XX_H

#include "tipos.h"

void dac_inicializar(void);
void dac_convertir(uint16_t dato_a_convertir);

#endif /* DAC_LPC40XX_H */
