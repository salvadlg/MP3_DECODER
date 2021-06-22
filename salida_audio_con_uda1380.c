#include <LPC407x_8x_177x_8x.h>
#include <string.h>
#include "salida_audio.h"
#include "i2s_lpc40xx.h"
#include "tipos.h"
#include "error.h"
#include "uda1380.h"

#define  STREAM_DECODED_SIZE   (2*1152)

bool_t generando_audio = FALSE;

typedef struct {
  short int raw[STREAM_DECODED_SIZE];	/* 16 bit PCM output samples [ch][sample] */
  volatile  unsigned short wr_idx;
  volatile  unsigned short rd_idx;
}decoded_stream_t;

static decoded_stream_t     DecodedBuff;

#define IS_DFIFO_FULL()    ((DecodedBuff.wr_idx+1)%STREAM_DECODED_SIZE == DecodedBuff.rd_idx)
#define IS_DFIFO_EMPTY()   (DecodedBuff.wr_idx == DecodedBuff.rd_idx)

#define DFIFO_WRITE(S)     do {                                                                   \
                             DecodedBuff.raw[DecodedBuff.wr_idx] = S;                             \
                             DecodedBuff.wr_idx = ( (DecodedBuff.wr_idx+1)%STREAM_DECODED_SIZE ); \
                           }while(0)

#define DFIFO_READ(S)      do {                                                                   \
                             S = DecodedBuff.raw[DecodedBuff.rd_idx];                             \
                             DecodedBuff.rd_idx = ( (DecodedBuff.rd_idx+1)%STREAM_DECODED_SIZE ); \
                           }while(0)

#define FIFO_LEN()        ( DecodedBuff.wr_idx >= DecodedBuff.rd_idx ? \
                            DecodedBuff.wr_idx - DecodedBuff.rd_idx  : \
                            DecodedBuff.wr_idx + (STREAM_DECODED_SIZE - DecodedBuff.rd_idx) )

/***************************************************************************//**
 *
 */
void salaud_habilitar(void)
{
    NVIC_ClearPendingIRQ(I2S_IRQn);
    NVIC_EnableIRQ(I2S_IRQn);
    generando_audio = TRUE;
}

/***************************************************************************//**
 *
 */
void salaud_deshabilitar(void)
{
    NVIC_DisableIRQ(I2S_IRQn);
    generando_audio = FALSE;
}

/***************************************************************************//**
 *
 */
void salaud_esperar_fin_fragmento(void)
{
    salaud_deshabilitar();
    //while (!IS_DFIFO_EMPTY());
}

/***************************************************************************//**
 *
 */
void salaud_encolar_bloque_muestras(int32_t *ptr_muestras_izquierda,
                                    int32_t *ptr_muestras_derecha,
                                    uint16_t longitud,
                                    uint16_t numero_canales)
{
    uint16_t i;
    
    /* libmad genera muestras de 24 bits útiles en los 24 bits más significativos
     * de datos de 32 bits. Aquí se convierten a 16 bits antes de enviarlos al
     * buffer de muestras.
     */
          
    if (numero_canales == 1)
    {
        for (i = 0; i < longitud; i++)
        {
            while (IS_DFIFO_FULL());
            DFIFO_WRITE((int16_t)((*ptr_muestras_izquierda) >> 16));
            DFIFO_WRITE((int16_t)((*ptr_muestras_izquierda) >> 16));
            ptr_muestras_izquierda++;
        }
    }
    else if (numero_canales == 2)
    {
        for (i = 0; i < longitud; i++)
        {
            while (IS_DFIFO_FULL());
            DFIFO_WRITE((int16_t)((*ptr_muestras_izquierda) >> 16));
            ptr_muestras_izquierda++;
            DFIFO_WRITE((int16_t)((*ptr_muestras_derecha) >> 16));
            ptr_muestras_derecha++;
        }
    }
    else
    {
        ERROR("Numero de canales incorrecto.");
    }

    if (!generando_audio) salaud_habilitar();
}

/***************************************************************************//**
 *
 */
void salaud_ajustar_tasa_muestreo(uint32_t sample_rate)
{
    /* ESTA FUNCIÓN PUEDE DEJARSE VACÍA SI TODOS LOS FICHEROS MP3 QUE VAMOS
     * A REPRODUCIR TIENEN UNA FRECUENCIA DE MUESTREO DE 44100 Hz. DE MOMENTO
     * PODEMOS DEJARLA SIN COMPLETAR.
     */
}

/***************************************************************************//**
 *
 */
void salaud_inicializar(void)
{    
    DecodedBuff.wr_idx = DecodedBuff.rd_idx = 0;
    generando_audio = FALSE;

    i2s_inicializar();
    uda1380_inicializar();
    salaud_deshabilitar();
    
    __enable_irq();
}

/***************************************************************************//**
 * \brief   Función manejadora de interrupción de las interrupciones del
 *          interfaz I2S.
 *
 *          En esta aplicación, el interfaz I2S generará interrupción siempre
 *          que el nivel de ocupación de la FIFO de transmisión sea menor o
 *          igual a 4 (la mitad de su capacidad de 8 o menos).
 *
 *          La función manejadora de interrupción sacará del buffer de salida
 *          dos muestras mediante la macro DFIFO_READ, una para el canal
 *          izquierdo y otra para el canal derecho. Si la macro IS_DFIFO_EMPTY
 *          indica que el buffer de salida está vacío, dar valor 0 a ambas
 *          muestras.
 *
 *          NOTA: las macros DFIFO_READ y IS_DFIFO_EMPTY no se refieren a la
 *                FIFO de transmisión del I2S sino a la FIFO o buffer en la
 *                que el decodificador coloca las muestras de audio generadas.
 *                En esta función, recogemos las muestras de esta FIFO, las
 *                combinamos y las enviamos a la FIFO de transmisión del I2S.
 *
 *          Las dos muestras de 16 bits deben ser combinadas en un único dato de
 *          32 bits que es el que se enviará a la FIFO de transmisión del I2S.
 *          La muestra del canal izquierdo debe colocarse en los 16 bits más
 *          significativos del dato de 32 bits y la muestra del canal derecho en
 *          los 16 menos significativos.
 *
 *          Una vez que hemos combinado las dos muestras en un dato de 32 bits
 *          escribimos éste en la FIFO de transmisión del interfaz I2S y salimos.
 *
 *          PRECAUCIÓN: las muestras de audio son números de 16 con signo. Para
 *                      combinar las dos muestras izquieda/derecha en un único
 *                      dato de 32 bis será necesario realizar desplazamientos
 *                      y operaciones lógicas. Según escribamos la expresión
 *                      o expresiones para llevar a cabo estas operaciones puede
 *                      ser que estemos haciendo que el compilador promocione
 *                      datos con signo de 16 bits a datos con signo de 32 bits
 *                      lo que implica extender el bit de signo a los 16 bits
 *                      más significativos. Para evitar que esto cree problemas
 *                      a la hora de combinar las muestras (típicamente con OR),
 *                      moldear el tipo de las dos muestras a uint32_t.
 */
void I2S_IRQHandler(void)
{
    int16_t muestra_izquierda;
    int16_t muestra_derecha;
        
    if (!IS_DFIFO_EMPTY())
    {
        DFIFO_READ(muestra_izquierda);
    }
    else
    {
        muestra_izquierda = 0;  
    }
    
    if (!IS_DFIFO_EMPTY())
    {
        DFIFO_READ(muestra_derecha);
    }
    else
    {
        muestra_derecha = 0;  
    }
   
    LPC_I2S->TXFIFO = ((uint32_t)muestra_izquierda << 16) |
                      ((uint32_t)muestra_derecha & 0xFFFF);
}
