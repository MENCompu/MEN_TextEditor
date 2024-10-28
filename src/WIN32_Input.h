#ifndef TE_WIN32_INPUT_H
#define TE_WIN32_INPUT_H

Internal inline void InitializeNoCharInputTable(u8 *noCharInputTalbe) {
    noCharInputTalbe[VK_LEFT]  = LeftArrow;
    noCharInputTalbe[VK_UP]    = UpArrow;
    noCharInputTalbe[VK_RIGHT] = RightArrow;
    noCharInputTalbe[VK_DOWN]  = DownArrow;

    noCharInputTalbe[VK_LBUTTON]  = LeftMouse;
    noCharInputTalbe[VK_RBUTTON]  = RightMouse;
    noCharInputTalbe[VK_MBUTTON]  = MiddleMouse;
    noCharInputTalbe[VK_XBUTTON1] = X1Mouse;
    noCharInputTalbe[VK_XBUTTON2] = X2Mouse;

    noCharInputTalbe[VK_F1]  = F1;
    noCharInputTalbe[VK_F2]  = F2;
    noCharInputTalbe[VK_F3]  = F3;
    noCharInputTalbe[VK_F4]  = F4;
    noCharInputTalbe[VK_F5]  = F5;
    noCharInputTalbe[VK_F6]  = F6;
    noCharInputTalbe[VK_F7]  = F7;
    noCharInputTalbe[VK_F8]  = F8;
    noCharInputTalbe[VK_F9]  = F9;
    noCharInputTalbe[VK_F10] = F10;
    noCharInputTalbe[VK_F11] = F11;
    noCharInputTalbe[VK_F12] = F12;
    noCharInputTalbe[VK_F13] = F13;
    noCharInputTalbe[VK_F14] = F14;
    noCharInputTalbe[VK_F15] = F15;
    noCharInputTalbe[VK_F16] = F16;
    noCharInputTalbe[VK_F17] = F17;
    noCharInputTalbe[VK_F18] = F18;
    noCharInputTalbe[VK_F19] = F19;
    noCharInputTalbe[VK_F20] = F20;
    noCharInputTalbe[VK_F21] = F21;
    noCharInputTalbe[VK_F22] = F22;
    noCharInputTalbe[VK_F23] = F23;
    noCharInputTalbe[VK_F24] = F24;

    noCharInputTalbe[VK_CANCEL]   = Cancel;
    noCharInputTalbe[VK_PRIOR]    = Prior;
    noCharInputTalbe[VK_NEXT]     = Next;
    noCharInputTalbe[VK_END]      = End;
    noCharInputTalbe[VK_HOME]     = Home;
    noCharInputTalbe[VK_SELECT]   = Select;
    noCharInputTalbe[VK_PRINT]    = Print;
    noCharInputTalbe[VK_EXECUTE]  = Execute;
    noCharInputTalbe[VK_SNAPSHOT] = Snapshot;
    noCharInputTalbe[VK_INSERT]   = Insert;
    noCharInputTalbe[VK_DELETE]   = Delete;
    noCharInputTalbe[VK_HELP]     = Help;
}

#include "WIN32_Input.h"
#endif //TE_WIN32_INPUT_H
