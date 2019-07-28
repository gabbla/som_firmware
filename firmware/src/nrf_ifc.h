#ifndef NRF_IFC_H_3P8AGRVE
#define NRF_IFC_H_3P8AGRVE

#include "system_definitions.h"
#include <peripheral/peripheral.h>
#include "memory_map.h"
#include "logger.h"

#define NRF_SPI_ID  SPI_ID_1

#define spi_active() PLIB_PORTS_PinClear(PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_0); asm("nop")
#define spi_idle()   PLIB_PORTS_PinSet(PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_0); asm("nop")

#define nrf_set_ce() PLIB_PORTS_PinClear(PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_15); asm("nop")
#define nrf_clr_ce() PLIB_PORTS_PinSet(PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_15); asm("nop")

typedef union {
    struct {
        uint8_t tx_full : 1;
        uint8_t rx_pipe_number : 3;
        uint8_t max_retransmissions : 1;
        uint8_t tx_data_sent : 1;
        uint8_t rx_data_ready : 1;
        uint8_t : 1;
    };
    uint8_t status;
} NRF_Status;

typedef enum {
    NRF_1MBPS = 0x00,
    NRF_2MBPS = 0x01,
    NRF_250KBPS = 0x02
} speed_t;

/*
 * @brief Initialize the chip at default state
 * @return Chip STATE register
 */
NRF_Status NRF_Initialize();

/*
 * @brief Get the chip STATUS register
 * @return Chip STATUS register
 */
NRF_Status NRF_GetStatus();

/*
 * @brief Turn On or Off the chip
 * @param power chip power
 * @return Chip STATUS register
 */
NRF_Status NRF_PowerEnable(const bool power);

NRF_Status NRF_SetMode(const uint8_t mode);
/*
 * @brief Clean the interrupts flags
 * @param int_mask Any ORed combination of RX_DR, TX_DS and MAX_RT
 * @return Chip STATUS register
 */
NRF_Status NRF_CleanInterrupts(const uint8_t int_mask);

/*
 * @brief Enable the interrupts reflection on IRQ pin
 * @param int_mask Any ORed combination of RX_DR,_TX_DS and MAX_RT
 * @return Chip STATUS register
 */
NRF_Status NRF_EnableInterrupts(const uint8_t int_mask);

/*
 * @brief Disable the interrupts reflection on IRQ pin
 * @param int_mask Any ORed combination of RX_DR,_TX_DS and MAX_RT
 * @return Chip STATUS register
 */
NRF_Status NRF_DisableInterrupts(const uint8_t int_mask);

/*
 * @brief Setup the address widthsm common for all data pipes
 * @param aw AW_x_BYTES where x = {3, 4, 5}. Cannot be ORed
 * @return Chip STATUS register
 */
NRF_Status NRF_SetAddressWidth(const uint8_t aw);

NRF_Status NRF_SetPipeAddress(const uint8_t pipe, const uint64_t address);

NRF_Status NRF_SetTxAddress(const uint64_t address);

NRF_Status NRF_OpenWritingPipe(const uint64_t addr);

NRF_Status NRF_CloseReadingPipe(const uint8_t pipe);
NRF_Status NRF_OpenReadingPipe(const uint8_t pipe, const uint64_t addr);

NRF_Status NRF_SetChannel(const uint8_t channel);

NRF_Status NRF_SetBaudrate(const speed_t baudrate);

/*
 * @brief Enable the desired features
 * @param features any ORed combination of EN_DPL, EN_ACK_PAY or EN_DYN_ACK
 * @return Chip STATUS register
 */
NRF_Status NRF_EnableFeatures(const uint8_t features);

NRF_Status NRF_EnableDynPayload(const uint8_t pipe);

NRF_Status NRF_EnableRxPipe(const uint8_t pipe);
NRF_Status NRF_EnableEnanchedShockBurst(const uint8_t pipe);

NRF_Status NRF_StartListening();

NRF_Status NRF_GetPayloadSize(const uint8_t pipe, uint8_t *size);

NRF_Status NRF_Write(const uint8_t pipe, const void *data, const size_t size);

bool NRF_Available(uint8_t *pipe);

NRF_Status NRF_DumpRegisters();

NRF_Status NRF_ReadPayload(void *buf, const size_t len);
/*
 * @brief Send the command FLUSH_TX to the chip
 * @return Chip STATUS register
 */
NRF_Status NRF_FlushTx();

/*
 * @brief Send the command FLUSH_RX to the chip
 * @return Chip STATUS register
 */
NRF_Status NRF_FlushRx();

#endif /* end of include guard: NRF_IFC_H_3P8AGRVE */
