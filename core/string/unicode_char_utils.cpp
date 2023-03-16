#define USE_GARR_SIZE
#define USE_GARR_SIZE_TEMPLATED
#include "../typedefs.h"

#include "unicode_char_utils.h"

bool XID_START_CONTAINS(char32_t chr)
{
	int left = 0;
	int right = (int)(_GARR_SIZE(unicode_xid_start)) - 1;

	while (left <= right) {
		int mid = (left + right) / 2;
		if (chr < unicode_xid_start[mid].start) {
			right = mid - 1;
		}
		else if (chr > unicode_xid_start[mid].end) {
			left = mid + 1;
		}
		else {
			return true;
		}
	}
	return false;
}

bool XID_CONTINUE_CONTAINS(char32_t chr)
{
	int left = 0;
	int right = (int)(_GARR_SIZE(unicode_xid_continue)) - 1;

	while (left <= right) {
		int mid = (left + right) / 2;
		if (chr < unicode_xid_continue[mid].start) {
			right = mid - 1;
		}
		else if (chr > unicode_xid_continue[mid].end) {
			left = mid + 1;
		}
		else {
			return true;
		}
	}
	return false;
}