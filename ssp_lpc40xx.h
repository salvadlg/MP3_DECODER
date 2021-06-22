/***************************************************************************//**
 * \file    ssp_lpc40xx.h
 *
 * \brief   Funciones básicas para usar los interfaces SSP del LPC40xx en
 *          modo SPI.
 */

#ifndef SSP_LPC40XX_H
#define SSP_LPC40XX_H

#include <LPC407x_8x_177x_8x.h>
#include "tipos.h"

/*===== Constantes =============================================================
 */
 
/* Símbolos para referirse a los interfaces SSP.
 */

#define SSP0    LPC_SSP0
#define SSP1    LPC_SSP1
#define SSP2    LPC_SSP2

/* Símbolos para referise a valores para CPOL y CPHA.
 */
 
#define SSP_CPOL_0  0
#define SSP_CPOL_1  1

#define SSP_CPHA_0  0
#define SSP_CPHA_1  1

/* Símbolos para referirse al número de bits de datos manejado por la interfaz.
 */
 
#define SSP_DATOS_8_BITS    8
#define SSP_DATOS_9_BITS    9
#define SSP_DATOS_10_BITS   10
#define SSP_DATOS_11_BITS   11
#define SSP_DATOS_12_BITS   12
#define SSP_DATOS_13_BITS   13
#define SSP_DATOS_14_BITS   14
#define SSP_DATOS_15_BITS   15
#define SSP_DATOS_16_BITS   16

/*===== Prototipos de funciones ================================================
 */

void ssp_inicializar(LPC_SSP_TypeDef  *ssp_regs,
                     uint32_t          numero_bits_datos,
                     uint32_t          frecuencia_sck,
                     uint32_t          cpol,
                     uint32_t          cpha,
                     LPC_GPIO_TypeDef *puerto_sck,
                     uint32_t          mascara_pin_sck,
                     LPC_GPIO_TypeDef *puerto_miso,
                     uint32_t          mascara_pin_miso,
                     LPC_GPIO_TypeDef *puerto_mosi,
                     uint32_t          mascara_pin_mosi);

uint16_t ssp_transferir(LPC_SSP_TypeDef *ssp_regs,
                        uint16_t         dato_a_transmitir);

#endif /* SSP2_LPC40XX_H */
