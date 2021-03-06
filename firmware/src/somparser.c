/*
 * somparser.c
 *
 *  Created on: 26 ott 2018
 *      Author: gabbla
 */
#include "somparser.h"
#include "system/random/sys_random.h"

uint32_t swapEndian(const uint32_t value) {
    return (((value >> 24) & 0x000000ff) | ((value >> 8) & 0x0000ff00) |
	    ((value << 8) & 0x00ff0000) | ((value << 24) & 0xff000000));
 }

SOM_INLINE int8_t PACKET_SetSource(Packet *p, const uint8_t src) {
    SYS_ASSERT(p != NULL, "PACKET_SetSource() Packet is null");
    p->src = src;
    return 0;
}

SOM_INLINE int8_t PACKET_SetDestination(Packet *p, const uint8_t dst) {
    SYS_ASSERT(p != NULL, "PACKET_SetDestination() Packet is null");
    p->dst = dst;
    return 0;
}

SOM_INLINE int8_t PACKET_SetTransactionID(Packet *p, const uint32_t tid) {
    SYS_ASSERT(p != NULL, "PACKET_SetTransactionID() Packet is null");
    p->tid = swapEndian(tid);
    return 0;
}

SOM_INLINE int8_t PACKET_SetMessageID(Packet *p, const uint32_t mid) {
    SYS_ASSERT(p != NULL, "PACKET_SetMessageID() Packet is null");
    p->mid = swapEndian(mid);
    return 0;
}

SOM_INLINE int8_t PACKET_SetCommand(Packet *p, const uint8_t cmd) {
    SYS_ASSERT(p != NULL, "PACKET_SetCommand() Packet is null");
    p->cmd = cmd;
    return 0;
}

int8_t PACKET_SetPayload(Packet *p, uint8_t *payload, size_t len) {
    SYS_ASSERT(p != NULL, "PACKET_SetPayload() Packet is null");
    p->pLen = len;
    if(p->payload)
        free(p->payload);
    p->payload = malloc(len);
    if(!p->payload)
        return -1;
    memcpy((void*)p->payload, payload, len);
    return 0;
}

SOM_INLINE uint8_t PACKET_GetSource(const Packet *p) {
    SYS_ASSERT(p != NULL, "PACKET_GetSource() Packet is null");
    return p->src;
}

SOM_INLINE uint8_t PACKET_GetDestination(const Packet *p) {
    SYS_ASSERT(p != NULL, "PACKET_GetDestination() Packet is null");
    return p->dst;
}

SOM_INLINE uint32_t PACKET_GetTransactionID(const Packet *p) {
    SYS_ASSERT(p != NULL, "PACKET_GetTransactionId() Packet is null");
    return swapEndian(p->tid);
}

SOM_INLINE uint32_t PACKET_GetMessageID(const Packet *p) {
    SYS_ASSERT(p != NULL, "PACKET_GetMessageID() Packet is null");
    return swapEndian(p->mid);
}

SOM_INLINE uint8_t PACKET_GetCommand(const Packet *p) {
    SYS_ASSERT(p != NULL, "PACKET_GetCommand() Packet is null");
    return p->cmd;
}

SOM_INLINE size_t PACKET_GetPayload(const Packet *p, uint8_t *payload) {
    SYS_ASSERT(p != NULL, "PACKET_GetPayload() Packet is null");
    if(!p->payload) {
        payload = NULL;
        return 0;
    }
    memcpy(payload, p->payload, p->pLen);
    return p->pLen;
}

uint8_t PACKET_IsRawValid(const uint8_t *raw) {
	return ((raw[FIELD_PREAMBLE0] == PREAMBLE0_VAL)
			&& (raw[FIELD_PREAMBLE1] == PREAMBLE1_VAL));
}

void PACKET_Init(Packet *p) {
	SYS_ASSERT(p != NULL, "Packet is null");
	static const uint8_t preamble[] = { PREAMBLE0_VAL, PREAMBLE1_VAL };
	memcpy(p->preamble, preamble, sizeof(preamble));
	p->pLen = 0;
	p->payload = NULL;
}


Packet *PACKET_Get(const uint8_t *raw) {
    Packet *p = malloc(sizeof(Packet));
	memcpy(p, raw, PACKET_BASE_LEN);
	if (p->pLen) {
		p->payload = malloc(p->pLen);
		memcpy((void*) p->payload, &raw[PAYLOAD_START], p->pLen); // FIXME FIELD_PKTLEN may be wrong
	} else {
        p->payload = NULL;
    }
    return p;
}

Packet *PACKET_Create(){
    Packet *p = malloc(sizeof(Packet));
    if(p != NULL){
        memset(p, 0, sizeof(Packet));
        p->preamble[0] = PREAMBLE0_VAL;
        p->preamble[1] = PREAMBLE1_VAL;
		PACKET_SetTransactionID(p, SYS_RANDOM_PseudoGet());
		PACKET_SetMessageID(p, SYS_RANDOM_PseudoGet());
        p->payload = NULL;
    }
    return p;
}

Packet *PACKET_CreateForReply(const Packet *p) {
    Packet *pp = PACKET_Create();
	PACKET_SetCommand(pp, PACKET_GetCommand(p));
    PACKET_SetDestination(pp, PACKET_GetSource(p));
    PACKET_SetSource(pp, PACKET_GetDestination(p));
    PACKET_SetTransactionID(pp, PACKET_GetTransactionID(p));
    PACKET_SetMessageID(pp, SYS_RANDOM_PseudoGet());
    return pp;
}

int8_t copyPacket(const Packet *src, Packet *dst) {
	memcpy((void*) dst, (void*) src, PACKET_BASE_LEN);
	if (src->pLen) {
		dst->payload = malloc(dst->pLen);
		if (dst->payload == NULL)
			return -1;
		memcpy((void*) dst->payload, (void*) src->payload, dst->pLen);
	}
	return 0;
}

void PACKET_Free(Packet *p) {
	SYS_ASSERT(p != NULL, "Packet is null");
	if (p->pLen)
		free(p->payload);
    free(p);
}

Packet *PACKET_CreatePositionStatus(const ChannelIndex idx, const ChannelStatus sts){
    Packet *p;
    if((p = PACKET_Create()) != NULL) {
        PACKET_SetCommand(p, BLE_CMD_POS_STATUS);
        uint8_t data[] = { CH_IDX_TO_MASK(idx), sts << idx};
        PACKET_SetPayload(p, data, 2);
    }
    return p;
}

Packet *PACKET_CreateBatteryPacket(const BQ27441_Command cmd, const uint16_t data){
    Packet *p;
    if((p = PACKET_Create()) != NULL) {
        p->cmd = BLE_CMD_BAT_DATA;
        p->pLen = 3;
        p->payload = malloc(p->pLen);
        p->payload[0] = cmd;
        p->payload[1] = HIBYTE(data);
        p->payload[2] = LOBYTE(data);
    }
    return p;
}

Packet *PACKET_FillBatteryData(Packet *p, const BQ27441_Command cmd,
                               const uint16_t data) {
    if(!p)
        return p;
    uint8_t payload[] = {cmd, HIBYTE(data), LOBYTE(data)};
    PACKET_SetPayload(p, payload, 3);
    return p;
}

void PACKET_GetByteArray(const Packet *p, uint8_t byteArray[]) {
	SYS_ASSERT(p != NULL, "Packet is null");
	memcpy((void*) byteArray, (void*) p, PACKET_BASE_LEN);
	if (p->pLen)
		memcpy(&byteArray[PAYLOAD_START], p->payload, p->pLen);
}

size_t PACKET_GetFullSize(const Packet *p){
    return PACKET_BASE_LEN + p->pLen;
}

int8_t PACKET_GetRunMode(const Packet *p, RunMode *mode, uint8_t *channel) {
    SYS_ASSERT(p != NULL, "PACKET_GetRunMode() Packet is null");
    if (PACKET_GetCommand(p) != BLE_CMD_MODE) {
        ERROR(
            "Packet TID: 0x%08X MID: 0x%08X cmd 0x%02X not BLE_CMD_MODE "
            "[0x%02X]",
            PACKET_GetTransactionID(p), PACKET_GetMessageID(p),
            PACKET_GetCommand(p), BLE_CMD_MODE);
        return -1;
    }
    uint8_t dummy[2];
    size_t res;
    if ((res = PACKET_GetPayload(p, dummy)) < 2) {
        ERROR("Error while getting payload TID: 0x%08X MID: 0x%08X",
              PACKET_GetTransactionID(p), PACKET_GetMessageID(p));
        return -1;
    }
    DEBUG("Packet TID: 0x%08X MID: 0x%08X Mode: 0x%02X Ch: 0x%02X",
          PACKET_GetTransactionID(p), PACKET_GetMessageID(p), dummy[0], dummy[1]);
    *mode = dummy[0];
    *channel = dummy[1];
    return res;
}


