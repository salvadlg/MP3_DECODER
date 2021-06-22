/***************************************************************************//**
 * \file   i2c_lpc40xx.h
 *
 * \brief  Funciones de manejo de los interfaces I2C del LPC40x.
 */

#ifndef I2C_LPC40XX_H
#define I2C_LPC40XX_H

#include <LPC407x_8x_177x_8x.h>
#include "tipos.h"

/*===== Constantes =============================================================
 */

#define I2C0            LPC_I2C0
#define I2C1            LPC_I2C1
#define I2C2            LPC_I2C2

/* Bits de los registros CONSET y CONCLR.
 */
#define I2C_CON_AA      (1u << 2)
#define I2C_CON_SI      (1u << 3)
#define I2C_CON_STO     (1u << 4)
#define I2C_CON_STA     (1u << 5)
#define I2C_CON_I2EN    (1u << 6)

/*===== Prototipos de funciones ================================================
 */

void i2c_inicializar(LPC_I2C_TypeDef *i2c_regs,
                     uint32_t frecuencia_scl,                       
                     LPC_GPIO_TypeDef *puerto_sda,
                     uint32_t mascara_pin_sda,
                     LPC_GPIO_TypeDef *puerto_scl,
                     uint32_t mascara_pin_scl);

void i2c_start(LPC_I2C_TypeDef *i2c_regs);

void i2c_stop(LPC_I2C_TypeDef *i2c_regs);
                        
bool_t i2c_transmitir_byte(LPC_I2C_TypeDef *i2c_regs,
                           uint8_t byte);
                                
uint8_t i2c_recibir_byte(LPC_I2C_TypeDef *i2c_regs,
                         bool_t ack);
                                       
#endif /* I2C_LPC40XX_H */
