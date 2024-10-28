#ifndef TE_SUBWINDOWGRIDSYSTEM_H
#define TE_SUBWINDOWGRIDSYSTEM_H

#include "Datatypes.h"
#include "Vectors.h"

#define SW_LEFT  0
#define SW_UP    1
#define SW_RIGHT 2
#define SW_DOWN  3

struct SubWindow;

typedef struct AdjacentSubWindowNode {
    SubWindow *adjacent;
    AdjacentSubWindowNode *next;
} AdjacentSubWindowNode;

typedef struct {
    union {
        AdjacentSubWindowNode *list;
        AdjacentSubWindowNode *first;
    };
    AdjacentSubWindowNode *tail;
} AdjacentList;

typedef struct {
    v2_u32 position;
    v2_f32 relPositionForResizing;

    u32 savedPosXForVertMov;

    char *positionInText;

    v3_f32 color;
} Cursor;

struct StringBuffer;

typedef struct SubWindow {
    b32 isActive;

    v2_f32 absTopLeftPosition;
    v2_f32 relTopLeftPosition;

    v2_f32 relDimensions;
    v2_f32 absDimensions;
    v2_u32 dimensionsInGlyphs;

    Cursor cursor;

    v2_u32 screenCharOffsetRelToText;

    char *firstProcessedLine;

    StringBuffer *displayedBuffer;

    union {
        struct {
            AdjacentList adjacentLeft;
            AdjacentList adjacentUp;
            AdjacentList adjacentRight;
            AdjacentList adjacentDown;
        };
        AdjacentList adjacents[4];
    };
} SubWindow;

#define SUBWINDOW_COUNT 256

typedef struct {
    AdjacentSubWindowNode *firstEmpty;
    AdjacentSubWindowNode nodes[SUBWINDOW_COUNT * 4];
} AdjacentNodes;

typedef struct SubWindowGridSystem {
    v2_f32 windowDimensions;

    v2_f32 minTopLeftPadding;
    v2_f32 minBottomRightPadding;

    v2_f32 topLeftPadding;
    v2_f32 bottomRightPadding;

    SubWindow *firstSubWindowEmpty;
    SubWindow subWindows[SUBWINDOW_COUNT];

    AdjacentNodes adjacentNodePool;
} SubWindowGridSystem;

Internal void MoveScreen(SubWindow *subWindow, v2_i32 cursorOffsetTopLeft, v2_i32 cursorOffsetBottomRight);
Internal inline void MoveScreenIfNecessary(SubWindow *subWindow);

#define MACRO_UpdateSubWindowsSize(name) void name(Memory *memory, v2_f32 newWindowDimensions)
typedef MACRO_UpdateSubWindowsSize(T_UpdateSubWindowsSize);
MACRO_UpdateSubWindowsSize(STUB_UpdateSubWindowsSize) { }

#define MACRO_SetSubWindowsForResizing(name) void name(SubWindowGridSystem *subWindowGridSystem, SubWindow *miniBufferSubWindow)
typedef MACRO_SetSubWindowsForResizing(T_SetSubWindowsForResizing);
MACRO_SetSubWindowsForResizing(STUB_SetSubWindowsForResizing) { }

#endif //TE_SUBWINDOWGRIDSYSTEM_H
