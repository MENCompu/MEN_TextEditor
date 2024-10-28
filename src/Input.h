#ifndef TE_INPUT_H
#define TE_INPUT_H

typedef enum {
    LeftArrow,
    UpArrow,
    RightArrow,
    DownArrow,

    LeftMouse,
    RightMouse,
    MiddleMouse,
    X1Mouse,
    X2Mouse,

    F1,  F2,  F3,  F4,  F5,  F6,
    F7,  F8,  F9,  F10, F11, F12,
    F13, F14, F15, F16, F17, F18,
    F19, F20, F21, F22, F23, F24,

    Cancel,
    Prior,
    Next,
    End,
    Home,
    Select,
    Print,
    Execute,
    Snapshot,
    Insert,
    Delete,
    Help
} NoCharInput;

#define MAX_CONTROL_KEYS 9

#define LRShifts    LeftShift   | RightShift
#define LRControls  LeftControl | RightControl
#define LRAlts      LeftAlt     | RightAlt

typedef enum {
    LeftShift      = 0x001,
    RightShift     = 0x002,
    LeftControl    = 0x004,
    RightControl   = 0x008,
    LeftAlt        = 0x010,
    RightAlt       = 0x020,
    CapsLock       = 0x040,
    NumLock        = 0x080,
    ScrollLock     = 0x100,
} ControlKeysFlags;

typedef enum {
    NormalMode,
    InsertMode,
    VisualMode,
    TerminalMode,
    CommandLineMode
} InputMode;

typedef struct {
    InputMode inputMode;

    b32 isChar;
    u32 controlKeysFlags;
} InputSystem;

#endif //TE_INPUT_H
