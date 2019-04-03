/*
 * somparser.c
 *
 *  Created on: 26 ott 2018
 *      Author: gabbla
 */
#include "somparser.h"
#include "system/random/sys_random.h"

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
        p->mid = SYS_RANDOM_PseudoGet();
        p->payload = NULL;
    }
    return p;
}

Packet *PACKET_CreateForReply(const Packet *p) {
    Packet *pp = PACKET_Create();
    pp->cmd = p->cmd;
    pp->dst = p->src;
    pp->src = p->dst;
    pp->tid = p->tid;
    pp->mid = SYS_RANDOM_PseudoGet();
    return pp;
}

PACKET_CODE copyPacket(const Packet *src, Packet *dst) {
	memcpy((void*) dst, (void*) src, PACKET_BASE_LEN);
	if (src->pLen) {
		dst->payload = malloc(dst->pLen);
		if (dst->payload == NULL)
			return PACKET_NO_MEM;
		memcpy((void*) dst->payload, (void*) src->payload, dst->pLen);
	}
	return PACKET_OK;
}

void PACKET_Free(Packet *p) {
	SYS_ASSERT(p != NULL, "Packet is null");
	if (p->pLen)
		free(p->payload);
}

Packet *PACKET_CreatePositionStatus(const ChannelIndex idx, const ChannelStatus sts){
    Packet *p;
    if((p = PACKET_Create()) != NULL) {
        p->cmd = BLE_CMD_POS_STATUS;
        p->pLen = 1;
        p->payload = malloc(p->pLen);
        p->payload[0] = (idx << 4) | sts;
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

uint16_t PACKET_GetTransactionId(const Packet *p) {
	SYS_ASSERT(p != NULL, "Packet is null");
	return p->tid;
}
uint8_t PACKET_GetCommand(const Packet *p) {
	SYS_ASSERT(p != NULL, "Packet is null");
	return p->cmd;
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
