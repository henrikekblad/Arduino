
#include <avr/pgmspace.h>
prog_char string_0[] PROGMEM = "String 0";
prog_char string_1[] PROGMEM = "String 1";
prog_char string_2[] PROGMEM = "String 2";
prog_char string_3[] PROGMEM = "String 3";
prog_char string_4[] PROGMEM = "String 4";
prog_char string_5[] PROGMEM = "String 5";


// Then set up a table to refer to your strings.

PROGMEM const char *string_table[] = 	   // change "string_table" name to suit
{
  string_0,
  string_1,
  string_2,
  string_3,
  string_4,
  string_5
};
