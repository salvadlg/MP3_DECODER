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
     * I2S (de transmisi�n), ya que son estos pines los que est�n conectados
     * al codec UDA1380 en la tarjeta Embedded Artist LPC4088.
     *
     * P0[7]  --> funci�n I2S_TX_SCK.
     * P0[8]  --> funci�n I2S_TX_WS.
     * P0[9]  --> funci�n I2S_TX_SDA.
     * P1[16] --> funci�n I2S_TX_MCLK.   
     */
    LPC_IOCON->P0_7 = 1 | (1u << 7);
    LPC_IOCON->P0_8 = 1 | (1u << 7);
    LPC_IOCON->P0_9 = 1 | (1u << 7);
    LPC_IOCON->P1_16 = 2 | (1u << 7);
    
    /* Activar el interfaz I2S en PCONP.
     */
    LPC_SC->PCONP |= 1 << 27;    
    
    /* En el registro DAI, activar el bit STOP para parar la parte de recepci�n
     * del I2S porque en esta aplicaci�n el microcontrolador s�lo env�a al
     * UDA1380, no recibe de �l.
     */
    LPC_I2S->DAI = 1 << 3;

    /* En el registro DAO, seleccionar:
     *
     * Ancho de palabras de datos: 16.
     * Modo est�reo.
     * De momento, activar el bit STOP para parar el interfaz hasta terminar la configuraci�n.
     * Activar el bit RESET para resetear el interfaz.
     * Modo m�ster.
     * Semiperiodo WS en unidades de TX_SCK de 16 ciclos.
     * De momento, activar el MUTE mientras dura la configuraci�n.
     */
    LPC_I2S->DAO = (1 << 15) | (15 << 6) | (1 << 4) | (1 << 3) | 1;    
    
    /* En el registro TXMODE, activar la salida TX_MCLK.
     */
    LPC_I2S->TXMODE = 1 << 3;        
    
    /* C�lculo de los divisores de reloj.
     *
     * La frecuencia de muestreo de audio que queremos es de
     *
     * f_S = 44100 Hz
     *
     * Como cada muestra de audio que vamos a enviar al UDA1380 est� compuesta
     * por 16 bits y en cada periodo de muestreo hay que enviar una muestra del
     * canal izquierdo y una muestra del canal derecho, la tasa de bits es de
     *
     * 44100*16*2 = 1411200 bits/s
     *
     * Por tanto la frecuencia de la se�al de reloj I2S_TX_SCK debe ser de
     * 1411200 Hz:
     *
     * f_I2S_TX_SCK = 1411200 Hz
     *
     * Por otra parte, la frecuencia de la se�al I2S_TX_MCLK debe ser 256 veces
     * la frecuencia de muestreo.
     *
     * f_I2S_TX_MCLK = 256*f_S = 256*44100 = 11289600 Hz
     *
     * Las frecuencias de las se�ales f_I2S_TX_SCK y f_I2S_TX_MCLK est�n determinadas
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
    
    /* Configurar el registro IRQ para habilitar las interrupciones de transmisi�n
     * I2S y fijar un nivel de 4 para la FIFO de transmisi�n. Esto har� que el
     * interfaz genere una interrupci�n siempre que en la FIFO de transmisi�n haya
     * 4 palabras de 32 bits o menos (cada palabra de 32 bits representa dos
     * muestras, canal izquierdo y derecho, de 16 bits cada una). La capacidad total
     * de la FIFO de transmisi�n es de 8 palabras de 32 bits, as� que se fija el
     * nivel de interrupci�n a la mitad de la capacidad de la FIFO.
     */
    LPC_I2S->IRQ = (4 << 16) | (1 << 1);
    
    /* Una vez que toda la parte de transmisi�n est� ajustada, volvemos a escribir
     * en el registro DAO la misma configuraci�n que antes, pero desactivando los
     * bits de RESET, STOP y MUTE.
     */
    LPC_I2S->DAO = (15 << 6) | 1;
    
    /* NOTA: En esta aplicaci�n, la configuraci�n que se realiza del UDA1380 a trav�s
     *       del interfaz I2C hace que �ste sintetize su reloj de sobremuestreo
     *       interno (SYSCLK = 256*f_S) a partir de la se�al I2S_TX_WS por lo que,
     *       en principio, no ser�a necesario que el microcontrolador generase la
     *       se�al I2S_TX_MCLK. Sin embargo, el interfaz I2C del UDA1380 no funciona
     *       hasta que no hay disponible una se�al SYSCLK, as� que antes de poder
     *       configurar el UDA1380 para sintetizar su propio SYSCLK es necesario
     *       aplicar un SYSCLK externo.
     */
}
