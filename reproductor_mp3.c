/***************************************************************************//**
 * \file    reproductor_mp3.c
 *
 * \brief   Este fichero contiene las funciones que usan conjuntamente la
 *          biblioteca de sistemas de archivos FatFs y la biblioteca de
 *          decodificaci�n del MP3 libmad para llevar a cabo la reproducci�n
 *          del fichero MP3 almacenado en el disco o memoria que se
 *          haya montado en FatFs.
 *
 *          La �nica funci�n p�blica que ofrece este m�dulo es reproducir_mp3,
 *          a la que se llama con un manejador al fichero a reproducir. El
 *          manejador se habr� obtenido mediante una llamada previa la
 *          funci�n f_open de FatFs.
 *
 *          El resto de funciones son las funciones "callback" que libmad
 *          llamar� para obtener datos del stream MP3 (funci�n input), entregar
 *          bloques de muestras de audio decodificadas (funci�n output) e
 *          indicar errores durante el proceso de reproducci�n (funci�n error).
 */
 
#include <LPC407x_8x_177x_8x.h>
#include "mad.h"
#include "salida_audio.h"
#include "reproductor_mp3.h"
#include <stdio.h>
#include <ff.h>
#include <string.h>
#include "tipos.h"
#include "joystick.h"
#include "timer_lpc40xx.h"
#include "glcd.h"

/* El siguiente b�ffer act�a como una FIFO que va siendo rellenada con datos
 * procedentes del fichero MP3 y del que el decodificador los va tomando para
 * procesarlos. Cada bloque de datos procesado va siendo retirado dejando un
 * hueco que ser� rellenado con nuevo datos del fichero.
 * Se reserva espacio para un total de 3 sectores de disco (512 bytes cada uno).
 */
static uint8_t buffer_stream_mp3[512];

/* Manejador al fichero MP3 que se est� reproduciendo. Global (pero privado para
 * este m�dulo) porque varias funciones de m�dulo necesitan acceder a �l.
 */
static FIL *manejador_fichero_mp3;

/* Tasa de muestreo actual de la salida de audio. Un valor 0 indica que a�n no
 * ha sido inicializada con una tasa de muestreo v�lida.
 */
static uint32_t tasa_muestreo_actual = 0;

/* La estructura buffer_info se usa para guardar informaci�n sobre un bloque de
 * bytes gen�rico. Tiene campos para indicar la direcci�n de comienzo del bloque
 * y su longitud.
 */
struct buffer_info {
    uint8_t const *comienzo;
    uint32_t longitud;
};

/* Funciones "callback" que libmad llamar� para obtener datos del stream MP3
 * (funci�n input), entregar bloques de muestras de audio decodificadas
 * (funci�n output) e indicar errores durante el proceso de reproducci�n
 * (funci�n error).
 */
static enum mad_flow input(void *data, struct mad_stream *stream);
static enum mad_flow output(void *data,
                            struct mad_header const *header,
                            struct mad_pcm *pcm);
static enum mad_flow error(void *data,
		                   struct mad_stream *stream,
		                   struct mad_frame *frame);

/***************************************************************************//**
 * \brief       Lanza la reproducci�n de un fichero MP3. La funci�n no retorna
 *              hasta que no termina la reproducci�n.
 *
 * \param[in]   manejador_fichero   manejador al fichero a reproducir obtenido
 *                                  mediante una llamada previa a f_open.
 *
 * \return      c�digo de salida de la funci�n mad_decoder_run.
 */
int32_t reproducir_mp3(FIL *manejador_fichero)
{ 
    struct buffer_info buffer;
    struct mad_decoder decoder;
    int32_t resultado;
    
    /* Inicializar tasa_muestro_actual con valor inicial inv�lido que ser�
     * cambiado cuando el decodificador sepa la tasa de muestro del fichero
     * que se va a reproducir.
     */
    tasa_muestreo_actual = 0; 
    
    /* Inicializar el sistema de generaci�n de audio que llevar� las muestras
     * de audio decodificadas al altavoz/altavoces o salida de audio concreta
     * usando DAC, I2S, etc.
     * Este m�dulo es independiente respecto del sistema de generaci�n de audio
     * concreto que se use. Los detalles estar�n en la implementaci�n de
     * generaci�n de audio usada.
     */
    salaud_inicializar();
    
    /* Inicializar la variable global est�tica manejador_fichero_mp3 que la
     * funci�n input usar� para acceder al fichero en reproducci�n.
     */
    manejador_fichero_mp3 = manejador_fichero;

    /* Se inicializan los campos de buffer (de tipo buffer_info) para que
     * inicialmente indique la disponibilidad de todo el buffer_stream_mp3
     * para que la funci�n input coloque datos del fichero en �l
     * (ver los comentarios de buffer_stream_mp3).
     *
     * Las funciones callback input, output y error reciben un puntero
     * a esta estructura a trav�s del argumento data.
     */
    buffer.comienzo  = buffer_stream_mp3;
    buffer.longitud = sizeof(buffer_stream_mp3);

    /* Inicializar el decodificador MP3 de libmad indic�ndole las funciones
     * input, output y error que queremos que use.
     */
    mad_decoder_init(&decoder, 
                     &buffer,
                     input, 
                     NULL, /* header callback */
                     NULL, /* filter callback */
                     output,
                     error, 
                     NULL); /* message callback */

    /* Comenzar la decodificaci�n.
		*/
    resultado = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

    /* "Cerrar" o finalizar el decodificador.
     */
    mad_decoder_finish(&decoder);

    /* Retornar el valor que devolvi� mad_decoder_run.
     */
    return resultado;    
}

/***************************************************************************//**
 * \brief       Esta es la funci�n a la que libmad llamar� cada vez que quiera
 *              rellenar parte (o todo) el buffer de entrada con nuevos datos
 *              del stream MP3. En nuestro el stream MP3 procede de un fichero
 *              as� que leeremos del fichero un bloque de datos del tama�o
 *              necesario.
 *
 * \param[in]   data    puntero a datos de usuario que libmad pasa a la funci�n.
 *                      En la llamada a mad_decoder_init indicamos que queremos
 *                      que nos llegue un puntero a la structura buffer (de
 *                      tipo buffer_info) declarada en reproducir_mp3. Usamos
 *                      esta estrucura para ganar acceso al espacio disponible
 *                      en el buffer de entrada buffer_stream_mp3.
 *
 *              stream  puntero a estructura de tipo mad_stream que indica
 *                      informaci�n sobre el stream MP3 en reproducci�n.
 *
 * \return      MAD_FLOW_CONTINUE => todo correcto, la reproducci�n MP3 puede
 *                                   proseguir.
 *              MAD_FLOW_STOP     => error, la reproducci�n MP3 deber�a parar.
 */
static enum mad_flow input(void *data, struct mad_stream *stream)
{
    /* Convertir el puntero void data a un puntero a estructura de tipo
     * buffer_info.
     */
    struct buffer_info *buffer = data;
    uint32_t segundos_totales_reproduccion = timer_leer(TIMER2);    
    uint32_t rb = 0;
    uint32_t numero_bytes_leidos;    
    uint32_t segundos_reproduccion = segundos_totales_reproduccion % 60;
    uint16_t minutos_reproduccion = segundos_totales_reproduccion / 60;

    glcd_xprintf(325, 0, WHITE, BLACK, FONT8X16, "Duracion: %02u:%02u",
                 minutos_reproduccion, segundos_reproduccion);

    if(leer_joystick() == JOYSTICK_IZQUIERDA)
    {
        return MAD_FLOW_STOP;
    }
			
    /* Si el hueco en buffer_stream_mp3 es 0, error.
     */
    if (buffer->longitud == 0)
    {
        return MAD_FLOW_STOP;
    }

    /* Rellenar el buffer de entrada con el n�mero de bytes solicitado.
     * (Explicar mejor).
     */
    if (stream->this_frame != NULL && stream->next_frame != NULL)
    {
        rb = buffer->longitud -
            ((uint32_t)stream->next_frame - (uint32_t)stream->buffer);

        memmove((void *)stream->buffer, (void *)stream->next_frame, rb);
        f_read(manejador_fichero_mp3,
               (void *)(stream->buffer + rb),
               buffer->longitud - rb,
               &numero_bytes_leidos);
    }
    else
    {
        f_read(manejador_fichero_mp3,
               (void *)buffer->comienzo,
               buffer->longitud,
               &numero_bytes_leidos);
    }

    if (numero_bytes_leidos == 0)
    {
        /* Si no se pudieron obtener nuevos datos del fichero, terminar la reproducci�n de
         * los muestras de audio decodificadas hasta el momento e indicar parar la
         * reproducci�n.
         */
        salaud_esperar_fin_fragmento();
        buffer->longitud = 0;
        return MAD_FLOW_STOP;
    }
    else
    {
        /* Actualizar el n�mero de datos disponibles en el buffer de entrada.
         */        
        buffer->longitud = numero_bytes_leidos + rb;
    }

    /* Indicar a libmad el bloque de datos de entrada de que dispone para decodificar.
     */
    mad_stream_buffer(stream, buffer->comienzo, buffer->longitud);

    /* Seguir con la decodificaci�n.
     */
    return MAD_FLOW_CONTINUE;
}

/***************************************************************************//**
 * \brief       Esta es la funci�n a la que libmad llamar� cada vez que haya
 *              decodificado un nuevo frame MP3 para que las muestras de
 *              audio resultantes se env�en a la salida de audio.
 *
 * \param[in]   data    puntero a datos de usuario que libmad pasa a la funci�n.
 *                      En la llamada a mad_decoder_init indicamos que queremos
 *                      que nos llegue un puntero a la structura buffer (de
 *                      tipo buffer_info) declarada en reproducir_mp3. Usamos
 *                      esta estrucura para ganar acceso al espacio disponible
 *                      en el buffer de entrada buffer_stream_mp3.
 *
 *              header  puntero a estructura de tipo mad_header que ...
 *
 *              pcm     puntero a estructura de tipo mad_pcm que ...
 *
 * \return      MAD_FLOW_CONTINUE => todo correcto, la reproducci�n MP3 puede
 *                                   proseguir.
 *              MAD_FLOW_STOP     => error, la reproducci�n MP3 deber�a parar.
 */
static enum mad_flow output(void *data,
                            struct mad_header const *header,
                            struct mad_pcm *pcm)
{
    if (pcm->samplerate != tasa_muestreo_actual)
    {
        salaud_ajustar_tasa_muestreo(pcm->samplerate);
        tasa_muestreo_actual = pcm->samplerate;
    }

    salaud_encolar_bloque_muestras(pcm->samples[0],
                                   pcm->samples[1],
                                   pcm->length,
                                   pcm->channels);
    
    return MAD_FLOW_CONTINUE;
}

/***************************************************************************//**
 * \brief       Esta es la funci�n a la que libmad llamar� cada vez que se
 *              produzca un error de decodificaci�n.
 *
 * \param[in]   data    puntero a datos de usuario que libmad pasa a la funci�n.
 *                      En la llamada a mad_decoder_init indicamos que queremos
 *                      que nos llegue un puntero a la structura buffer (de
 *                      tipo buffer_info) declarada en reproducir_mp3. Usamos
 *                      esta estrucura para ganar acceso al espacio disponible
 *                      en el buffer de entrada buffer_stream_mp3.
 *
 *              stream  puntero a estructura de tipo mad_stream que. El campo
 *                      stream->error indica el error que se ha producido.
 *                      La lista de posibles errores puede consultarse en
 *                      mad.h o stream.h.
 *
 *              frame   puntero a estructura de tipo mad_frame que ...
 *
 * \return      MAD_FLOW_CONTINUE => todo correcto, la reproducci�n MP3 puede
 *                                   proseguir.
 *              MAD_FLOW_STOP     => error, la reproducci�n MP3 deber�a parar.
 */
static enum mad_flow error(void *data,
		                   struct mad_stream *stream,
		                   struct mad_frame *frame)
{
    /* Ignoramos el error e intentamos seguir decodificando.
     */
    return MAD_FLOW_CONTINUE;
}
