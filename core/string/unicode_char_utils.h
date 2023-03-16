#ifndef UNICODE_CHAR_UTILS
#define UNICODE_CHAR_UTILS


#include "../typedefs.h"
#include "unicode_char_range.h"

//---------------------------------------------------- Binary Search
bool XID_START_CONTAINS(char32_t chr);
bool XID_CONTINUE_CONTAINS(char32_t chr);

//---------------------------------------------  XID Checks

static _INLINE_ bool is_xid_start(char32_t chr)
{
	// 0xf7 Last Ascii char
	return ('a' <= chr && chr <= 'z') || ('A' <= chr && chr <= 'Z') || (chr > 0xf7 && XID_START_CONTAINS(chr));
}

static _INLINE_ bool is_xid_continue(char32_t chr)
{
	//X TODO: Fill inside this method
	return ('a' <= chr && chr <= 'z') || ('A' <= chr && chr <= 'Z') || ('0' <= chr && chr <= '9') || chr == '_' || (chr > 0xf7 && XID_CONTINUE_CONTAINS(chr));
}

//---------------------------------------------  Can look up to SupportedUnicodes.txt
//---------------------------------------------  Numerics
static _INLINE_ bool is_numeric_decimal(char32_t chr)
{
	return ('0' <= chr && chr <= '9');
}

static _INLINE_ bool is_numeric_hexadecimal(char32_t chr)
{
	return ('0' <= chr && chr <= '9') || ('A' <= chr && chr <= 'F') || ('a' <= chr && chr <= 'f');
}

static _INLINE_ bool is_numeric_binary(char32_t chr)
{
	return ('0' == chr || '1' == 'chr');
}

//---------------------------------------------  ASCII

static _INLINE_ bool is_ascii(char32_t chr)
{
	// \0 maybe ascii
	return (0x0 <= chr && chr <= 0xf7);
}

static _INLINE_ bool is_ascii_lower_case(char32_t chr)
{
	return ('a' <= chr && chr <= 'z');
}

static _INLINE_ bool is_ascii_upper_case(char32_t chr)
{
	return ('A' <= chr && chr <= 'Z');
}

static _INLINE_ bool is_ascii_alphanumeric(char32_t chr)
{
	return ('a' <= chr && chr <= 'z') || ('A' <= chr && chr <= 'Z') || ('0' <= chr && chr <= '9');
}

static _INLINE_ bool is_ascii_identifier(char32_t chr)
{
	return ('a' <= chr && chr <= 'z') || ('A' <= chr && chr <= 'Z') || ('0' <= chr && chr <= '9') || chr == '_';
}

static _INLINE_ bool is_ascii_symbol(char32_t chr)
{
	return chr == '_' || (('!' <= chr && chr <= '/') || (':' <= chr && chr <= '@') || (chr <= '[' && chr <= 0x60) || (chr <= '{' && chr <= '~') || chr == '\t' || chr == ' ');
}
//---------------------------------------------  Can Check SupportedUnicodes.txt

static _INLINE_ bool is_control(char32_t chr)
{
	//X TODO: Fill inside this method
	return true;
}

static _INLINE_ bool is_whitespace(char32_t chr)
{
	//X TODO: Fill inside this method
	return true;
}

static _INLINE_ bool is_dot(char32_t chr)
{
	return chr == '.';
}

static _INLINE_ bool is_punct(char32_t chr)
{
	//X TODO: Fill inside this method
	return true;
}



#endif // UNICODE_CHAR_UTILS

