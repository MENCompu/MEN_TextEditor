#include "String.h"

Internal u32 CountCharacterOcurrencesInLine(char *string, u32 length, char charToCount) {
    u32 result = 0;
    char *scanner = string;

    for (u32 scannerIndex = 0; scannerIndex < length && *scanner != '\n'; scannerIndex++) {
	if (*scanner++ == charToCount) { result++; }
    }

    return result;
}

char* FindCharInStringForward(char *string, char charToFind, u32 occurrencesToIgnore) {
    u32 occurrencesLeft = occurrencesToIgnore;

    char *scanner = string;
    char *lastCharFind = 0;

    while (occurrencesLeft && *scanner != '\0') {
	if (*scanner == charToFind) { 
	    occurrencesLeft--; 
            lastCharFind = scanner;
	}

        *scanner++;
    }

    return lastCharFind;
}

Internal b32 FindCharInStringBackward(char *string, u32 tillBeginning, char charToFind, u32 *index) {
    char *scanner = string;

    for (u32 scannerIndex = 0; scannerIndex <= tillBeginning; scannerIndex++) {
	if (*scanner-- == charToFind) {
	    *index = scannerIndex;
	    return true;
	}
    }

    return false;
}
