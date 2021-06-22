#include <LPC407x_8x_177x_8x.h>
#include <string.h>
#include "salida_audio.h"
#include "dac_lpc40xx.h"
#include "tipos.h"
#include "error.h"

#define  STREAM_DECODED_SIZE     (2*1152)
#define  ENABLED                  1
#define  DISABLED                 0

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
    LPC_TIM0->TCR = 1;
    generando_audio = TRUE;
}

/***************************************************************************//**
 *
 */
void salaud_deshabilitar(void)
{
    LPC_TIM0->TCR = 0;
    generando_audio = FALSE;
}

/***************************************************************************//**
 *
 */
void salaud_esperar_fin_fragmento(void)
{
    while (!IS_DFIFO_EMPTY());
    salaud_deshabilitar();
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
    
    if (numero_canales == 1)
    {
        for (i = 0; i < longitud; i++)
        {
            while (IS_DFIFO_FULL());
            DFIFO_WRITE(*ptr_muestras_izquierda);
            ptr_muestras_izquierda++;
        }
    }
    else if (numero_canales == 2)
    {
        for (i = 0; i < longitud; i++)
        {
            while (IS_DFIFO_FULL());
            DFIFO_WRITE((int16_t)(((int32_t)*ptr_muestras_izquierda + (int32_t)*ptr_muestras_derecha)/2));
            ptr_muestras_izquierda++;
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
    LPC_TIM0->TCR = 0;
    LPC_TIM0->PC = 0;
    LPC_TIM0->TC = 0;
    LPC_TIM0->PR = 0;
    LPC_TIM0->MCR = 3;
    LPC_TIM0->MR0 = ((uint32_t)PeripheralClock/sample_rate) - 1;
    LPC_TIM0->TCR = 1;
}

/***************************************************************************//**
 *
 */
void salaud_inicializar(void)
{    
    DecodedBuff.wr_idx = DecodedBuff.rd_idx = 0;
    generando_audio = FALSE;

    LPC_TIM0->TCR = 0;    
    LPC_TIM0->IR = 1;
    LPC_TIM0->PC = 0;
    LPC_TIM0->TC = 0;
    LPC_TIM0->PR = 0;
    LPC_TIM0->MCR = 3;
    
    NVIC_ClearPendingIRQ(TIMER0_IRQn);
    NVIC_EnableIRQ(TIMER0_IRQn);
    
    dac_inicializar();
    
    __enable_irq();
}

/***************************************************************************//**
 *
 */
void TIMER0_IRQHandler(void)
{
    int16_t muestra;

    LPC_TIM0->IR = 1;
        
    if (!IS_DFIFO_EMPTY())
    {
        DFIFO_READ(muestra);
    }
    else
    {
        muestra = 0;  
    }
   
    dac_convertir(muestra/64 + 512);
}
