#ifndef TE_STRING_H
#define TE_STRING_H

#define BACKWARDSTRING(address, length) ((char *)(address) - (length))

typedef struct {
    u32 length;
} LengthBaseString;

char *FindCharInString(char *string, char *end, char charToFind) {
    char *scanner = string;
    
    while (scanner <= end) {
	if (*scanner++ == charToFind) { return --scanner; }
    }

    return 0;
}

#define CopyStringBackwardToPointer(destination, source, endPointer) CopyStringBackward(destination, source, \
		                                                              (u32)((source) - (endPointer) + 1));
Internal void CopyStringBackward(char *destination, char *source, u32 length) {
    char *scanner = source;

    for (u32 charIndex = 0; charIndex < length; charIndex++) {
        *destination-- = *scanner--;
    }
}

Internal void CopyStringForward(char *destination, char *source, u32 size) {
    char *scanner = source;

    while (size--) { *destination++ = *scanner++; }
}

Internal inline char DigitToChar(u32 value) {
    return (char)(value + 48);
}

Internal void IntegerToString(char *string, i32 value, u32 digitsCount) {
    char *scanner = string + digitsCount - 1;

    while (digitsCount--) {
	u32 mod = (u32)value % 10;
	value /= 10;
	char character = DigitToChar(mod);
	*scanner-- = character;
    }
}

Internal void FloatToString(char *string, f32 value, u32 intergerDigitsCount, u32 decimalDigitsCount) {
    f32 temp = value;

    IntegerToString(string, (i32)value, intergerDigitsCount);

    char *scanner = string + intergerDigitsCount;
    *scanner++ = '.';

    temp = value;

    while (decimalDigitsCount--) {
	temp *= 10;
	u32 mod = (i32)temp % 10;
	char character = DigitToChar(mod);
	*scanner++ = character;
    }
}

Internal char *ProcessString(char *string, u32 length, f32 time, i32 repeat) {
    char *endString = string + length - 1;

    char *scanner = string;

    while ((scanner = FindCharInString(scanner, endString, '%')) != 0) {
        char *beginControlString = scanner;
        char next = *++scanner;

        switch (next) {
            case '.': {
                next = *++scanner;

                u32 decimalDigitsCount = (u32)(next - 48);
                u32 integerDigitsCount = GetIntegerDigits((i32)time) + 1;

                if ((next = *++scanner) != 'f') { ASSERT(!"nononono bad"); }

                //NOTE(JENH): +1 for the point.
                u32 stringToInsertLength = decimalDigitsCount + integerDigitsCount + 1;

                CopyStringBackward(endString + stringToInsertLength - 4, endString, stringToInsertLength);

                FloatToString(beginControlString, time, integerDigitsCount, decimalDigitsCount);

                scanner += stringToInsertLength - 4;
                endString += stringToInsertLength - 4;
            } break;

            case 'd': {
                u32 digitsCount = GetIntegerDigits((i32)repeat) + 1;

                if (digitsCount < 2) {
                    CopyStringForward(beginControlString, beginControlString + digitsCount, (u32)( endString - beginControlString ));
                } else {
                    CopyStringBackwardToPointer(endString + digitsCount - 2, endString, beginControlString);
                }

                IntegerToString(beginControlString, repeat, digitsCount);

                scanner += digitsCount - 2;
                endString += digitsCount - 2;
            } break;
        }
    }

    return endString;
}

//TODO(JENH): End variable shouldn't be neccesary.
Internal char *GetCharLocationInLineByHorSpatialValue(char *string, char *end, u32 *horSpatialValue) {
    char *scanner = string;
    u32 lineWidth = 0;

    while (*scanner != '\r' && *scanner != '\n' && scanner < end) {
	if (lineWidth >= *horSpatialValue) { break; }
	lineWidth += (*scanner++ == '\t') ? 4 : 1;
    }

    if (*scanner == '\r') { ++scanner; }

    *horSpatialValue = lineWidth;
    return scanner;
}

Internal u32 GetSpatialLineWidthBackward(char *string) {
    char *scanner = string;
    u32 result = 0;

    while (*scanner != '\n') {
	result += (*scanner-- == '\t') ? 4 : 1;
    }

    return result;
}

Internal v2_i32 GetCursorDisplacementForward(char *cursorInText, char *end, u32 tabToChars, u32 cursorPositionX) {
    v2_i32 result {0, 0};

    char *scanner = cursorInText;

    u32 tabCharCount = tabToChars - (cursorPositionX % tabToChars);

    while (scanner < end) {
	if (*scanner != '\r') {
	    if (*scanner == '\t') {
	        result.x += tabCharCount;
                tabCharCount = 1;
	    } else if (*scanner == '\n') {
	        ++result.y;
	        result.x = 0;
                tabCharCount = 1;
	    } else {
	        ++result.x;
	    }

	    if (!--tabCharCount) {
	        tabCharCount = tabToChars;
	    }
	}

        scanner++;
    }

    return result;
}

Internal char *GetCursorPositionTillCharForward(char *string, char *end, char charLookingFor, u32 tabToChars,
		                               u32 occurrenceToFind, u32 *cursorPositionX, u32 *cursorPositionY) {
    char *scanner = string;
    char *lastCharFind = 0;

    u32 occurrencesLeft = occurrenceToFind;

    u32 tabCharCount = tabToChars - ((*cursorPositionX % tabToChars) + 1);

    while (scanner < end) {
        if (*scanner == charLookingFor) { 
	    lastCharFind = scanner; 
            occurrencesLeft--;
	}

	if (!occurrencesLeft) { break; }

	if (*scanner != '\r') {
	    if (*scanner == '\t') {
	        (*cursorPositionX) += tabCharCount;
                tabCharCount = 1;
	    } else if (*scanner == '\n') {
	        (*cursorPositionY)++;
	        (*cursorPositionX) = 0;
                tabCharCount = 1;
	    } else {
	        (*cursorPositionX)++;
	    }

	    if (!--tabCharCount) {
	        tabCharCount = tabToChars;
	    }
	}

        scanner++;
    }

    return lastCharFind;
}

Internal void GetCursorPositionForward(char *string, char *end, u32 tabToChars, u32 *cursorPositionX,
		                       u32 *cursorPositionY) {
    char *scanner = string;

    u32 tabCharCount = tabToChars - (*cursorPositionX % tabToChars);

    while (scanner < end) {
	if (*scanner != '\r') {
	    if (*scanner == '\t') {
	        (*cursorPositionX) += tabCharCount;
                tabCharCount = 1;
	    } else if (*scanner == '\n') {
	        ++(*cursorPositionY);
	        (*cursorPositionX) = 0;
                tabCharCount = 1;
	    } else {
	        ++(*cursorPositionX);
	    }

	    if (!--tabCharCount) {
	        tabCharCount = tabToChars;
	    }
	}

        scanner++;
    }
}

char *FindSomeOtherCharInStringForward(char *string, char *end, char *charsToAvoid, u32 length, 
		                       u32 occurrenceToFind) {
    u32 occurrencesLeft = occurrenceToFind;

    char *scanner = string;
    char *lastCharFind = 0;

    while (occurrencesLeft && scanner <= end) {
        char *scannerCharsToAvoid = charsToAvoid;
	b32 isCharToAvoid = false;

	u32 charsLookingForIndex = length;

	while (charsLookingForIndex--) {
	    if (*scanner == *scannerCharsToAvoid++) { isCharToAvoid = true; }
	}

	if (!isCharToAvoid) { 
	    occurrencesLeft--; 
            lastCharFind = scanner;
	}

        *scanner++;
    }

    return lastCharFind;
}

char* FindSomeOtherCharInStringBackward(char *string, char *end, char *charsToAvoid, u32 occurrenceToFind) {
    u32 occurrencesLeft = occurrenceToFind;

    char *scanner = string;
    char *lastCharFind = 0;

    while (occurrencesLeft && scanner >= end) {
        char *scannerCharsToAvoid = charsToAvoid;
	b32 isCharToAvoid = false;
	while (*scannerCharsToAvoid != '\0') {
	    if (*scanner == *scannerCharsToAvoid++) { isCharToAvoid = true; }
	}

	if (!isCharToAvoid) { 
	    occurrencesLeft--; 
            lastCharFind = scanner;
	}

        *scanner--;
    }

    return lastCharFind;
}

char *FindSomeCharInStringForward(char *string, char *end, char *charsLookingFor, u32 length, 
		                  u32 occurrenceToFind) {
    u32 occurrencesLeft = occurrenceToFind;

    char *scanner = string;
    char *lastCharFind = 0;

    while (occurrencesLeft && scanner <= end) {
        char *scannerCharsLookingFor = charsLookingFor;

	u32 charsLookingForIndex = length;

	while (charsLookingForIndex--) {
	    if (*scanner == *scannerCharsLookingFor++) { 
	        occurrencesLeft--; 
                lastCharFind = scanner;
	    }
	}

        *scanner++;
    }

    return lastCharFind;
}

char *FindSomeCharInStringBackward(char *string, char *end, char *charsLookingFor, u32 occurrenceToFind) {
    u32 occurrencesLeft = occurrenceToFind;

    char *scanner = string;
    char *lastCharFind = 0;

    while (occurrencesLeft && scanner >= end) {
        char *scannerCharsLookingFor = charsLookingFor;
	while (*scannerCharsLookingFor != '\0') {
	    if (*scanner == *scannerCharsLookingFor++) { 
	        occurrencesLeft--; 
                lastCharFind = scanner;
	    }
	}

        *scanner--;
    }

    return lastCharFind;
}

char *FindCharInStringForward(char *string, char *end, char charToFind, u32 occurrenceToFind) {
    u32 occurrencesLeft = occurrenceToFind;

    char *scanner = string;
    char *lastCharFind = 0;

    while (occurrencesLeft && scanner <= end) {
    	if (*scanner == charToFind) { 
	    occurrencesLeft--; 
            lastCharFind = scanner;
	}

        *scanner++;
    }

    return lastCharFind;
}

char *FindCharInStringBackward(char *string, char *end, char charToFind, u32 occurrenceToFind) {
    u32 occurrencesLeft = occurrenceToFind;

    char *scanner = string;
    char *lastCharFind = 0;

    while (occurrencesLeft && scanner >= end) {
	if (*scanner == charToFind) { 
	    occurrencesLeft--; 
            lastCharFind = scanner;
	}

        *scanner--;
    }

    return lastCharFind;
}

#define ZeroMem(begin, size) CopyValueInMemRegion(begin, size, 0)
Internal void CopyValueInMemRegion(void *begin, u32 size, u8 value) {
    char *scanner = (char *)begin;

    while (size--) { *scanner++ = value; }
}

Internal i32 CompareStrings(char *string1, char *string2, u32 size) {
    while (size--) {
	if (*string1 != *string2) {
	    return (*string1 < *string2) ? 1 : -1;
	}

	string1++;
	string2++;
    }

    return 0;
}

#endif //TE_STRING_H
