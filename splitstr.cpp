#include "splitstr.h"

String splitStrStart(String s, String delimiter) {
	int i = s.indexOf(delimiter);
	if (i == -1) return s;
	return s.substring(0, i);
}

String splitStrEnd(String s, String delimiter) {
	int i = s.indexOf(delimiter);
	if (i == -1) return "";
	return s.substring(i + delimiter.length());
}

String splitStr(String s, String delimiter, int index) {
	return s;
}

bool strHasDelimiter(String s, String delimiter) {
  if (s.indexOf(delimiter) == -1) return false;
  return true;
}
