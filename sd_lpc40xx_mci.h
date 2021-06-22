/***************************************************************************//**
 *
 */

#ifndef SD_LPC40XX_MCI_H
#define SD_LPC40XX_MCI_H

#include "tipos.h"

#define MCI_CLOCK_ENABLE            (1u<<8)
#define MCI_CLOCK_PWRSAVE           (1u<<9)
#define MCI_CLOCK_BYPASS            (1u<<10)
#define MCI_CLOCK_WIDEBUS           (1u<<11)

#define MCI_COMMAND_RESPONSE        (1u<<6)
#define MCI_COMMAND_LONGRESP        (1u<<7)
#define MCI_COMMAND_INTERRUPT       (1u<<8)
#define MCI_COMMAND_PENDING         (1u<<9)
#define MCI_COMMAND_ENABLE          (1u<<10)

#define MCI_STATUS_CMDCRCFAIL       (1u<<0)
#define MCI_STATUS_CMDTIMEOUT       (1u<<2)
#define MCI_STATUS_CMDRESPEND       (1u<<6)
#define MCI_STATUS_CMDSENT          (1u<<7)
#define MCI_STATUS_DATABLOCKEND     (1u<<10)
#define MCI_STATUS_CMDACTIVE        (1u<<11)
#define MCI_STATUS_TXFIFOHALFEMPTY  (1u<<14)
#define MCI_STATUS_RXFIFOHALFFULL   (1u<<15)
#define MCI_STATUS_TXFIFOFULL       (1u<<16)
#define MCI_STATUS_RXFIFOFULL       (1u<<17)
#define MCI_STATUS_TXFIFOEMPTY      (1u<<18)
#define MCI_STATUS_RXFIFOEMPTY      (1u<<19)
#define MCI_STATUS_TXDATAAVLBL      (1u<<20)
#define MCI_STATUS_RXDATAAVLBL      (1u<<21)

#define NO_RESPONSE 0
#define SHORT_RESPONSE 1
#define LONG_RESPONSE 2

#define GO_IDLE_STATE 0
#define APP_CMD 55
#define SD_SEND_OP_COND 41
#define ALL_SEND_CID 2
#define SEND_RELATIVE_ADDR 3
#define SEND_STATUS 13
#define SELECT_DESELECT_CARD 7
#define SET_BUS_WIDTH 6
#define SEND_IF_COND 8
 
#define	CMDREAD             17
#define	CMDREADMULTIPLE		18
#define	CMDWRITE            24
#define	CMDWRITEMULTIPLE    24
#define	CMDREADCSD          9
#define CMD55               55
#define ACMD23              23
#define CMDSTOPTRANSMISION  12

#define MCI_PRESCALE_MIN  5

#define OCR_VOLTAGE_WINDOW 0x00FF8000

#define OCR_HCS (1u<<30)

uint32_t sd_init(void);
uint32_t sd_status(void);
uint32_t sd_read(uint8_t  *buff, int32_t sector, int32_t count);
uint32_t sd_write(const uint8_t  *buff, int32_t sector, int32_t count);
uint32_t sd_ioctl(int32_t ctrl, void *buff);
uint32_t sd_getfattime(void);
void mci_set_speed(int32_t speed);
uint32_t sd_command(uint32_t cmd, uint32_t resp_type, uint32_t arg, uint32_t* resp);

#endif  /* SD_LPC40XX_MCI_H */
