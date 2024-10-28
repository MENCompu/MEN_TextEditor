#ifndef TE_COMMANDS_H
#define TE_COMMANDS_H

#include "TextEditor.h"
#include "DataTypes.h"
#include "String.h"

//TODO(JENH): See if a new line table is better for line processing. Like moving the cursor, 
//            line based commands, etc.
//NOTE(JENH): The new line table could be a gap buffer for a more performant modification.
//NOTE(JENH): Too much space?.

Internal inline void MoveDown(StringBuffer *stringBuffer, Cursor *cursor, u32 tabToChars, u32 count) {
    BeginTimeMeasurement();

    char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 3;

    char *positionInText = cursor->positionInText;

    char *nextLine = FindCharInStringForward(positionInText, endOfBuffer, '\n', 1);
    if (nextLine) {
        ++nextLine;

        cursor->position.x = cursor->savedPosXForVertMov;

        cursor->positionInText = GetCharLocationInLineByHorSpatialValue(nextLine, endOfBuffer,
			                                                &cursor->position.x);

        cursor->position.y++;
    }

    f32 moveDownTime = ToMiliseconds(EndTimeMeasurement());

    Logging(Log_MoveCursorVertByChar, "Move cursor vertically (%.4fms)", moveDownTime);
}

Internal inline void MoveUp(StringBuffer *stringBuffer, Cursor *cursor, u32 tabToChars, u32 count) {
    BeginTimeMeasurement();

    if (cursor->position.y - 1 < cursor->position.y) {
	char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 1;
	char *beginningOfBuffer = stringBuffer->buffer - 2;

	char *positionInText = cursor->positionInText;
	if (*positionInText == '\n') { positionInText -= 2; }

	char *prevLine = FindCharInStringBackward(positionInText, beginningOfBuffer, '\n', 2); 
	if (prevLine) {
	    prevLine++;

	    cursor->position.x = cursor->savedPosXForVertMov;

	    cursor->positionInText = GetCharLocationInLineByHorSpatialValue(prevLine, endOfBuffer, 
			                                                    &cursor->position.x);

	    cursor->position.y--;
	}
    }

    f32 moveUpTime = ToMiliseconds(EndTimeMeasurement());

    Logging(Log_MoveCursorVertByChar, "Move cursor vertically (%.4fms)", moveUpTime);
}

Internal inline void MoveLeft(StringBuffer *stringBuffer, Cursor *cursor, u32 tabToChars, u32 count) {
    char *positionInTextBegining = cursor->positionInText;

    if (*(positionInTextBegining - 1) == '\r') { --positionInTextBegining; }

    if (positionInTextBegining > stringBuffer->buffer) {
        cursor->positionInText--;

	if (*cursor->positionInText == '\r') { --cursor->positionInText; }

        switch (*cursor->positionInText) {
	    case '\t': {
	        cursor->position.x -= 4; 
	    } break;

	    case '\n': {
	        char *positionInText = cursor->positionInText - 1;

		if (*positionInText == '\r') { --positionInText; }

	        cursor->position.x = GetSpatialLineWidthBackward(positionInText);
	        cursor->position.y--;
	    } break;

	    default: {
	        cursor->position.x--;
	    } break;
        }
    }
}

Internal inline void MoveRight(StringBuffer *stringBuffer, Cursor *cursor, u32 tabToChars, u32 count) {
    char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 2; 
    if (cursor->positionInText < endOfBuffer) {
        switch (*cursor->positionInText) {
	    case '\n': {
	        cursor->position.x = 0;
	        cursor->position.y++;
	    } break;

	    case '\t': {
	        cursor->position.x += 4;
	    } break;

	    default: {
	        cursor->position.x++; 
	    } break;
        }

        cursor->positionInText++;

	if (*cursor->positionInText == '\r') { cursor->positionInText++; }
    }
}

//TODO(JENH): Change the FindInString funcitions in terms of numerical or alphabetic character.
Internal inline void MoveToNextWord(StringBuffer *stringBuffer, Cursor *cursor, u32 tabToChars, u32 count) {
    char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 1;

    char charsLookingFor1[] = " \t\n";

    char *newTextPosition = FindSomeCharInStringForward(cursor->positionInText, endOfBuffer, charsLookingFor1, 
		                                        ARRAYLENGTH(charsLookingFor1), count) + 1;
    
    if (*newTextPosition == '\r') { ++newTextPosition; }

    char charsLookingFor2[] = " \t";

    newTextPosition = FindSomeOtherCharInStringForward(newTextPosition, endOfBuffer, charsLookingFor2, 
		    				       ARRAYLENGTH(charsLookingFor2), count);

    GetCursorPositionForward(cursor->positionInText, newTextPosition, tabToChars, 
		             &cursor->position.x, &cursor->position.y);
    cursor->positionInText = newTextPosition;
}

Internal inline void MoveToPrevWord(StringBuffer *stringBuffer, Cursor *cursor, u32 tabToChars, u32 count) {
    char *beginningOfBuffer = stringBuffer->buffer;

    char *positionInText = cursor->positionInText - 1;

    if (*positionInText == '\r') { --positionInText; }

    char *newTextPosition = FindSomeOtherCharInStringBackward(positionInText, beginningOfBuffer - 2, 
			                                      " \t\r", count);

    newTextPosition = FindSomeCharInStringBackward(newTextPosition - 1, beginningOfBuffer, " \t\n", count);

    if (newTextPosition) {
        ++newTextPosition;
	
	if (*newTextPosition == '\r') { ++newTextPosition; }

        char *startOfLine = FindCharInStringBackward(newTextPosition, beginningOfBuffer - 2, '\n', 1) + 1;

        v2_i32 cursorDisplacement = GetCursorDisplacementForward(startOfLine, newTextPosition, tabToChars, 0);

        cursor->position.x = (u32)cursorDisplacement.x;

        cursorDisplacement = GetCursorDisplacementForward(newTextPosition, cursor->positionInText, tabToChars, 0);

        cursor->position.y -= (u32)cursorDisplacement.y;

        cursor->positionInText = newTextPosition;
    } else {
	cursor->positionInText = beginningOfBuffer;
        cursor->position = {0, 0};
    }
}

Internal inline void MoveToPrevBlankLine(StringBuffer *stringBuffer, Cursor *cursor, u32 count) {
    char *beginningOfBuffer = stringBuffer->buffer;
    char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 1;
    char *scanner = FindCharInStringForward(cursor->positionInText, endOfBuffer, '\n', 1);

    u32 newPositionY = cursor->position.y;
    cursor->position = {0, 0};

    cursor->positionInText = beginningOfBuffer;
	    
    b32 passFirstNonBlankLine = false;

    while (scanner >= beginningOfBuffer) {
        --scanner;

	if (*scanner == '\r') { --scanner; }

	char *prevNewLine = FindCharInStringBackward(scanner, beginningOfBuffer - 2, '\n', 1);

	char *beginingPrevLine = prevNewLine + 1;

	if (*beginingPrevLine == '\r') { ++beginingPrevLine; }

	i32 truncatedPointer = TruncateI64ToI32((i64)FindSomeOtherCharInStringBackward(scanner, beginingPrevLine, 
				                                                       " \t", 1));

	b32 isNonBlankLine = (b32)truncatedPointer;

	if (isNonBlankLine) { passFirstNonBlankLine = true; }

        if (!isNonBlankLine && passFirstNonBlankLine) {
            cursor->position.y = newPositionY;
	    cursor->positionInText = beginingPrevLine;
	    break;
	}

        --newPositionY;
	scanner = prevNewLine;
    }
}

Internal inline void MoveToNextBlankLine(StringBuffer *stringBuffer, Cursor *cursor, u32 count) {
    char *beginningOfBuffer = stringBuffer->buffer;
    char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 1;
    char *scanner = FindCharInStringBackward(cursor->positionInText - 1, beginningOfBuffer - 2, '\n', 1);

    u32 newPositionY = cursor->position.y;

    cursor->positionInText = endOfBuffer;
    cursor->position = {0, 0};
	    
    b32 passFirstNonBlankLine = false;

    while (scanner <= endOfBuffer) {
	++scanner;

	if (*scanner == '\r') { ++scanner; }

	char *nextNewLine = FindCharInStringForward(scanner, endOfBuffer, '\n', 1);

	char charsLookingFor[] = " \t";

	char *endLine = nextNewLine - 1;
	
	if (*endLine == '\r') { --endLine; }

	i32 truncatedPointer = TruncateI64ToI32((i64)FindSomeOtherCharInStringForward(scanner, endLine, 
				                            charsLookingFor, ARRAYLENGTH(charsLookingFor), 1));

	b32 isNonBlankLine = (b32)truncatedPointer;

	if (isNonBlankLine) { passFirstNonBlankLine = true; }

        if (!isNonBlankLine && passFirstNonBlankLine) {
	    cursor->positionInText = scanner;
	    cursor->position.y = newPositionY;
	    break;
	}

        newPositionY++;
	scanner = nextNewLine;
    }
}

Internal inline void MoveToEndOfLine(StringBuffer *stringBuffer, Cursor *cursor, u32 count) {
    char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 1;
    if (*cursor->positionInText == '\n') { cursor->positionInText++; }
    cursor->positionInText = GetCursorPositionTillCharForward(cursor->positionInText, endOfBuffer, '\n', 4, 
		                                              count, &cursor->position.x, &cursor->position.y);
}

Internal inline void InsertNewLineForward(StringBuffer *stringBuffer, Cursor *cursor, u32 count) {
    char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 1;
    char *endLine = FindCharInStringForward(cursor->positionInText, endOfBuffer, '\n', 1) + 1;

    CopyStringBackwardToPointer(endOfBuffer + (2 * count), endOfBuffer, endLine);
    for (u32 counter = 0; counter < count; counter++) {
        *endLine++ = '\r';
        *endLine++ = '\n';
    }

    cursor->positionInText = --endLine;

    stringBuffer->usedSize += 2 * count;

    cursor->position.x = 0;
    cursor->position.y += count;
}

Internal inline void InsertNewLineBackward(StringBuffer *stringBuffer, Cursor *cursor, u32 count) {
    char *beginningOfBuffer = stringBuffer->buffer;
    char *endPrevLine = FindCharInStringBackward(cursor->positionInText, beginningOfBuffer - 2, '\n', 1) + 1;

    cursor->positionInText = endPrevLine - 1;

    char *endOfBuffer = (char *)stringBuffer->buffer + stringBuffer->usedSize - 1;

    CopyStringBackwardToPointer(endOfBuffer + (2 * count), endOfBuffer, endPrevLine);
    for (u32 counter = 0; counter < count; counter++) {
        *endPrevLine++ = '\r';
        *endPrevLine++ = '\n';
    }

    stringBuffer->usedSize += 2 * count;

    cursor->position.x = 0;
}

Internal inline void DeleteRegion(StringBuffer *stringBuffer, char *beginning, char *end) {
    char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 1;
    CopyStringForward(beginning, end, (u32)(endOfBuffer - end + 1));
    stringBuffer->usedSize -= (u32)(end - beginning);
}

Internal inline void DeleteLineBlock(StringBuffer *stringBuffer, Cursor *cursor, u32 count) {
    char *beginningOfBuffer = stringBuffer->buffer;
    char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 1;
    char *beginningLineBlock = FindCharInStringBackward(cursor->positionInText - 1, beginningOfBuffer - 2, 
		                                        '\n', 1) + 1;
    char *endLineBlock = FindCharInStringForward(cursor->positionInText, endOfBuffer, '\n', count) + 1;

    if (*(endLineBlock + 1) != '\0') {
        DeleteRegion(stringBuffer, beginningLineBlock, endLineBlock);

	cursor->position.x = cursor->savedPosXForVertMov;
	cursor->positionInText = GetCharLocationInLineByHorSpatialValue(beginningLineBlock, endOfBuffer,
			                                                &cursor->position.x);
    }
}

Internal inline LengthBaseString *Yank(LengthBaseString *lastCopiedString, char *beginning, char *end) {
    char *newCopiedStringAddress = (char *)lastCopiedString + sizeof(LengthBaseString);
    u32 newCopiedStringLength = (u32)(end - beginning + 1);
    LengthBaseString *newCopiedString = (LengthBaseString *)(newCopiedStringAddress + newCopiedStringLength);
    newCopiedString->length = newCopiedStringLength;

    CopyStringForward(newCopiedStringAddress, beginning, newCopiedString->length);
    
    return newCopiedString;
}

Internal inline LengthBaseString *YankLineBlock(StringBuffer *stringBuffer, Cursor *cursor, 
		                                LengthBaseString *lastCopiedString, u32 count) {
    LengthBaseString *newCopiedString = 0;

    char *beginningOfBuffer = stringBuffer->buffer;
    char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 1;
    char *beginningLineBlock = FindCharInStringBackward(cursor->positionInText - 1, beginningOfBuffer - 2, 
		                                        '\n', 1) + 1;
    char *endLineBlock = FindCharInStringForward(cursor->positionInText, endOfBuffer, '\n', count);

    if (*(endLineBlock + 1) != '\0') {
        newCopiedString = Yank(lastCopiedString, beginningLineBlock, endLineBlock);
    }

    return newCopiedString;
}

Internal void Paste(StringBuffer *stringBuffer, char *addressToPaste, char *string, u32 length) {
    char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 1;
    CopyStringBackwardToPointer(endOfBuffer + length, endOfBuffer, addressToPaste);

    CopyStringForward(addressToPaste, string, length);

    stringBuffer->usedSize += length;
}

Internal inline void PasteNextLine(StringBuffer *stringBuffer, Cursor *cursor, 
		                   LengthBaseString *lastCopiedString) {
    char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 1;
    char *beginningNextLine = FindCharInStringForward(cursor->positionInText, endOfBuffer, '\n', 1) + 1;
    
    Paste(stringBuffer, beginningNextLine, BACKWARDSTRING(lastCopiedString, lastCopiedString->length), 
	  lastCopiedString->length);

    cursor->positionInText = beginningNextLine;
    cursor->position.x = 0;
    cursor->savedPosXForVertMov = 0;
    cursor->position.y++;
}

#endif //TE_COMMANDS_H
