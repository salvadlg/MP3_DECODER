/***************************************************************************//**
 */

#include <LPC407x_8x_177x_8x.h>
#include "rtc_lpc40xx.h"
#include "glcd.h"
#include <stdio.h>
#include "ff.h"
#include "reproductor_mp3.h"
#include "tipos.h"
#include "timer_lpc40xx.h"
#include <string.h>
#include "joystick.h"
#include "error.h"
#include "teclado_4x4.h"
#include <stdlib.h>

/* La siguiente definici�n de la funcion _ttywrch es s�lo para quitar el error
 * de linkado que generan las llamadas a printf y fprintf en libmad cuando no
 * se est� usando semihosting.
 */
void _ttywrch(int ch){}

//Estructura para almacenar informacion de la pista y luego utilizarla junto al teclado
typedef struct{
	uint32_t numero;
	char nombre[255];
} pista;


int main(){
    FATFS fs;           /* Estructura donde FatFs mantendr� la informaci�n
                         * sobre el sistema de archivos.
                         */
    FRESULT  fresult;   /* Muchas funciones de FatFs retornan un codigo de
                         * error de tipo FRESULT. Usaremos esta variable
                         * para recoger el codigo de error retornado y
                         * comprobar si indica exito (FR_OK) o no.
                         */
    FIL      fichero;   /* Estructura de  tipo FIL con informaci�n sobre el
                         * fichero abierto.
                         */
      	
    DIR dir;            /* Directory search object */
		
    FILINFO flinfo;     /* File information */
    
    /* Inicializar y borrar el LCD.*/  
  
    glcd_inicializar();
    glcd_borrar(NEGRO);

    //Inicializar teclado
    tec4x4_inicializar();

  	/* Inicializar el RTC y ajustar una fecha y una hora arbitrarias.
     * La funci�n get_fattime en sd_lpc40xx_mci.c usa el RTC para obtener
     * la hora y fecha del sistema cuando se crea un archivo.
     * En una aplicaci�n real el RTC estar�a mantenido por una bater�a
     * conectada al pin VBAT del LPC40xx.*/
		 
  	rtc_inicializar();
  	rtc_ajustar_fecha(07,06,2021);
  	rtc_ajustar_hora(11,00,00);

    /* Montar el sistema de archivos.*/	
    fresult = f_mount(&fs, "", 0);
    ASSERT(fresult == FR_OK, "Error al montar el sistema de ficheros");
		
		//inicializamos los timers que harán falta para la reproducción 
		timer_inicializar(TIMER0);
		timer_inicializar(TIMER1);
		timer_inicializar(TIMER2);

    
    /* Listar los ficheros mp3 en la tarjeta SD y reproducir la seleccionada*/
    while(TRUE){

        FRESULT fr, fr2;

        //numero para la siguiente fila en el LCD
        int32_t y = 96;

        //numero de la pista
        uint32_t numPista = 0;

        //estructura
        pista p[10];

				//valor char de lo que introducimos por teclado
        char buffOp[2];

        //cancion seleccionada
        char seleccion[255];
			
				//numero acorde a la cancion
				char num[1];
			
				//valor entero de lo que introducimos por teclado
				int op;

        glcd_xprintf(0, 0, WHITE, BLACK, FONT16X32, "  <-Cancion seleccionada");

        glcd_xprintf(0, 64, WHITE, BLACK, FONT16X32, "Listado de canciones:");

        //buscamos en la tarjeta SD solo los archivos que tengan extension .mp3
        fr = f_findfirst(&dir, &flinfo, "", "*.mp3");

        while (fr == FR_OK && flinfo.fname[0] && y < 225){
            //guardamos el numero de las pistas
            p[numPista].numero = numPista + 1;
            //guardamos el nombre de las pistas
            strcpy(p[numPista].nombre, flinfo.fname);
					
						sprintf(num, "%d", numPista + 1);
						strcat(num, ".");
						
						//imprimimos el numero de la cancion
						glcd_xprintf(0, y, WHITE, BLACK, FONT16X32, num);
            //imprimimos el titulo de la cancion.mp3
            glcd_xprintf(35, y, WHITE, BLACK, FONT16X32, flinfo.fname);

            fr = f_findnext(&dir, &flinfo);

            y += 32;
            numPista++;
        }

        //Termina la lectura cuando se pulsa '#'
        tec4x4_leer_cadena_numeros(buffOp, 2);

        //pasamos a entero el valor que hemos leido por teclado
        op = atoi(buffOp);

        //varible para salir del bucle
        uint32_t x = 0;

        while (x != 1) {
            glcd_borrar(NEGRO);

            //copiamos en la variable seleccion la pista que este en la posicion que hemos leido por teclado
            strcpy(seleccion, p[op - 1].nombre);

            glcd_xprintf(0, 0, WHITE, BLACK, FONT16X32, "Reproduciendo...");
            //imprimimos la pista que estamos reproduciendo
            glcd_xprintf(0, 32, WHITE, BLACK, FONT16X32, seleccion);

            //FA_OPEN_EXISTING no lo ponemos porque viene por defecto
            //abrimos el archivo con el nombre de la pista que hemos elegido
            //y lo guardamos en el fichero vacio
            fr2 = f_open(&fichero, seleccion, FA_READ);
            ASSERT(fr2 == FR_OK, "Error al abrir el archivo .mp3");

            reproducir_mp3(&fichero);

            f_close(&fichero);

            x = 1;
        }
        glcd_borrar(NEGRO);
    }
}
