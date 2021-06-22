/***************************************************************************//**
 * \file
 *
 * \brief
 */

#include <LPC407x_8x_177x_8x.h>
#include "i2s_lpc40xx.h"

/***************************************************************************//**
 * \brief       Inicializar el interfaz I2S del LPC40xx.
 */
void i2s_inicializar(void)
{
    /* Configurar los pines P0[7], P0[8], P0[9] y P0[16] con funciones
     * I2S (de transmisión), ya que son estos pines los que están conectados
     * al codec UDA1380 en la tarjeta Embedded Artist LPC4088.
     *
     * P0[7]  --> función I2S_TX_SCK.
     * P0[8]  --> función I2S_TX_WS.
     * P0[9]  --> función I2S_TX_SDA.
     * P1[16] --> función I2S_TX_MCLK.   
     */
    LPC_IOCON->P0_7 = 1 | (1u << 7);
    LPC_IOCON->P0_8 = 1 | (1u << 7);
    LPC_IOCON->P0_9 = 1 | (1u << 7);
    LPC_IOCON->P1_16 = 2 | (1u << 7);
    
    /* Activar el interfaz I2S en PCONP.
     */
    LPC_SC->PCONP |= 1 << 27;    
    
    /* En el registro DAI, activar el bit STOP para parar la parte de recepción
     * del I2S porque en esta aplicación el microcontrolador sólo envía al
     * UDA1380, no recibe de él.
     */
    LPC_I2S->DAI = 1 << 3;

    /* En el registro DAO, seleccionar:
     *
     * Ancho de palabras de datos: 16.
     * Modo estéreo.
     * De momento, activar el bit STOP para parar el interfaz hasta terminar la configuración.
     * Activar el bit RESET para resetear el interfaz.
     * Modo máster.
     * Semiperiodo WS en unidades de TX_SCK de 16 ciclos.
     * De momento, activar el MUTE mientras dura la configuración.
     */
    LPC_I2S->DAO = (1 << 15) | (15 << 6) | (1 << 4) | (1 << 3) | 1;    
    
    /* En el registro TXMODE, activar la salida TX_MCLK.
     */
    LPC_I2S->TXMODE = 1 << 3;        
    
    /* Cálculo de los divisores de reloj.
     *
     * La frecuencia de muestreo de audio que queremos es de
     *
     * f_S = 44100 Hz
     *
     * Como cada muestra de audio que vamos a enviar al UDA1380 está compuesta
     * por 16 bits y en cada periodo de muestreo hay que enviar una muestra del
     * canal izquierdo y una muestra del canal derecho, la tasa de bits es de
     *
     * 44100*16*2 = 1411200 bits/s
     *
     * Por tanto la frecuencia de la señal de reloj I2S_TX_SCK debe ser de
     * 1411200 Hz:
     *
     * f_I2S_TX_SCK = 1411200 Hz
     *
     * Por otra parte, la frecuencia de la señal I2S_TX_MCLK debe ser 256 veces
     * la frecuencia de muestreo.
     *
     * f_I2S_TX_MCLK = 256*f_S = 256*44100 = 11289600 Hz
     *
     * Las frecuencias de las señales f_I2S_TX_SCK y f_I2S_TX_MCLK están determinadas
     * por los valores de los registros TXRATE y TXBITRATE y por la frecuencia de
     * reloj de la CPU f_CCLK (el interfaz I2S usa CCLK y no PCLK).
     *
     * f_I2S_TX_MCLK = f_CCLK*TXRATE_X_DIVIDER/(2*TXRATE_Y_DIVIDER)
     *
     * donde TXRATE_X_DIVIDER y TXRATE_Y_DIVIDER son los campos de 8 bits
     * en el registro TXRATE.
     *
     * Resulta TXRATE_X_DIVIDER/TXRATE_Y_DIVIDER = 2*f_I2S_TX_MCLK/f_CCLK = 0.18816
     *
     * Seleccionamos TXRATE_X = 19
     *               TXRATE_Y = 101
     *
     * Por su parte
     *
     * f_I2S_TX_SCK = f_I2S_TX_MCLK/(TXBITRATE + 1)
     *
     * luego TXBITRATE = f_I2S_TX_MCLK/f_I2S_TX_SCK - 1 = 7
     */
    LPC_I2S->TXRATE = (19 << 8) | 101;
    LPC_I2S->TXBITRATE = 7;
    
    /* Configurar el registro IRQ para habilitar las interrupciones de transmisión
     * I2S y fijar un nivel de 4 para la FIFO de transmisión. Esto hará que el
     * interfaz genere una interrupción siempre que en la FIFO de transmisión haya
     * 4 palabras de 32 bits o menos (cada palabra de 32 bits representa dos
     * muestras, canal izquierdo y derecho, de 16 bits cada una). La capacidad total
     * de la FIFO de transmisión es de 8 palabras de 32 bits, así que se fija el
     * nivel de interrupción a la mitad de la capacidad de la FIFO.
     */
    LPC_I2S->IRQ = (4 << 16) | (1 << 1);
    
    /* Una vez que toda la parte de transmisión está ajustada, volvemos a escribir
     * en el registro DAO la misma configuración que antes, pero desactivando los
     * bits de RESET, STOP y MUTE.
     */
    LPC_I2S->DAO = (15 << 6) | 1;
    
    /* NOTA: En esta aplicación, la configuración que se realiza del UDA1380 a través
     *       del interfaz I2C hace que éste sintetize su reloj de sobremuestreo
     *       interno (SYSCLK = 256*f_S) a partir de la señal I2S_TX_WS por lo que,
     *       en principio, no sería necesario que el microcontrolador generase la
     *       señal I2S_TX_MCLK. Sin embargo, el interfaz I2C del UDA1380 no funciona
     *       hasta que no hay disponible una señal SYSCLK, así que antes de poder
     *       configurar el UDA1380 para sintetizar su propio SYSCLK es necesario
     *       aplicar un SYSCLK externo.
     */
}
