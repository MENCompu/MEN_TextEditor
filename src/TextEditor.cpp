#include "TextEditor.h"

#include "Commands.h"
#include "SubWindowGridSystem.cpp"
#include <stdio.h>

//TODO(JENH): Revisit tab handeling.

//NOTE(JENH): May regret having all \r for \n.

Internal void MoveScreen(SubWindow *subWindow, v2_i32 cursorOffsetTopLeft, v2_i32 cursorOffsetBottomRight) {
    StringBuffer *stringBuffer = subWindow->displayedBuffer;

    if (cursorOffsetTopLeft.y > 0) {
        char *beginningOfBuffer = stringBuffer->buffer - 2;

        char *firstProcessedLine = subWindow->firstProcessedLine;
        subWindow->firstProcessedLine = FindCharInStringBackward(firstProcessedLine - 2 - 1, beginningOfBuffer,
                                                                    '\n', cursorOffsetTopLeft.y) + 1;
        subWindow->screenCharOffsetRelToText.y -= cursorOffsetTopLeft.y;
    } else if (cursorOffsetBottomRight.y > 0) {
        char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 1;

        char *firstProcessedLine = subWindow->firstProcessedLine;
        subWindow->firstProcessedLine = FindCharInStringForward(firstProcessedLine, endOfBuffer, '\n', 
                                                                   cursorOffsetBottomRight.y) + 1;
        subWindow->screenCharOffsetRelToText.y += cursorOffsetBottomRight.y;
    }

    if (cursorOffsetTopLeft.x > 0) {
        subWindow->screenCharOffsetRelToText.x -= cursorOffsetTopLeft.x;
    } else if (cursorOffsetBottomRight.x > 0) {
        subWindow->screenCharOffsetRelToText.x += cursorOffsetBottomRight.x;
    }
}

Internal inline void MoveScreenIfNecessary(SubWindow *subWindow) {
    StringBuffer *stringBuffer = subWindow->displayedBuffer;
    v2_u32 dimensionsInGlyphs = subWindow->dimensionsInGlyphs;

    v2_u32 cursorPosition = subWindow->cursor.position;
    v2_u32 screenCharOffset = subWindow->screenCharOffsetRelToText;

    v2_i32 cursorOffsetTopLeft = Toi32(screenCharOffset - cursorPosition);
    v2_i32 cursorOffsetBottomRight = Toi32(cursorPosition - (screenCharOffset + 
                                                             dimensionsInGlyphs - V2_u32(1, 1)));

    MoveScreen(subWindow, cursorOffsetTopLeft, cursorOffsetBottomRight);
}

Internal void ClearSubWindow(SubWindow *subWindow) {
    StringBuffer *stringBuffer = subWindow->displayedBuffer;

    ZeroMem(stringBuffer->buffer, stringBuffer->usedSize - 2);

    stringBuffer->usedSize = 2;

    subWindow->screenCharOffsetRelToText = {0, 0};

    subWindow->firstProcessedLine = stringBuffer->buffer;

    subWindow->cursor.position = {0, 0};
    subWindow->cursor.relPositionForResizing = {0, 0};
    subWindow->cursor.savedPosXForVertMov = 0;

    subWindow->cursor.positionInText = stringBuffer->buffer;

    if (*subWindow->cursor.positionInText == '\r') { ++subWindow->cursor.positionInText; }
}

Internal void InitializeSubWindow(SubWindow *subWindow, StringBuffer *displayedBuffer) {
    subWindow->displayedBuffer = displayedBuffer;
	
    subWindow->cursor.position = {0, 0};
    subWindow->cursor.savedPosXForVertMov = subWindow->cursor.position.x;

    subWindow->cursor.positionInText = displayedBuffer->buffer;
    
    if (*subWindow->cursor.positionInText == '\r') { ++subWindow->cursor.positionInText; }

    subWindow->cursor.color = GREY;

    subWindow->screenCharOffsetRelToText = {0, 0}; 

    subWindow->firstProcessedLine = displayedBuffer->buffer;
}

Internal void InitializeStringBuffer(StringBuffer *stringBuffer, char **bufferAddress, u32 maxSize) {
    char *buffer = *bufferAddress;

    *((char *)buffer++) = '\r';
    *((char *)buffer++) = '\n';

    stringBuffer->buffer = buffer;

    *((char *)buffer++) = '\r';
    *((char *)buffer  ) = '\n';

    stringBuffer->usedSize = 2;
    stringBuffer->maxSize  = maxSize;

    *bufferAddress += maxSize;
}

Internal void SwapSubWindowsScreentextCursorAndDisplayedBuffer(SubWindow *subWindow1, SubWindow *subWindow2) {
    SWAP(subWindow1->cursor, subWindow2->cursor, Cursor);
    SWAP(subWindow1->screenCharOffsetRelToText, subWindow2->screenCharOffsetRelToText, v2_u32);
    SWAP(subWindow1->firstProcessedLine, subWindow2->firstProcessedLine, char *);
    SWAP(subWindow1->displayedBuffer, subWindow2->displayedBuffer, StringBuffer *);
}

Internal StringBuffer *SearchStringBufferByPath(StringBuffers *stringBuffers, char *path, u32 pathSize) {
    StringBuffer *stringBuffer = stringBuffers->array;
    
    while (stringBuffer < stringBuffers->firstEmpty) {
        if (pathSize == stringBuffer->pathSize && CompareStrings(path, stringBuffer->path, pathSize) == 0) {
            return stringBuffer;
        }
        
        stringBuffer++;
    } 

    return 0;
}

//TODO(JENH): Convert relative paths to abs paths before comparing them.
Internal void OpenAFileInAStringBuffer(StringBuffer *stringBuffer, char *path, u32 pathSize, 
		                       char **firstAddresEmpty, u32 OSPageSize, u32 defaultStringBufferSize, 
				       T_ReadEntireFile *ReadEntireFile) {
    CopyStringForward(stringBuffer->path, path, pathSize);
    stringBuffer->pathSize = pathSize;
    
    InitializeStringBuffer(stringBuffer, firstAddresEmpty, defaultStringBufferSize);
    
    u32 fileSize = ReadEntireFile(stringBuffer->path, stringBuffer->buffer);
    
    stringBuffer->usedSize = fileSize + 2;

    u32 fileMemorySize = CeilF32ToU32(((f32)fileSize + 2.0f) / (f32)OSPageSize) * OSPageSize;
    stringBuffer->maxSize += fileMemorySize; 
    
    char *endOfBuffer = stringBuffer->buffer + fileSize;
    
    *endOfBuffer++ = '\r';
    *endOfBuffer++ = '\n';
    
    *firstAddresEmpty += fileMemorySize;
}

//NOTE(JENH): When inserting in a stringBuffer that it's contained in two or more subWindows, 
//            the other cursors should be updated correctly.

//NOTE(JENH): Try to improve this brute-force implementation. See gap buffer and rope data structures.
//            Also, see if moving the gap by steps in the gap buffer data structure works.
Internal inline void InsertCharacter(Cursor *cursor, StringBuffer *stringBuffer, char character) {
    char *endOfBuffer = (char *)stringBuffer->buffer + stringBuffer->usedSize - 1;
    
    if (character == '\b') {
        if (cursor->positionInText > stringBuffer->buffer) {
            u32 bytesDeleted;

	    char *positionInText = cursor->positionInText;
	    
	    if (*(positionInText - 1) == '\r') { --positionInText; }

            if (*(positionInText - 1) == '\n') {
                bytesDeleted = 2;

	        char *newLineToDelete = positionInText - 1;
	    
	        if (*(newLineToDelete - 1) == '\r') { --newLineToDelete; }

                cursor->position.x = GetSpatialLineWidthBackward(newLineToDelete - 1);
                cursor->position.y--;
            } else {
                bytesDeleted = 1;
                cursor->position.x -= (*(positionInText - 1) == '\t') ? 4 : 1;
            }
            
            DeleteRegion(stringBuffer, positionInText - bytesDeleted, positionInText);
            cursor->positionInText -= bytesDeleted;
        }
    } else {
	if (*(cursor->positionInText - 1) == '\r') { --cursor->positionInText; }

        if (character == '\r') {
            CopyStringBackwardToPointer(endOfBuffer + 2, endOfBuffer, cursor->positionInText);
            *cursor->positionInText++ = '\r';
            *cursor->positionInText++ = '\n';

            cursor->position.x = 0;
            cursor->position.y++;
            stringBuffer->usedSize += 2;
        } else {
            CopyStringBackwardToPointer(endOfBuffer + 1, endOfBuffer, cursor->positionInText);
            *cursor->positionInText++ = character;
            cursor->position.x += (character == '\t') ? 4 : 1;
            stringBuffer->usedSize++;
        }

	if (*cursor->positionInText == '\r') { ++cursor->positionInText; }
    }
}

Internal inline void InsertInputMode(State *state, u32 key) {
    InsertCharacter(&state->currentSubWindow->cursor, state->currentBuffer, (char)key);
    state->currentSubWindow->cursor.savedPosXForVertMov = state->currentSubWindow->cursor.position.x;
}

Internal inline void CommandLineInputMode(State *state, Memory *memory, u32 key) {
    switch (key) {
        case '\r': {
	    StringBuffer *newStringBuffer;

	    char *path = state->miniBuffer.buffer;
	    u32 pathSize = state->miniBuffer.usedSize - 2;

	    if (state->savedSubWindow->displayedBuffer->path) {
                newStringBuffer = SearchStringBufferByPath(&state->stringBuffers, path, pathSize);

		if (!newStringBuffer) {
		    ASSERT(state->stringBuffers.firstEmpty < state->stringBuffers.array + 
			   ARRAYLENGTH(state->stringBuffers.array) - 1);

		    newStringBuffer = state->stringBuffers.firstEmpty++;

                    OpenAFileInAStringBuffer(newStringBuffer, path, pathSize, &state->firstAddresEmpty, 
				             memory->GetOSPageSize(), state->defaultStringBufferSize, 
					     memory->ReadEntireFile);
		}
	    } else {
	        newStringBuffer = state->savedSubWindow->displayedBuffer; 
	    }
            
            ClearSubWindow(&state->miniBufferSW);

	    InitializeSubWindow(state->savedSubWindow, newStringBuffer);
            ChangeSubWindow(state, state->savedSubWindow);
                
            state->inputSystem.inputMode = NormalMode;
        } break;
        
        default: {
            if (!(key >> 8)) {
                InsertInputMode(state, key);
            }
        } break;
    }
}

Internal inline void NormalInputMode(State *state, u32 key) {
    switch (key) {
        case 'i': {
            state->inputSystem.inputMode = InsertMode;
		    
            Logging(Log_InsertMode, "- SWITCHING MODE: INSERT.", 0.0f);
        } break;
        
        case 'a': {
            MoveRight(state->currentBuffer, &state->currentSubWindow->cursor, 4, 1);
            state->currentSubWindow->cursor.savedPosXForVertMov = state->currentSubWindow->cursor.position.x;
            state->inputSystem.inputMode = InsertMode;
		    
            Logging(Log_InsertMode, "- SWITCHING MODE: INSERT.", 0.0f);
        } break;
        
        case 'v': {
            state->inputSystem.inputMode = VisualMode;
		    
            Logging(Log_VisualMode, "- SWITCHING MODE: VISUAL.", 0.0f);
        } break;
        
        case '|': {
            state->inputSystem.inputMode = TerminalMode;
        } break;
        
        case ':': {
            ChangeSubWindow(state, &state->miniBufferSW);
            state->inputSystem.inputMode = CommandLineMode;
		    
            Logging(Log_CommandLineMode, "- SWITCHING MODE: COMMAND LINE.", 0.0f);
        } break;
        
        case 'j': {
            MoveDown(state->currentBuffer, &state->currentSubWindow->cursor, 4, 1);
        } break;
        
        case 'k': {
            MoveUp(state->currentBuffer, &state->currentSubWindow->cursor, 4, 1);
        } break; 
        
        case 'h': {
            MoveLeft(state->currentBuffer, &state->currentSubWindow->cursor, 4, 1);
            state->currentSubWindow->cursor.savedPosXForVertMov = state->currentSubWindow->cursor.position.x;
        } break;
        
        case 'l': {
            MoveRight(state->currentBuffer, &state->currentSubWindow->cursor, 4, 1);
            state->currentSubWindow->cursor.savedPosXForVertMov = state->currentSubWindow->cursor.position.x;
        } break;
        
        case 'w': {
            MoveToNextWord(state->currentBuffer, &state->currentSubWindow->cursor, 4, 1);
            state->currentSubWindow->cursor.savedPosXForVertMov = state->currentSubWindow->cursor.position.x;
        } break;
        
        case 'b': {
            MoveToPrevWord(state->currentBuffer, &state->currentSubWindow->cursor, 4, 1);
            state->currentSubWindow->cursor.savedPosXForVertMov = state->currentSubWindow->cursor.position.x;
        } break;
        
        case '[': {
            MoveToPrevBlankLine(state->currentBuffer, &state->currentSubWindow->cursor, 1);
        } break;
        
        case ']': {
            MoveToNextBlankLine(state->currentBuffer, &state->currentSubWindow->cursor, 1);
        } break;
        
        case '$': {
            MoveToEndOfLine(state->currentBuffer, &state->currentSubWindow->cursor, 1);
            state->currentSubWindow->cursor.savedPosXForVertMov = 1000;
        } break;
        
        case 'o': {
            InsertNewLineForward(state->currentBuffer, &state->currentSubWindow->cursor, 1);
        } break;
        
        case 'O': {
            InsertNewLineBackward(state->currentBuffer, &state->currentSubWindow->cursor, 1);
        } break;
        
        case 'd': {
            DeleteLineBlock(state->currentBuffer, &state->currentSubWindow->cursor, 1);
        } break;
        
        case 'y': {
            state->lastCopiedString = YankLineBlock(state->currentBuffer, &state->currentSubWindow->cursor, 
                                                    state->lastCopiedString, 1);
        } break;
        
        case 'p': {
            PasteNextLine(state->currentBuffer, &state->currentSubWindow->cursor, state->lastCopiedString);
        } break;
    }
}

Internal inline b32 GlobalInput(State *state, Memory *memory, u32 key) {
    b32 wasProcessed = true;

    u32 controlKeysFlags = state->inputSystem.controlKeysFlags;

    switch (key) {
        case LeftArrow: {
            MoveLeft(state->currentBuffer, &state->currentSubWindow->cursor, 4, 1);
            state->currentSubWindow->cursor.savedPosXForVertMov = state->currentSubWindow->cursor.position.x;
        } break;
        
        case UpArrow: {
            MoveUp(state->currentBuffer, &state->currentSubWindow->cursor, 4, 1);
        } break; 
        
        case RightArrow: {
            MoveRight(state->currentBuffer, &state->currentSubWindow->cursor, 4, 1);
            state->currentSubWindow->cursor.savedPosXForVertMov = state->currentSubWindow->cursor.position.x;
        } break;
        
        case DownArrow: {
            MoveDown(state->currentBuffer, &state->currentSubWindow->cursor, 4, 1);
        } break;
        
#if 1
        case F1: {
            memory->WriteNewFile(state->currentBuffer->path, state->currentBuffer->buffer, 
                                 state->currentBuffer->usedSize - 2);
        } break;
#endif

        case F11: {
            SetSubWindowsForResizing(&state->subWindowGridSystem, &state->miniBufferSW);
            memory->ToggleFullNormalWindowSize();
        } break;
        
	default: {
            wasProcessed = false;
        } break;
    }

    if (!wasProcessed && AT_LEAST_ONE_FLAG_IS_SET(controlKeysFlags, LRControls)) {
        wasProcessed = true;

	switch (key) {
            case 'a': {
                SubWindow *newSubWindow = SplitSubWindow(&state->subWindowGridSystem, state->currentSubWindow,
                                                         state->characterInfo.glyphDimensions, AXIS_HOR);
            
                MoveScreenIfNecessary(state->currentSubWindow);
            
                StringBuffer *newStringBuffer = state->stringBuffers.firstEmpty++;
                InitializeStringBuffer(newStringBuffer, &state->firstAddresEmpty, 
				       state->defaultStringBufferSize);
                state->firstAddresEmpty += newStringBuffer->maxSize;

		InitializeSubWindow(newSubWindow, newStringBuffer);
                ChangeSubWindow(state, newSubWindow);
                state->savedSubWindow = state->currentSubWindow;
            } break;
        
            case 'w': {
                SubWindow *newSubWindow = SplitSubWindow(&state->subWindowGridSystem, state->currentSubWindow,
                                                         state->characterInfo.glyphDimensions, AXIS_VERT);
            
                MoveScreenIfNecessary(state->currentSubWindow);
            
                StringBuffer *newStringBuffer = state->stringBuffers.firstEmpty++;
                InitializeStringBuffer(newStringBuffer, &state->firstAddresEmpty, 
				       state->defaultStringBufferSize);
                state->firstAddresEmpty += newStringBuffer->maxSize;
            
		InitializeSubWindow(newSubWindow, newStringBuffer);
                ChangeSubWindow(state, newSubWindow);
                state->savedSubWindow = state->currentSubWindow;
            } break;
        
            case 'd': {
                SubWindow *newSubWindow = SplitSubWindow(&state->subWindowGridSystem, state->currentSubWindow,
                                                         state->characterInfo.glyphDimensions, AXIS_HOR);
            
                MoveScreenIfNecessary(state->currentSubWindow);
            
                StringBuffer *newStringBuffer = state->stringBuffers.firstEmpty++;
                InitializeStringBuffer(newStringBuffer, &state->firstAddresEmpty, 
				       state->defaultStringBufferSize);
                state->firstAddresEmpty += newStringBuffer->maxSize;

		InitializeSubWindow(newSubWindow, newStringBuffer);
                SwapSubWindowsScreentextCursorAndDisplayedBuffer(newSubWindow, state->currentSubWindow);
		state->currentSubWindow->cursor.color = GREEN;
		newSubWindow->cursor.color = GREY;

		state->currentBuffer = newStringBuffer;
                state->savedSubWindow = state->currentSubWindow;
            } break;

            case 's': {
                SubWindow *newSubWindow = SplitSubWindow(&state->subWindowGridSystem, state->currentSubWindow,
                                                         state->characterInfo.glyphDimensions, AXIS_VERT);
            
                MoveScreenIfNecessary(state->currentSubWindow);
            
                StringBuffer *newStringBuffer = state->stringBuffers.firstEmpty++;
                InitializeStringBuffer(newStringBuffer, &state->firstAddresEmpty, 
				       state->defaultStringBufferSize);
                state->firstAddresEmpty += newStringBuffer->maxSize;

		InitializeSubWindow(newSubWindow, newStringBuffer);
                SwapSubWindowsScreentextCursorAndDisplayedBuffer(newSubWindow, state->currentSubWindow);
		state->currentSubWindow->cursor.color = GREEN;
		newSubWindow->cursor.color = GREY;

		state->currentBuffer = newStringBuffer;
                state->savedSubWindow = state->currentSubWindow;
            } break;

            case 'j': {
                MoveToSubWindow(state, SW_LEFT);
            } break;
        
            case 'i': {
                MoveToSubWindow(state, SW_UP);
            } break;
        
            case 'l': {
                MoveToSubWindow(state, SW_RIGHT);
            } break;

            case 'k': {
                MoveToSubWindow(state, SW_DOWN);
            } break;
        
            case 'q': {
                DeleteSubWindow(state, &state->subWindowGridSystem, state->currentSubWindow);
            } break;

	    default: {
                wasProcessed = false;
            } break;
	}
    }
    
    if (!wasProcessed && AT_LEAST_ONE_FLAG_IS_SET(controlKeysFlags, LRAlts)) {
        wasProcessed = true;

	switch (key) {
	    case '\r': {
                SetSubWindowsForResizing(&state->subWindowGridSystem, &state->miniBufferSW);
                memory->ToggleFullNormalWindowSize();
	    } break;

	    case F4: {
                memory->CloseProgram();
	    } break;

	    default: {
                wasProcessed = false;
            } break;
	}
    }

    return wasProcessed;
}

//TODO(JENH): Measure if switch-case is better than using an array of function pointers.
//            If it is, use switch-case and turn all key-binded functions into inline functions.
MACRO_KeyProcessing(KeyProcessing) {
    State *state = (State *)memory->permanent;
    
    if (keyIsDown) {
        if (key == '\x1B') {
            ChangeSubWindow(state, state->savedSubWindow);
            state->inputSystem.inputMode = NormalMode;

            Logging(Log_NormalMode, "- SWITCHING MODE: NORMAL.", 0.0f);
            return;
        }
        
        b32 wasGlobalInput = GlobalInput(state, memory, key);

        if (!wasGlobalInput) {
            switch (state->inputSystem.inputMode) {
                case NormalMode: {
                    NormalInputMode(state, key);
                } break;
            
                case InsertMode: {
                    InsertInputMode(state, key);
                } break;
            
                case VisualMode: {
                } break;
            
                case TerminalMode: {
                } break;
            
                case CommandLineMode: {
                    CommandLineInputMode(state, memory, key);
                } break;
            }
	}
    }
    
        ASSERT(*state->currentSubWindow->cursor.positionInText != '\r');
    
    MoveScreenIfNecessary(state->currentSubWindow);

    f32 frameTime = ToMiliseconds(EndTimeMeasurement());
	
    char printBuffer[256];
    sprintf_s(printBuffer, sizeof(printBuffer), "frame time: %.4fms\n", frameTime);
    OutputDebugStringA(printBuffer);
}

Internal BMPImage LoadBMPImage(char *fileName, void *address, T_ReadEntireFile *ReadEntireFile) {
    BMPImage result = {0};

    u32 BMPImageSize = ReadEntireFile(fileName, address);

    BitmapHeader *bitmapHeader = (BitmapHeader *)address;

    result.width = bitmapHeader->width;
    result.height = bitmapHeader->height;
    u32 *pixels = (u32 *)((u8 *)address + bitmapHeader->bitmapOffset);
    result.pixels = pixels;

    return result;
}

void DrawBMPImage(WindowBuffer *windowBuffer, BMPImage *bmpImage, f32 topX, f32 topY) {
    i32 topXI = RoundF32ToI32(topX);
    i32 topYI = RoundF32ToI32(topY);
    i32 bottomXI = RoundF32ToI32(topX + bmpImage->width);
    i32 bottomYI = RoundF32ToI32(topY + bmpImage->height);
    
    if (topXI < 0) {
        topXI = 0;
    } if (topYI < 0) {
        topYI = 0;
    } if (bottomXI > windowBuffer->width) {
        bottomXI = windowBuffer->width;
    } if (bottomYI > windowBuffer->height) {
        bottomYI = windowBuffer->height;
    }

    u8 *destRow = (u8 *)windowBuffer->memory + topYI * windowBuffer->pitch + topXI * windowBuffer->bytesPerPixel;
    u32 *sourceRow = bmpImage->pixels + ((bmpImage->height - 1) * bmpImage->width);

    for (i32 y = topYI; y < bottomYI; y++) {
        u32 *destination = (u32 *)destRow;
        u32 *source = sourceRow;

        for (i32 x = topXI; x < bottomXI; x++) {
            u8 alphaChannel = (u8)(*source >> 24);

            if (alphaChannel) { *destination = *source; }

            *destination++;
            *source++;
        }

        sourceRow -= bmpImage->width;
        destRow += windowBuffer->pitch;
    }
}

void DrawBMPImageRectangle(WindowBuffer *windowBuffer, BMPImage *bmpImage, v2_f32 topBMP, v2_f32 bottomBMP, 
                           v2_f32 topWindowBuffer, v3_f32 color) {
    i32 topBMPX = RoundF32ToI32(topBMP.x);
    i32 topBMPY = RoundF32ToI32(bmpImage->height - topBMP.y);
    i32 bottomBMPX = RoundF32ToI32(bottomBMP.x);
    i32 bottomBMPY = RoundF32ToI32(bmpImage->height - bottomBMP.y);
    
    if (topBMPX < 0) {
        topBMPX = 0;
    } if (topBMPY > bmpImage->height) {
        topBMPY = bmpImage->height;
    } if (bottomBMPX > bmpImage->width) {
        bottomBMPX = bmpImage->width;
    } if (bottomBMPY < 0) {
        bottomBMPY = 0;
    }
    
    i32 topWindowBufferX = RoundF32ToI32(topWindowBuffer.x);
    i32 topWindowBufferY = RoundF32ToI32(topWindowBuffer.y);
    i32 bottomWindowBufferX = RoundF32ToI32(topWindowBuffer.x + (bottomBMPX - topBMPX));
    i32 bottomWindowBufferY = RoundF32ToI32(topWindowBuffer.y + (topBMPY - bottomBMPY));
    
    if (topWindowBufferX < 0) {
        topWindowBufferX = 0;
    } if (topWindowBufferY < 0) {
        topWindowBufferY = 0;
    } if (bottomWindowBufferX > windowBuffer->width) {
        bottomWindowBufferX = windowBuffer->width;
    } if (bottomWindowBufferY > windowBuffer->height) {
        bottomWindowBufferY = windowBuffer->height;
    }
    
    u32 colorU32 = (u32)(((u32)(color.red   * 255) << 16) |
                         ((u32)(color.green * 255) << 8)  |
                         ((u32)(color.blue  * 255) << 0)  );
    
    u8 *destRow = (u8 *)windowBuffer->memory + topWindowBufferY * windowBuffer->pitch +  
        topWindowBufferX * windowBuffer->bytesPerPixel;
    u32 *sourceRow = bmpImage->pixels + ((topBMPY - 1) * bmpImage->width) + topBMPX;
    
    for (i32 y = topWindowBufferY; y < bottomWindowBufferY; y++) {
        u32 *destination = (u32 *)destRow;
        u32 *source = sourceRow;
        
        for (i32 x = topWindowBufferX; x < bottomWindowBufferX; x++) {
            u8 alphaChannel = (u8)(*source >> 24);
            
            if (alphaChannel) { *destination = colorU32; }
            
            *destination++;
            *source++;
        }
        
        sourceRow -= bmpImage->width;
        destRow += windowBuffer->pitch;
    }
}

void DrawRectangle(WindowBuffer *windowBuffer, v2_f32 topLeft, v2_f32 bottomRight, v3_f32 color) {
    i32 topXI = RoundF32ToI32(topLeft.x);
    i32 topYI = RoundF32ToI32(topLeft.y);
    i32 bottomXI = RoundF32ToI32(bottomRight.x);
    i32 bottomYI = RoundF32ToI32(bottomRight.y);
    
    if (topXI < 0) {
        topXI = 0;
    } if (topYI < 0) {
        topYI = 0;
    } if (bottomXI > windowBuffer->width) {
        bottomXI = windowBuffer->width;
    } if (bottomYI > windowBuffer->height) {
        bottomYI = windowBuffer->height;
    }
    
    //BBGGRRAA
    u32 colorU32 = (u32)(((u32)(color.red   * 255) << 16) |
                         ((u32)(color.green * 255) << 8)  |
                         ((u32)(color.blue  * 255) << 0)  );
    
    u8 *row = (u8 *)windowBuffer->memory + topYI * windowBuffer->pitch + topXI * windowBuffer->bytesPerPixel;
    
    for (i32 y = topYI; y < bottomYI; y++) {
        u32 *pixel = (u32 *)row;
        
        for (i32 x = topXI; x < bottomXI; x++) {
            *pixel++ = colorU32;
        }
        
        row += windowBuffer->pitch;
    }
}

Internal inline v2_f32 MapCharToImage(char character, f32 charWidth, f32 charHeight) {
    v2_f32 result;
    
    result.x = charWidth  * ((character - 32) % 12);
    result.y = charHeight * ((character - 32) / 12);
    
    return result;
}

Internal void DrawSubWindowOutLine(WindowBuffer *windowBuffer, SubWindow *subWindow, v3_f32 color) {
    f32 lineWidth = 5.0f;
    
    v2_f32 topLeftPosition = subWindow->absTopLeftPosition;
    v2_f32 dimensions = subWindow->absDimensions;
    
    v2_f32 upOutLineTop = topLeftPosition;
    v2_f32 leftOutLineTop = topLeftPosition;
    v2_f32 downOutLineTop = topLeftPosition + V2_f32(0.0f, dimensions.height);
    v2_f32 rightOutLineTop = topLeftPosition + V2_f32(dimensions.width, 0.0f);
    
    v2_f32 upOutLineBottom = upOutLineTop + V2_f32(dimensions.width + lineWidth, lineWidth);
    v2_f32 leftOutLineBottom = leftOutLineTop + V2_f32(lineWidth, dimensions.height + lineWidth);
    v2_f32 downOutLineBottom = downOutLineTop + V2_f32(dimensions.width + lineWidth, lineWidth);
    v2_f32 rightOutLineBottom = rightOutLineTop + V2_f32(lineWidth, dimensions.height + lineWidth);
    
    DrawRectangle(windowBuffer, upOutLineTop, upOutLineBottom, color);
    DrawRectangle(windowBuffer, leftOutLineTop, leftOutLineBottom, color);
    DrawRectangle(windowBuffer, downOutLineTop, downOutLineBottom, color);
    DrawRectangle(windowBuffer, rightOutLineTop, rightOutLineBottom, color);
}

Internal void DEBUG_DrawAdjacentSubWindow(WindowBuffer *windowBuffer, AdjacentList *adjacentList, v3_f32 color, 
		                          BMPImage *glyphAtlas, v2_f32 glyphDimensions) {
    v3_f32 colorRest = V3_f32(1.0f, 1.0f, 1.0f) - color;
    
    char count = '1';
    
    u32 listLength = 0;
    
    for (AdjacentSubWindowNode *traverse = adjacentList->list; traverse;) {
        listLength++;
	if (!traverse->next) { ASSERT(traverse == adjacentList->tail); }
        traverse = traverse->next;
    }

    f32 brightnessRate = (listLength) ? (1.0f / (listLength + 1)) : 0.0f;
    
    for (AdjacentSubWindowNode *traverse = adjacentList->list; traverse; traverse = traverse->next) {
        SubWindow *adjacent = traverse->adjacent;
        v2_f32 topLeftPosition = adjacent->absTopLeftPosition;
        
        DrawSubWindowOutLine(windowBuffer, adjacent, color);
        v2_f32 TopRectangle = topLeftPosition + V2_f32(10.0f, 10.0f);
        
        DrawRectangle(windowBuffer, TopRectangle, TopRectangle + glyphDimensions, {1.0f, 1.0f, 1.0f});
        
        v2_f32 topGlyph = MapCharToImage(count, glyphDimensions.width, glyphDimensions.height);
        DrawBMPImageRectangle(windowBuffer, glyphAtlas, topGlyph, topGlyph + glyphDimensions, 
                              topLeftPosition + V2_f32(10.0f, 10.0f), color);
        
        color += colorRest * brightnessRate;
        count = (char)((u32)count + 1);
    }
}

Internal void DEBUG_SeeSubWindowsAdjacents(WindowBuffer *windowBuffer, SubWindow *subWindow, 
		                           BMPImage *glyphAtlas, v2_f32 glyphDimensions) {
    v3_f32 red = {1.0f, 0.0f, 0.0f};
    v3_f32 green = {0.0f, 1.0f, 0.0f};
    v3_f32 blue = {0.0f, 0.0f, 1.0f};
    v3_f32 purple = {0.5f, 0.0f, 0.5f};
    
    DEBUG_DrawAdjacentSubWindow(windowBuffer, &subWindow->adjacentLeft,  red,    glyphAtlas, glyphDimensions);
    DEBUG_DrawAdjacentSubWindow(windowBuffer, &subWindow->adjacentUp,    green,  glyphAtlas, glyphDimensions);
    DEBUG_DrawAdjacentSubWindow(windowBuffer, &subWindow->adjacentDown,  blue,   glyphAtlas, glyphDimensions);
    DEBUG_DrawAdjacentSubWindow(windowBuffer, &subWindow->adjacentRight, purple, glyphAtlas, glyphDimensions);
}

//NOTE(JENH): endianness
//registers: registers are in "big endian". That is, the least significant byte of the data
//is store in the least significant byte of the register.
//value = 0xFF773300  ,  register = 0xFF773300.
//
//memory: the memory is in "little endian". That is, the least significant byte of the data
//is store in the lowest memory address. In other words, the data is store flipt or left to
//right in memory.
//value = 0xFF773300  ,  memory = 0x003377FF
//
//bit shifts: bit shifts are done in registers, or in "big endian". So if you plan to use bit
//shifts with data "in the memory", you first need to flip it and then do the shifts.
//Remember that in registers the least significant byte is the right-most byte.
//
//Colors:
//colors in registers = 0xAARRGGBB
//colors in memory    = 0xBBGGRRAA

Internal void RenderSubWindow(WindowBuffer *windowBuffer, SubWindow *subWindow,
		              SubWindowGridSystem *subWindowGridSystem, CharacterInfo *characterInfo) {
    v2_f32 glyphDimensions = characterInfo->glyphDimensions;
    
    v2_f32 subWindowTopLeftPosition = subWindow->absTopLeftPosition + subWindowGridSystem->topLeftPadding;
    
    BMPImage *glyphAtlas = &characterInfo->glyphAtlas;
    
    StringBuffer *stringBuffer = subWindow->displayedBuffer;
    
    u32 processedGlyphsX = subWindow->screenCharOffsetRelToText.x;
    
    char *endOfBuffer = stringBuffer->buffer + stringBuffer->usedSize - 1;
    
    char *scanner = GetCharLocationInLineByHorSpatialValue(subWindow->firstProcessedLine, endOfBuffer,
                                                           &processedGlyphsX);
    
    processedGlyphsX = processedGlyphsX - subWindow->screenCharOffsetRelToText.x;
    v2_f32 topScreen = subWindowTopLeftPosition;
    
    topScreen.x += processedGlyphsX;
    
    u32 processedGlyphsY = 0;
    
    while (processedGlyphsY < (u32)subWindow->dimensionsInGlyphs.height && scanner <= endOfBuffer) {
        if (processedGlyphsX >= (u32)subWindow->dimensionsInGlyphs.width) {
            scanner = FindCharInStringForward(scanner, endOfBuffer, '\n', 1);
        }
        
        switch (*scanner) {
            case '\r': {
            } break;
            
            case '\n': {
                topScreen.y += glyphDimensions.height;
                
                u32 screenCharOffsetRelToTextX = subWindow->screenCharOffsetRelToText.x;
                
                scanner = GetCharLocationInLineByHorSpatialValue(scanner + 1, endOfBuffer,
                                                                 &screenCharOffsetRelToTextX) - 1;
                
                processedGlyphsX = screenCharOffsetRelToTextX - subWindow->screenCharOffsetRelToText.x;
                topScreen.x = subWindowTopLeftPosition.x + processedGlyphsX * glyphDimensions.width;
                processedGlyphsY++;
            } break;
            
            case '\t': {
                processedGlyphsX += 4;
                topScreen.x += 4.0f * glyphDimensions.width;
            } break;
            
            case ' ': {
                processedGlyphsX++;
                topScreen.x += glyphDimensions.width;
            } break;
            
            default: {
                v2_f32 topGlyph = MapCharToImage(*scanner, glyphDimensions.width, glyphDimensions.height);
                DrawBMPImageRectangle(windowBuffer, glyphAtlas, topGlyph, topGlyph + glyphDimensions, 
                                      topScreen, {0.0f, 0.0f, 0.0f});

                topScreen.x += glyphDimensions.width;
                processedGlyphsX++;
            }
        }

        *scanner++;
    }

    v3_f32 black = {0.0f, 0.0f, 0.0f};

    DrawSubWindowOutLine(windowBuffer, subWindow, black);

    v2_f32 topLeft = subWindowTopLeftPosition + HadamardProd(Tof32(subWindow->cursor.position - 
                                                                   subWindow->screenCharOffsetRelToText), 
		                                                   glyphDimensions);
    
    DrawRectangle(windowBuffer, topLeft, topLeft + glyphDimensions, subWindow->cursor.color);
}

void Render(Memory *memory, WindowBuffer *windowBuffer) {
    State *state = (State *)memory->permanent;
    
    DrawRectangle(windowBuffer, {0, 0}, {(f32)windowBuffer->width, 
		  (f32)windowBuffer->height}, {1.0f, 1.0f, 1.0f});

    RenderSubWindow(windowBuffer, &state->miniBufferSW, &state->subWindowGridSystem, &state->characterInfo);
    
    SubWindow *SubWindows = state->subWindowGridSystem.subWindows;
    SubWindow *firstSubWindowEmpty = state->subWindowGridSystem.firstSubWindowEmpty;
    
    for (SubWindow *subWindow = SubWindows; subWindow < firstSubWindowEmpty; subWindow++) {
	if (subWindow->isActive) {
            RenderSubWindow(windowBuffer, subWindow, &state->subWindowGridSystem, &state->characterInfo);
	}
    }
    
    SubWindow *currentSubWindow = state->currentSubWindow;
    
    DEBUG_SeeSubWindowsAdjacents(windowBuffer, currentSubWindow, &state->characterInfo.glyphAtlas, 
		                 state->characterInfo.glyphDimensions);
}

Internal inline void InitializeMiniBuffer(State *state, SubWindow *miniBufferSubWindow) {
    InitializeStringBuffer(&state->miniBuffer, &state->firstAddresEmpty, state->defaultStringBufferSize);

    InitializeSubWindow(&state->miniBufferSW, &state->miniBuffer);

    state->firstAddresEmpty += state->miniBuffer.maxSize;
    
    v2_f32 glyphDimensions = state->characterInfo.glyphDimensions;
    v2_f32 windowDimensions = state->subWindowGridSystem.windowDimensions;
    v2_f32 topLeftPadding = state->subWindowGridSystem.topLeftPadding;
    v2_f32 bottomRightPadding = state->subWindowGridSystem.bottomRightPadding;

    miniBufferSubWindow->isActive = true;
    miniBufferSubWindow->dimensionsInGlyphs = {(u32)windowDimensions.width / (u32)glyphDimensions.width, 1};
    miniBufferSubWindow->absDimensions = {windowDimensions.width, miniBufferSubWindow->dimensionsInGlyphs.height * 1.28f * glyphDimensions.height};

    miniBufferSubWindow->absTopLeftPosition = {0.0f, windowDimensions.height - miniBufferSubWindow->absDimensions.height};
    miniBufferSubWindow->displayedBuffer = &state->miniBuffer;

    state->currentSubWindow = miniBufferSubWindow;
}

#define Array_Count(array) ( sizeof(array) / sizeof((array)[0]) )

MACRO_InitializeMemory(InitializeMemory) {
    InitProfiling();

    BeginTimeMeasurement();

    State *state = (State *)memory->permanent;

    state->inputSystem.inputMode = NormalMode;

    state->characterInfo.glyphDimensions = {10.0f, 19.0f};

    char* dummy = 0;
    char fontPath[MAX_PATH] = { 0 };
    u32 hola = GetFullPathNameA("..\\assets\\Fonts\\Hermit\\hermit_bold.bmp", Array_Count(fontPath), fontPath, &dummy);

    state->characterInfo.glyphAtlas = LoadBMPImage(fontPath, (char *)memory->permanent + MEGABYTES(5), memory->ReadEntireFile);

    SubWindowGridSystem *subWindowGridSystem = &state->subWindowGridSystem;

    subWindowGridSystem->firstSubWindowEmpty = subWindowGridSystem->subWindows;

    subWindowGridSystem->minTopLeftPadding = {5.0f, 5.0f};
    subWindowGridSystem->minBottomRightPadding  = {5.0f, 5.0f};
    subWindowGridSystem->topLeftPadding = subWindowGridSystem->minTopLeftPadding;
    subWindowGridSystem->bottomRightPadding = subWindowGridSystem->minBottomRightPadding;

    subWindowGridSystem->windowDimensions = windowDimensions - subWindowGridSystem->topLeftPadding;

    subWindowGridSystem->adjacentNodePool.firstEmpty = subWindowGridSystem->adjacentNodePool.nodes;

    state->stringBuffers.firstEmpty = state->stringBuffers.array;
    state->firstAddresEmpty = (char *)memory->temporary + MEGABYTES(32);

    state->defaultStringBufferSize = KILOBYTES(64);

    SubWindow *miniBufferSubWindow = &state->miniBufferSW;

    InitializeMiniBuffer(state, miniBufferSubWindow);

    logSystem.logBuffer = state->stringBuffers.firstEmpty++;

    StringBuffer *logBuffer = logSystem.logBuffer;

    InitializeStringBuffer(logBuffer, &state->firstAddresEmpty, state->defaultStringBufferSize);

    char logBufferName[] = "{LOGGING}";

    CopyStringForward(logBuffer->path, logBufferName, ARRAYLENGTH(logBufferName) - 1);
    logBuffer->pathSize = ARRAYLENGTH(logBufferName) - 1;

    u32 OSPageSize = memory->GetOSPageSize();

    char **filePath = filePaths;

    char *garbage = (char *)MAX_VALUE_PTR;

    if (pathsCount) {
        for (i32 pathIndex = 0; pathIndex < pathsCount; ++pathIndex) {
	    u32 filePathSize = (u32)(FindCharInStringForward(*filePath, garbage, '\0', 1) - *filePath);

	    state->currentBuffer = state->stringBuffers.firstEmpty++;

            OpenAFileInAStringBuffer(state->currentBuffer, *filePath, filePathSize, &state->firstAddresEmpty, 
				     OSPageSize, state->defaultStringBufferSize, memory->ReadEntireFile);

	    ++filePath;
        }
    } else {
        state->currentBuffer = state->stringBuffers.firstEmpty++;
        InitializeStringBuffer(state->currentBuffer, &state->firstAddresEmpty, state->defaultStringBufferSize);
        state->firstAddresEmpty += state->miniBuffer.maxSize;
    }

    ZeroMem(memory->temporary, sizeof(char *) * pathsCount);

    SubWindow *initialSubWindow = state->subWindowGridSystem.firstSubWindowEmpty++;

    initialSubWindow->isActive = true;

    InitializeSubWindow(initialSubWindow, state->currentBuffer);

    initialSubWindow->absTopLeftPosition = {0.0f, 0.0f};
    initialSubWindow->absDimensions = subWindowGridSystem->windowDimensions;
    initialSubWindow->absDimensions.height -= miniBufferSubWindow->absDimensions.height;

    initialSubWindow->dimensionsInGlyphs = Tou32(HadamardDiv(initialSubWindow->absDimensions, 
							     state->characterInfo.glyphDimensions));

    state->savedSubWindow = initialSubWindow;
    ChangeSubWindow(state, initialSubWindow);

    state->copyBuffer = (char *)memory->temporary;
    state->lastCopiedString = (LengthBaseString *)state->copyBuffer;

    memory->isInitialized = true;

    f32 memoryInitializationTime = ToMiliseconds(EndTimeMeasurement());

    Logging(Log_MemoryInitialization, "memory initialization (%.4fms)\r\n", memoryInitializationTime);
}
