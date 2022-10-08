#include "AdvancedfxTypes.h"
#include "AdvancedfxUuid.h"

#include "stdio.h"

void WinMain()
{
	struct AdvancedfxUuid_t uuid = {ADVANCEDFX_UUID_UUID};
	union {
		struct AdvancedfxUuid_t uuid;
		struct {
			AdvancedfxUInt8_t uc0;
			AdvancedfxUInt8_t uc1;
			AdvancedfxUInt8_t uc2;
			AdvancedfxUInt8_t uc3;
			AdvancedfxUInt8_t uc4;
			AdvancedfxUInt8_t uc5;
			AdvancedfxUInt8_t uc6;
			AdvancedfxUInt8_t uc7;
			AdvancedfxUInt8_t uc8;
			AdvancedfxUInt8_t uc9;
			AdvancedfxUInt8_t ucA;
			AdvancedfxUInt8_t ucB;
			AdvancedfxUInt8_t ucC;
			AdvancedfxUInt8_t ucD;
			AdvancedfxUInt8_t ucE;
			AdvancedfxUInt8_t ucF;
		} bytes;
	} uuid_union;

	uuid_union.uuid = uuid;

	printf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
		uuid_union.bytes.uc0,
		uuid_union.bytes.uc1,
		uuid_union.bytes.uc2,
		uuid_union.bytes.uc3,
		uuid_union.bytes.uc4,
		uuid_union.bytes.uc5,
		uuid_union.bytes.uc6,
		uuid_union.bytes.uc7,
		uuid_union.bytes.uc8,
		uuid_union.bytes.uc9,
		uuid_union.bytes.ucA,
		uuid_union.bytes.ucB,
		uuid_union.bytes.ucC,
		uuid_union.bytes.ucD,
		uuid_union.bytes.ucE,
		uuid_union.bytes.ucF
	);
}
