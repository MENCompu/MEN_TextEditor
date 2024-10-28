#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#define ASSERT(expression) if (!(expression)) { *(int *)0 = 0; }
#define ARRAYLENGTH(array) (sizeof(array) / sizeof((array)[0]))

#include <windows.h>
#include "DataTypes.h"
#include "Vectors.h"

#include "Math.h"
#include "String.h"

typedef struct StringBuffer {
    u32 pathSize;
    char path[MAX_PATH];

    u32 usedSize;
    u32 maxSize;

    char *buffer;
} StringBuffer;

#include "Profiling.h"
#include "Logging.h"

#include "Input.h"

#define SWAP(var1, var2, type) \
    do {		               \
        type temp = (var1);    \
		(var1) = (var2);       \
		(var2) = temp;         \
    } while (0);

#define FLAGS_ARE_SET(bitfield, flags) (((bitfield) & (flags)) == (flags))
#define AT_LEAST_ONE_FLAG_IS_SET(bitfield, flags) (bitfield) & (flags)
#define ADD_FLAGS(bitfield, flags) (bitfield) |= (flags)
#define DEL_FLAGS(bitfield, flags) (bitfield) &= ~flags
#define ADD_FLAG_BY_BIT(bitfield, bit) (bitfield) |= (1 << (bit))

typedef struct {
    i32 width;
    i32 height;
    i32 pitch;
    i32 bytesPerPixel;
    void *memory;
} WindowBuffer;

#define MACRO_ToggleFullNormalWindowSize(name) void name()
typedef MACRO_ToggleFullNormalWindowSize(T_ToggleFullNormalWindowSize);
MACRO_ToggleFullNormalWindowSize(STUB_ToggleFullNormalWindowSize) { }
MACRO_ToggleFullNormalWindowSize(ToggleFullNormalWindowSize);

#define MACRO_CloseProgram(name) void name()
typedef MACRO_CloseProgram(T_CloseProgram);
MACRO_CloseProgram(STUB_CloseProgram) { }
MACRO_CloseProgram(CloseProgram);

#define MACRO_ReadEntireFile(name) u32 name(char *fileName, void *address)
typedef MACRO_ReadEntireFile(T_ReadEntireFile);

#define MACRO_WriteNewFile(name) b32 name(char *newFilePath, void *fileMemory, u32 fileSize)
typedef MACRO_WriteNewFile(T_WriteNewFile);

#define MACRO_FreeFileMemory(name) void name(void *memory)
typedef MACRO_FreeFileMemory(T_FreeFileMemory);

#define MACRO_GetOSPageSize(name) u32 name()
typedef MACRO_GetOSPageSize(T_GetOSPageSize);

typedef struct {
    b32 isInitialized;

    u64 permanentSize;
    void *permanent;
    u64 temporarySize;
    void *temporary;

    T_CloseProgram *CloseProgram;
    T_ToggleFullNormalWindowSize *ToggleFullNormalWindowSize;

    T_ReadEntireFile *ReadEntireFile;
    T_WriteNewFile   *WriteNewFile;
    T_FreeFileMemory *FreeFileMemory;
    T_GetOSPageSize  *GetOSPageSize;
} Memory;

#include "SubWindowGridSystem.h"

#define MACRO_KeyProcessing(name) void name(Memory *memory, u32 key, b32 keyIsDown)
typedef MACRO_KeyProcessing(T_KeyProcessing);
MACRO_KeyProcessing(STUB_KeyProcessing) { }

#define MACRO_Render(name) void name(Memory *memory, WindowBuffer *windowBuffer)
typedef MACRO_Render(T_Render);
MACRO_Render(STUB_Render) { }

#define MACRO_InitializeMemory(name) void name(Memory *memory, v2_f32 windowDimensions, \
		                               char **filePaths, i32 pathsCount)
typedef MACRO_InitializeMemory(T_InitializeMemory);
MACRO_InitializeMemory(STUB_InitializeMemory) { memory->isInitialized = false; }

typedef struct {
    b32 isValid;

    HMODULE DLLHandle;
    char *DLLPath;

    FILETIME lastTimeWritten;

    T_KeyProcessing *KeyProcessing;
    T_Render *Render;
    T_UpdateSubWindowsSize *UpdateSubWindowsSize;
    T_InitializeMemory *InitializeMemory;
    T_SetSubWindowsForResizing *SetSubWindowsForResizing;
} TextEditor;

#pragma pack(push, 1)
typedef struct {
    u16 fileType;
    u32 fileSize;
    u16 reserved1;
    u16 reserved2;
    u32 bitmapOffset;
    u32 size;
    i32 width;
    i32 height;
    u16 planes;
    u16 bitsPerPixel;
} BitmapHeader;
#pragma pack(pop)

typedef struct {
    i32 width;
    i32 height;

    u32 *pixels;
} BMPImage;

//TODO(JENH): Cursor "coordinates" must change to something more granular than "char coordinates".
typedef struct {
    v2_f32 glyphDimensions;
    
    b32 monoSpacedTabStops;
    f32 tabStops[256];

    BMPImage glyphAtlas;
} CharacterInfo;

typedef struct {
    StringBuffer *firstEmpty;
    StringBuffer array[256];
} StringBuffers;

typedef struct {
    char exeDirPath[MAX_PATH];
    u32 exeDirPathSize;

    CharacterInfo characterInfo;

    InputSystem inputSystem;

    SubWindowGridSystem subWindowGridSystem;
    SubWindow *savedSubWindow;
    SubWindow *currentSubWindow;

    char *copyBuffer;
    LengthBaseString *lastCopiedString;

    u32 defaultStringBufferSize;
    char *firstAddresEmpty;

    StringBuffers stringBuffers;
    StringBuffer *currentBuffer;

    SubWindow miniBufferSW;
    StringBuffer miniBuffer;
} State;

extern "C" MACRO_KeyProcessing(KeyProcessing);
extern "C" MACRO_Render(Render);
extern "C" MACRO_InitializeMemory(InitializeMemory);
extern "C" MACRO_UpdateSubWindowsSize(UpdateSubWindowsSize);
extern "C" MACRO_SetSubWindowsForResizing(SetSubWindowsForResizing);

#endif //TEXTEDITOR_H
