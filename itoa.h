#ifndef _ITOA_H_
#define _ITOA_H_

#define ITOA_FORCESIGN 1
#define ITOA_NEGSIGNONLY 0
/** Convert integer value into base-10 string result.
 *
 * result will NOT be null-terminated by this function!
 * Whatever number you convert, make sure maxlen is one byte more than the
 * actual maximum digit count.  That additional byte is reserved for the
 * negative sign if applicable.
 * Set forcesign true to enforce a '+' character prepended for 0 and above.
 *
 * returns the number of bytes written into result.
 */
static inline unsigned char
itoa(int value, char* result, int maxlen, char forcesign)
{
	char* ptr = result;
	char* ptr1 = result;
	char bytecount = 0;
	char tmp_char;
	int tmp_value;

	// numeric conversion, result is now in _reverse_ order
	do {
		tmp_value = value;
		value /= 10;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * 10)];
		bytecount++;
	} while (value && (bytecount < maxlen - 1));

	// Apply sign
	if (tmp_value < 0) {
		*ptr++ = '-';
		bytecount++;
	} else if (forcesign) {
		*ptr++ = '+';
		bytecount++;
	}

	// Reverse digit order to correct one
	ptr--;
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return bytecount;
}
#endif /* include guard */
