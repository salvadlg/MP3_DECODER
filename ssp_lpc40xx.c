/***************************************************************************//**
 * \file    ssp_lpc40xx.c
 *
 * \brief   Funciones básicas para usar los interfaces SSP del LPC40xx en
 *          modo SPI.
 */

#include <LPC407x_8x_177x_8x.h>
#include "ssp_lpc40xx.h"
#include "error.h"
#include "iocon_lpc40xx.h"

/***************************************************************************//**
 * \brief       Inicializar un interfaz SSP del LP40xx en modo maestro SPI.
 * 
 * \param[in]   ssp_regs          puntero a regs. de interfaz SSP a inicializar.
 * \param[in]   numero_bits_datos bits que tendrán los datos que se transferirán.
 * \param[in]   frecuencia_sck    frecuencia de reloj a usar.
 * \param[in]   cpol              polaridad de reloj (0 o 1).
 * \param[in]   cpha              fase de reloj (0 o 1).
 * \param[in]   puerto_sck        puntero a regs. GPIO de puerto para señal SCK.
 * \param[in]   mascara_pin_sck   mascara de seleccion de pin para señal SCK.
 * \param[in]   puerto_miso       puntero a regs. GPIO de puerto para señal MISO.
 * \param[in]   mascara_pin_miso  mascara de seleccion de pin para señal MISO.
 * \param[in]   puerto_mosi       puntero a regs. GPIO de puerto para señal MOSI.
 * \param[in]   mascara_pin_mosi  mascara de seleccion de pin para señal MOSI.
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
                     uint32_t          mascara_pin_mosi)
{
    uint32_t valor_scr;
    
    /* Comprobar los parámetros.
     */
    ASSERT(numero_bits_datos >= 4 && numero_bits_datos <= 16,
           "El número de bits de los datos debe estar entre 4 y 16.");
    
    ASSERT(cpol == 0 || cpol == 1,
           "cpol debe ser 0 o 1");
    
    ASSERT(cpha == 0 || cpha == 1,
           "cpha debe ser 0 o 1");    
    
    ASSERT(PeripheralClock / 2 >= frecuencia_sck,
           "La frecuencia sck requerida es demasiado elevada.");    
    
    valor_scr = PeripheralClock / (2*frecuencia_sck);    
    ASSERT(valor_scr <= 256,
           "La frecuencia sck requerida es demasiado baja.");

    /* Activar la "alimentación" del interfaz seleccionado.
     */
    if (ssp_regs == SSP0)
    {
        LPC_SC->PCONP |= 1u << 21;
    }
    else if (ssp_regs == SSP1)
    {
        LPC_SC->PCONP |= 1u << 10;
    }
    else if (ssp_regs == SSP2)
    {
        LPC_SC->PCONP |= 1u << 20;
    }
    
    /* Asegurarse de que el interfaz está deshabilitado mientras se configura.
     */        
    ssp_regs->CR1 = 0; 
        
    /* El registro CPSR se programará con el valor fijo 2 para simplificar
     * el ajuste de la frecuencia de SCK.
     */    
    ssp_regs->CPSR = 2; 

    /* En el registro CR0, programar el número de bits de datos, modo SPI,
     * modos CPOL y CPHA y valor del divisor SCR.
     */
    ssp_regs->CR0 = (numero_bits_datos - 1) |
                    (cpol << 6) |
                    (cpha << 7) |
                    (valor_scr << 8);
    
    /* En el registro CR1, seleccionar el modo maestro y habilitar
     * el interfaz.
     */
    ssp_regs->CR1 = 1u << 1;

    /* Seleccionar las funciones SSPn_SCK, SSPn_MISO y SSPn_MOSI en los pines
     * indicados.
     */
    if (ssp_regs == SSP0)
    {
        iocon_configurar_pin(puerto_sck, mascara_pin_sck, SSP0_SCK,
            IOCON_NO_PULL_UP_NO_PULL_DOWN);
        iocon_configurar_pin(puerto_miso, mascara_pin_miso, SSP0_MISO,
            IOCON_NO_PULL_UP_NO_PULL_DOWN);
        iocon_configurar_pin(puerto_mosi, mascara_pin_mosi, SSP0_MOSI,
            IOCON_NO_PULL_UP_NO_PULL_DOWN);        
    }
    else if (ssp_regs == SSP1)
    {
        iocon_configurar_pin(puerto_sck, mascara_pin_sck, SSP1_SCK,
            IOCON_NO_PULL_UP_NO_PULL_DOWN);
        iocon_configurar_pin(puerto_miso, mascara_pin_miso, SSP1_MISO,
            IOCON_NO_PULL_UP_NO_PULL_DOWN);
        iocon_configurar_pin(puerto_mosi, mascara_pin_mosi, SSP1_MOSI,
            IOCON_NO_PULL_UP_NO_PULL_DOWN);        
    }
    else if (ssp_regs == SSP2)
    {
        iocon_configurar_pin(puerto_sck, mascara_pin_sck, SSP2_SCK,
            IOCON_NO_PULL_UP_NO_PULL_DOWN);
        iocon_configurar_pin(puerto_miso, mascara_pin_miso, SSP2_MISO,
            IOCON_NO_PULL_UP_NO_PULL_DOWN);
        iocon_configurar_pin(puerto_mosi, mascara_pin_mosi, SSP2_MOSI,
            IOCON_NO_PULL_UP_NO_PULL_DOWN);        
    }
}

/***************************************************************************//**
 * \brief   Realizar una transferencia a través de un interfaz SSP del
 *          LPC40xx en modo SPI. La función no retorna hasta que
 *          termine la transferencia.
 *
 * \param[in]   ssp_regs            puntero a regs. de interfaz SSP.
 * \param[in]   dato_a_transmitir   dato a enviar al esclavo. Sólo se
 *                                  enviarán los bits de 0 a n-1, siendo n
 *                                  el número de bits de datos para el que
 *                                  ha sido configurada la interfaz.
 *
 * \return      dato recibido desde el esclavo. Sólo son significativos los
 *              bits de  0 a n-1, siendo n el número de bits de datos para el
 *              que ha sido configurada la interfaz.
 */
uint16_t ssp_transferir(LPC_SSP_TypeDef  *ssp_regs, uint16_t dato_a_transmitir)
{
    /* Esperar hasta que el bit TNF (Transmit FIFO Not Full) del registro de
     * estado del interfaz esté a 1, indicando así que hay espacio en la
     * FIFO de transmisión.
     */
    while ((ssp_regs->SR & (1u << 1)) == 0) {}
    
    /* Escribir el dato a transmitir en el registro de datos. Esto 
     * hace que ingrese en la FIFO de transmisión. Si en este momento la FIFO
     * de transmisión está vacía, la transmisión del dato comienza
     * inmediatamente.
     */
    ssp_regs->DR = dato_a_transmitir;

    /* Esperar hasta que el bit RNE (Receive FIFO Not Empty) del registro de
     * estado del interfaz esté a 1, indicando así que hay al menos un dato
     * en la FIFO de recepción.
     */        
    while ((ssp_regs->SR & (1u << 2)) == 0) {}
    
    /* Leer un dato de la FIFO de recepción y retornarlo.
     */
    return ssp_regs->DR; 
}
