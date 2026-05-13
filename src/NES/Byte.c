#include "Byte.h"

u8 AssignBit(u8 field, u8 index, bool isSet) {
	field = ClearBit(field, index);
	if (isSet) field = SetBit(field, index);
	return field;
}
