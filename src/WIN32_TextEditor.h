#ifndef TE_WIN32_WINDOWS32_H
#define TE_WIN32_WINDOWS32_H

#include "TextEditor.h"
#include "DataTypes.h"
#include "Math.h"
#include "String.h"

typedef struct {
    i32 width;
    i32 height;
    i32 pitch;
    i32 bytesPerPixel;
    BITMAPINFO info;
    void *memory;
} WIN32_WindowBuffer;

typedef struct {
    i32 width;
    i32 height;
} Dimensions;

Internal inline void WIN32_ProcessPendingEvents(MSG *event, u8 *noCharInputTable);
LRESULT CALLBACK WIN32_EventCallback(HWND windowHandle, UINT event, WPARAM wParam, LPARAM lParam);

Internal Dimensions WIN32_GetWindowDimensions();
Internal void WIN32_UpdateWindow(HDC deviceContext, i32 windowWidth, i32 windowHeight);
Internal void WIN32_ResizeBuffer(WIN32_WindowBuffer *windowBuffer, i32 width, i32 height);

Internal TextEditor LoadTextEditor(char *sourceFileNameDLL, char *tempFileNameDLL);
Internal void UnloadTextEditor();
Internal inline FILETIME WIN32_GetFileWriteTime(char *fileName);

Internal void CommandLineArgsToCStandarA(char *commandLineArgs, char **argv, int *argsCount);

MACRO_CloseProgram(WIN32_CloseProgram);

Internal inline void ToggleTopMostIfWindowIsActive(HWND windowHandle, b32 isBeingActivated);

MACRO_ToggleFullNormalWindowSize(WIN32_ToggleFullNormalWindowSize);
Internal inline void WIN32_MaximizeWindow();
Internal inline void WIN32_NormalWindow();

MACRO_ReadEntireFile(WIN32_ReadEntireFile);
MACRO_FreeFileMemory(WIN32_FreeFileMemory);
MACRO_WriteNewFile(WIN32_WriteNewFile);

MACRO_GetOSPageSize(WIN32_GetOSPageSize);
#endif //TE_WIN32_WINDOWS32_H
