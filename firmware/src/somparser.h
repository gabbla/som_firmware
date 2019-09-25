/*
 * somparser.h
 *
 *  Created on: 26 ott 2018
 *      Author: gabbla
 */

#ifndef PARSER_SOMPARSER_H_
#define PARSER_SOMPARSER_H_

#include <stdint.h>  // uint8_t ecc
#include <stdlib.h>  // malloc, free
#include <string.h>  // memcpy
#include <stdio.h>   // sprintf
#include "bq27441_parser.h"
#include "channel_common.h"
#include "logger.h"

#ifdef __XC32
#include "system/random/sys_random.h"
#elif defined __linux__
#include <stdbool.h>
#include <stdlib.h>
#define SYS_RANDOM_PseudoGet() random()
#endif

// Preamble
#define PREAMBLE0_VAL (0x53)  // DEC 83
#define PREAMBLE1_VAL (0x4D)  // DEC 77

#define PACKET_BASE_LEN 14
#define MAX_PAYLOAD_LEN 255
#define MAX_PACKET_LEN (PACKET_BASE_LEN + MAX_PAYLOAD_LEN)

// Byte position
#define FIELD_PREAMBLE0 0
#define FIELD_PREAMBLE1 1
#define FIELD_SRC 2
#define FIELD_DST 3
#define FIELD_TID 4
#define FIELD_MID 8
#define FIELD_PKTLEN 12
#define FIELD_COMMAND 13
#define PAYLOAD_START 14

#define LOBYTE(w) ((uint8_t)(w))
#define HIBYTE(w) ((uint8_t)(((uint16_t)(w) >> 8) & 0xFF))
// Command return code
#define PING_OK (0x00)



typedef enum __attribute__((packed)) _commands {
    // 0x00 to 0x0F commands that are not propagated, managed by BLE APP
    BLE_CMD_PING = 0x00,
    BLE_CMD_GET_SN = 0x01,
    BLE_CMD_GET_INFO = 0x02,
    BLE_CMD_ACK = 0x03,
    BLE_CMD_SYS_RESET = 0x04,

    BLE_CMD_GET_BAT_DATA = 0x0A,
    // 0x10 - 0x4F commands forwarded to MainApp
    BLE_CMD_MODE = 0x10,
    BLE_CMD_START_POS = 0x11,
    BLE_CMD_DONE_POS = 0x12,
    BLE_CMD_GET_CHANNEL_STS = 0x13,

    // 0x50 - 0x?? messages generated by the system
    BLE_CMD_START_GATE = 0x50,
    BLE_CMD_POS_STATUS = 0x51,
    BLE_CMD_RUN_RESULTS = 0x52,
    BLE_CMD_CH_STATUS = 0x53,
    BLE_CMD_GATE_CROSSED = 0x54,

    BLE_CMD_BAT_DATA = 0x6A,
    BLE_CMD_NEW_SLAVE = 0x70,
    // 0xE0 - 0xEF Nrf commands
    BLE_CMD_DISCOVERY = 0xE0,
    BLE_CMD_ANNOUNCE = 0xE1,
    BLE_CMD_DISCOVERY_ACK = 0xE2,
    // 0xF0 - 0xFF Special meaning
    BLE_CMD_RESPONSE = 0xF0,
    BLE_CMD_NOT_SUPPORTED = 0xF1,
    // MUST always be the last enum
    BLE_CMD_MAX_CMD
} BLECommand;

#define MAINAPP_CMD_OFFSET (BLE_CMD_MODE)

typedef struct _packet {
    uint8_t preamble[2];
    uint8_t src;
    uint8_t dst;
    uint32_t tid;
    uint32_t mid;
    uint8_t pLen;
    uint8_t cmd;
    uint8_t *payload;
} Packet;

typedef enum {
    RUN_MODE_INVALID = -1,
    RUN_MODE_NONE = 0,
    RUN_MODE_POSITIONING,
    RUN_MODE_FREE_START,
    RUN_MODE_KO,
    RUN_MODE_LAP,

    RUN_MODE_CNT
} RunMode;

typedef enum {
    CMD_RESPONSE_FAIL = -1,
    CMD_RESPONSE_OK = 0,
} CommandResponse;

typedef enum {
    DEV_INVALID = (0),
    DEV_APPLICATION = (1 << 0),
    DEV_MASTER = (1 << 1),
    DEV_SLAVE0 = (1 << 2),
    DEV_SLAVE1 = (1 << 3),
    DEV_SLAVE2 = (1 << 4),
    DEV_SLAVE3 = (1 << 5),
    DEV_SLAVE4 = (1 << 6),
    DEV_ALL_SLAVES = DEV_SLAVE0 | DEV_SLAVE1 | DEV_SLAVE2 | DEV_SLAVE3 | DEV_SLAVE4,
} Device;

typedef struct {
    uint8_t sn[14];
    soc_t soc;
    bool charging;
} AnnouncePayload;

// Access utilities
#define SOM_INLINE inline

SOM_INLINE int8_t PACKET_SetSource(Packet *p, const uint8_t src);
SOM_INLINE int8_t PACKET_SetDestination(Packet *p, const uint8_t dst);
SOM_INLINE int8_t PACKET_SetSource(Packet *p, const Device src) __attribute__((deprecated));
SOM_INLINE int8_t PACKET_SetTransactionID(Packet *p, const uint32_t tid);
SOM_INLINE int8_t PACKET_SetMessageID(Packet *p, const uint32_t mid);
SOM_INLINE int8_t PACKET_SetCommand(Packet *p, const uint8_t cmd);
SOM_INLINE int8_t PACKET_SetPayload(Packet *p, void *payload, size_t len);

SOM_INLINE uint8_t PACKET_GetSource(const Packet *p);
SOM_INLINE uint8_t PACKET_GetDestination(const Packet *p);
SOM_INLINE uint32_t PACKET_GetTransactionID(const Packet *p);
SOM_INLINE uint32_t PACKET_GetMessageID(const Packet *p);
SOM_INLINE uint8_t PACKET_GetCommand(const Packet *p);
SOM_INLINE size_t PACKET_GetPayload(const Packet *p, void *payload);

uint8_t PACKET_IsRawValid(const uint8_t *raw);
void PACKET_Init(Packet *p);

Packet *PACKET_Get(const uint8_t *raw);
Packet *PACKET_Create();
Packet *PACKET_CreateForReply(const Packet *p);
void PACKET_Free(Packet *p);

// Helper function for self-generated packets
typedef bool LASER_STATUS;
Packet *PACKET_CreatePositionStatus(const ChannelIndex idx,
                                    const ChannelStatus sts);
Packet *PACKET_CreateBatteryPacket(const bool charging, const soc_t soc);

Packet *PACKET_CreateGateCrossPacket(const ChannelIndex idx);

Packet *PACKET_FillBatteryData(Packet *p, const BQ27441_Command cmd,
                               const uint16_t data);

/* Starting from now, every API to get data from packets payload must return
 * the number of read bytes (aka pLen) or negative error codes
 */
int8_t PACKET_GetRunMode(const Packet *p, RunMode *mode, uint8_t *channel);

// Utils

void PACKET_GetByteArray(const Packet *p, uint8_t byteArray[]);
size_t PACKET_GetFullSize(const Packet *p);

// NRF connections
Packet *PACKET_CreateDiscovery();
Packet *PACKET_CreateAnnounce(const uint8_t *sn, const soc_t soc,
                              const bool charging);
AnnouncePayload PACKET_GetAnnouncePayload(const Packet *p);
Packet *PACKET_CreateNewSlave(AnnouncePayload *ap);

Packet *PACKET_CreateRunResult(const ChannelIndex idx, const uint32_t timeUs);

#endif /* PARSER_SOMPARSER_H_ */
