#ifndef ADVANCEDFX_UUID_H
#define ADVANCEDFX_UUID_H

#include "AdvancedfxTypes.h"

#define ADVANCEDFX_UUID_FROM_VARIANT2(time_low,time_mid,time_hi_and_version,clock_seq_hi_and_res__clock_seq_low,node_uc5,node_uc4,node_uc3,node_uc2,node_uc1,node_uc0) (AdvancedfxUInt32_t)time_low, (AdvancedfxUInt32_t)time_mid|(AdvancedfxUInt32_t)time_hi_and_version<<16, ((AdvancedfxUInt32_t)clock_seq_hi_and_res__clock_seq_low&0x00ff)<<8|((AdvancedfxUInt32_t)clock_seq_hi_and_res__clock_seq_low&0xff00)>>8|(AdvancedfxUInt32_t)node_uc4<<24|(AdvancedfxUInt32_t)node_uc5<<16, (AdvancedfxUInt32_t)node_uc0<<24|(AdvancedfxUInt32_t)node_uc1<<16|(AdvancedfxUInt32_t)node_uc2<<8|(AdvancedfxUInt32_t)node_uc3

#define ADVANCEDFX_UUID_BYTE_0(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul0 & 0xff000000) >> 24))
#define ADVANCEDFX_UUID_BYTE_1(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul0 & 0x00ff0000) >> 16))
#define ADVANCEDFX_UUID_BYTE_2(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul0 & 0x0000ff00) >> 8))
#define ADVANCEDFX_UUID_BYTE_3(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul0 & 0x000000ff) >> 0))
#define ADVANCEDFX_UUID_BYTE_4(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul1 & 0xff000000) >> 24))
#define ADVANCEDFX_UUID_BYTE_5(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul1 & 0x00ff0000) >> 16))
#define ADVANCEDFX_UUID_BYTE_6(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul1 & 0x0000ff00) >> 8))
#define ADVANCEDFX_UUID_BYTE_7(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul1 & 0x000000ff) >> 0))
#define ADVANCEDFX_UUID_BYTE_8(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul2 & 0xff000000) >> 24))
#define ADVANCEDFX_UUID_BYTE_9(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul2 & 0x00ff0000) >> 16))
#define ADVANCEDFX_UUID_BYTE_A(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul2 & 0x0000ff00) >> 8))
#define ADVANCEDFX_UUID_BYTE_B(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul2 & 0x000000ff) >> 0))
#define ADVANCEDFX_UUID_BYTE_C(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul3 & 0xff000000) >> 24))
#define ADVANCEDFX_UUID_BYTE_D(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul3 & 0x00ff0000) >> 16))
#define ADVANCEDFX_UUID_BYTE_E(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul3 & 0x0000ff00) >> 8))
#define ADVANCEDFX_UUID_BYTE_F(ul0,ul1,ul2,ul3) ((AdvancedfxUInt8)((ul3 & 0x000000ff) >> 0))

#define ADVANCEDFX_UUID_BYTES(uuid) ADVANCEDFX_UUID_BYTE_0(uuid),ADVANCEDFX_UUID_BYTE_1(uuid),ADVANCEDFX_UUID_BYTE_2(uuid),ADVANCEDFX_UUID_BYTE_3(uuid),ADVANCEDFX_UUID_BYTE_4(uuid),ADVANCEDFX_UUID_BYTE_5(uuid),ADVANCEDFX_UUID_BYTE_6(uuid),ADVANCEDFX_UUID_BYTE_7(uuid),ADVANCEDFX_UUID_BYTE_8(uuid),ADVANCEDFX_UUID_BYTE_9(uuid),ADVANCEDFX_UUID_BYTE_A(uuid),ADVANCEDFX_UUID_BYTE_B(uuid),ADVANCEDFX_UUID_BYTE_C(uuid),ADVANCEDFX_UUID_BYTE_D(uuid),ADVANCEDFX_UUID_BYTE_E(uuid),ADVANCEDFX_UUID_BYTE_F(uuid)

struct AdvancedfxUuid_t
{
	AdvancedfxUInt32_t uuid_0;
	AdvancedfxUInt32_t uuid_1;
	AdvancedfxUInt32_t uuid_2;
	AdvancedfxUInt32_t uuid_3;
};

#define ADVANCEDFX_UUID_UUID ADVANCEDFX_UUID_FROM_VARIANT2(0x9A58B3EA,0xB183,0x44DE,0x8B92,0x3F,0x7D,0xF1,0x25,0x7F,0xF7)

#endif