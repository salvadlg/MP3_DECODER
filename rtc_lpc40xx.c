/***************************************************************************//**
 * \file	rtc_lpc4088.c 
 * 
 * \brief	Funciones de acceso al RTC del LPC4088.
 *
 */

#include <LPC407x_8x_177x_8x.h>
#include "rtc_lpc40xx.h"

/***************************************************************************//**
 * \brief   Inicializar el LCD.
 *
 */
void rtc_inicializar(void)
{
    LPC_RTC->CCR = 1;
}

/***************************************************************************//**
 * \brief   Leer la hora del RTC.
 *
 * \param[out]  horas       puntero a la variable donde guardar las horas.
 * \param[out]  minutos     puntero a la variable donde guardar los minutos.
 * \param[out]  segundos    puntero a la variable donde guardar los segundos.
 *
 */
void rtc_leer_hora(uint32_t *horas, uint32_t *minutos,
                   uint32_t *segundos)
{
    int temp;

    temp = LPC_RTC->CTIME0;         /* Leer registro consolidado con la hora. */
    *segundos = temp & 0x3F;        /* Extraer segundos de los bits 0 a 5. */
    *minutos = (temp >> 8) & 0x3F;  /* Extraer minutos de los bits 8 a 13. */
    *horas = (temp >> 16) & 0x1F;   /* Extraer horas de los bits 16 a 20. */
}

/***************************************************************************//**
 * \brief   Ajustar la hora del RTC.
 *
 * \param[in]   horas       Horas a ajustar.
 * \param[in]   minutos     Minutos a ajustar.
 * \param[in]   segundos    Segundos a ajustar.
 *
 */
void rtc_ajustar_hora(uint32_t horas,
                      uint32_t minutos,
                      uint32_t segundos)
{
    LPC_RTC->CCR &= ~1;     /* Deshabilitar el avance del RTC mientras se ajusta
                             * la hora. */
    LPC_RTC->SEC = segundos;
    LPC_RTC->MIN = minutos;
    LPC_RTC->HOUR = horas;

    LPC_RTC->CCR |= 1;      /* Volver a permitir que el RTC avance. */
}

/***************************************************************************//**
 * \brief   Leer la fecha del RTC.
 *
 * \param[out]  dia     puntero a la variable donde guardar el día del mes.
 * \param[out]  mes     puntero a la variable donde guardar el mes.
 * \param[out]  ano     puntero a la variable donde guardar el año.
 *
 */
void rtc_leer_fecha(uint32_t *dia,
                    uint32_t *mes,
                    uint32_t *ano)
{
    int temp;

    temp = LPC_RTC->CTIME1;     /* Leer registro consolidado con la fecha. */
    *dia = temp & 0x1F;         /* Extraer el dia de los bits 0 a 4. */
    *mes = (temp >> 8) & 0x0F;  /* Extraer el mes de los bits 8 a 11. */
    *ano = (temp >> 16) & 0xFFF;/* Extraer el año de los bits 16 a 27. */
}

/***************************************************************************//**
 * \brief   Ajustar la fecha del RTC.
 *
 * \param[in]   dia     Día a ajustar.
 * \param[in]   mes     Mes a ajustar.
 * \param[in]   ano     Año a ajustar.
 *
 */
void rtc_ajustar_fecha(uint32_t dia,
                       uint32_t mes,
                       uint32_t ano)
{
    LPC_RTC->CCR &= ~1;     /* Deshabilitar el avance del RTC mientras se ajusta
                             * la fecha. */
    LPC_RTC->DOM = dia;
    LPC_RTC->MONTH = mes;
    LPC_RTC->YEAR = ano;

    LPC_RTC->CCR |= 1;      /* Volver a permitir que el RTC avance. */
}
