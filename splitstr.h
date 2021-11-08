#ifndef _splitstr_h_
#define _splitstr_h_

#include <Arduino.h>

// Prototypes
String splitStrStart(String s, String delimiter);
String splitStrEnd(String s, String delimiter);
String splitStr(String s, String delimiter, int index);

bool strHasDelimiter(String s, String delimiter);

#endif // _splitstr_h_
