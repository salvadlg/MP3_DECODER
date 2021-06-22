/***************************************************************************//**
 * \file	rtc_lpc4088.h
 * 
 * \brief	Funciones de acceso al RTC del LPC4088.
 *
 */

#ifndef RTC_LPC40XX_H
#define RTC_LPC40XX_H

#include "tipos.h"

void rtc_inicializar(void);

void rtc_leer_hora(uint32_t *horas,
                   uint32_t *minutos,
                   uint32_t *segundos);

void rtc_ajustar_hora(uint32_t horas,
                      uint32_t minutos,
                      uint32_t segundos);

void rtc_leer_fecha(uint32_t *dia,
                    uint32_t *mes,
                    uint32_t *ano);

void rtc_ajustar_fecha(uint32_t dia,
                       uint32_t mes,
                       uint32_t ano);


#endif  /* RTC_LPC40XX_H */
