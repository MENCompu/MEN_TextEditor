//Starting date: 2023-8-21

#include <windows.h>
#include <stdio.h>

#include "WIN32_TextEditor.h"
#include "WIN32_Input.h"
#include "Profiling.h"

GlobalVariable b32 globalRunning;
GlobalVariable b32 globalFullScreen;
GlobalVariable DWORD globalWindowMaximizedFlag;
GlobalVariable HWND globalWindow;
GlobalVariable WIN32_WindowBuffer globalWindowBuffer;
GlobalVariable TextEditor textEditor;

//TODO(JENH): May be a bad idea.
GlobalVariable Memory memory;

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE hinstprev, LPSTR cmdline, int cmdshow) {
    v2_i32 windowDimensions = {1280, 720};

    RECT clientRect = {0, 0, windowDimensions.x, windowDimensions.y};

    AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, false);

    clientRect.right += -clientRect.left;
    clientRect.bottom += -clientRect.top;

    WNDCLASSA windowClass = {0};

    windowClass.style = CS_VREDRAW|CS_HREDRAW;
    windowClass.lpfnWndProc = WIN32_EventCallback;
    windowClass.hInstance = instance;
//  windowClass.hIcon = ;
//  windowClass.hCursor = ;
    windowClass.lpszClassName = "TextEditor";

    if (!RegisterClassA(&windowClass)) {
	OutputDebugStringA("The window class did not register successfully");
	return 0;
    }
	
    globalWindow = CreateWindowExA(0, windowClass.lpszClassName, "Text Editor", WS_VISIBLE, 
		                   CW_USEDEFAULT, CW_USEDEFAULT, clientRect.right, clientRect.bottom, 
				   0, 0, instance, 0);

    globalFullScreen = true;
    WIN32_ToggleFullNormalWindowSize();

    if (!globalWindow) {
	OutputDebugStringA("Failed at creating window.");
	return 0;
    }

    Dimensions dimensions = WIN32_GetWindowDimensions();

    WIN32_ResizeBuffer(&globalWindowBuffer, dimensions.width, dimensions.height);
    globalRunning = true;

    char executableDirectoryPath[MAX_PATH];
    DWORD sizeOfFileName = GetModuleFileNameA(0, executableDirectoryPath, sizeof(executableDirectoryPath));

    char *endExecutableDirectoryPath = FindCharInStringBackward(executableDirectoryPath + sizeOfFileName - 1, 
		                                                executableDirectoryPath, '\\', 1);

    u32 executableDirectoryPathSize = (u32)(endExecutableDirectoryPath - executableDirectoryPath + 1);

    char DLLPath[MAX_PATH];
    char DLLFileName[] = "TEPlatformIndep.dll";
    CopyStringForward(DLLPath, executableDirectoryPath, executableDirectoryPathSize);
    CopyStringForward(DLLPath + executableDirectoryPathSize, DLLFileName, ARRAYLENGTH(DLLFileName));

    char tempDLLPath[MAX_PATH];
    char tempDLLFileName[] = "TEPlatformIndep_temp.dll";
    CopyStringForward(tempDLLPath, executableDirectoryPath, executableDirectoryPathSize);
    CopyStringForward(tempDLLPath + executableDirectoryPathSize, tempDLLFileName, 
		      ARRAYLENGTH(tempDLLFileName));

    textEditor = LoadTextEditor(DLLPath, tempDLLPath);

#ifdef TE_INTERNAL
    LPVOID baseAddress = (LPVOID)TERABYTES((u64)2);
#else
    LPVOID baseAddress = 0;
#endif

    memory.CloseProgram = *WIN32_CloseProgram;
    memory.ToggleFullNormalWindowSize = *WIN32_ToggleFullNormalWindowSize;
    memory.ReadEntireFile = *WIN32_ReadEntireFile;
    memory.WriteNewFile = *WIN32_WriteNewFile;
    memory.FreeFileMemory = *WIN32_FreeFileMemory;
    memory.GetOSPageSize = *WIN32_GetOSPageSize;

    memory.permanentSize = MEGABYTES(64);
    memory.temporarySize = MEGABYTES(64);
    u64 totalSize = memory.permanentSize + memory.temporarySize;

    memory.permanent = VirtualAlloc(baseAddress, (size_t)totalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    memory.temporary = ((u8 *)memory.permanent + memory.permanentSize);

    State *state = (State *)memory.permanent;

    CopyStringForward(state->exeDirPath, executableDirectoryPath, executableDirectoryPathSize);
    state->exeDirPathSize = executableDirectoryPathSize;

    int pathsCount = 0; 

    char **filePaths = (char **)memory.temporary;

    if (cmdline) {
        CommandLineArgsToCStandarA((char *)cmdline, filePaths, &pathsCount);
    }

    textEditor.InitializeMemory(&memory, Tof32(windowDimensions), filePaths, (i32)pathsCount);

    u8 noCharInputTable[256];
    InitializeNoCharInputTable(noCharInputTable);

    while (globalRunning) {
        WaitMessage();

        FILETIME UpdateWriteTime = WIN32_GetFileWriteTime(textEditor.DLLPath);

	//TODO(JENH): Hot reloading have problems with global variables in the dll
	//	      (like the log variable).
	if (CompareFileTime(&UpdateWriteTime, &textEditor.lastTimeWritten)) {
	    UnloadTextEditor();
            textEditor = LoadTextEditor(DLLPath, tempDLLPath);
	}

        MSG event;
        GetMessageA(&event, 0, 0, 0);

        WIN32_ProcessPendingEvents(&event, noCharInputTable);

	WindowBuffer windowBuffer = {0};

        windowBuffer.width = globalWindowBuffer.width;
        windowBuffer.height = globalWindowBuffer.height;
        windowBuffer.pitch = globalWindowBuffer.pitch;
        windowBuffer.bytesPerPixel = globalWindowBuffer.bytesPerPixel;
        windowBuffer.memory = globalWindowBuffer.memory;

	textEditor.Render(&memory, &windowBuffer);

	HDC deviceContext = GetDC(globalWindow);

        Dimensions dimensions = WIN32_GetWindowDimensions();
        WIN32_UpdateWindow(deviceContext, dimensions.width, dimensions.height);

	ReleaseDC(globalWindow, deviceContext);
    }

    return 1;
}

Internal inline void WIN32_ProcessPendingEvents(MSG *event, u8 *noCharInputTable) {
	switch (event->message){
	    case WM_QUIT: {
		globalRunning = false;
	    } break;

	    case WM_KEYUP: 
	    case WM_SYSKEYUP: {
	        WPARAM virtualKey = event->wParam;
	        LPARAM virtualKeyInfo = event->lParam;

		State *state = (State *)memory.permanent;

                u32 *controlKeysFlags = &state->inputSystem.controlKeysFlags;
		b32 isControlKey = false;

		switch (virtualKey) {
		    case VK_SHIFT: {
			u8 scanCode = (u8)(virtualKeyInfo >> 16);
			u32 virtualKeyExtended = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
			u32 shiftFlag = (virtualKeyExtended == VK_LSHIFT) ? LeftShift : RightShift;
		        DEL_FLAGS(*controlKeysFlags, shiftFlag);
                        isControlKey = true;
		    } break;

		    case VK_CONTROL: {
		        u32 isLeftControlKeyVersion = ((virtualKeyInfo & (1 << 24)) == 0);
			u32 controlFlag = (isLeftControlKeyVersion) ? LeftControl : RightControl;
		        DEL_FLAGS(*controlKeysFlags, controlFlag);
                        isControlKey = true;
		    } break;

		    case VK_MENU: {
		        u32 isLeftControlKeyVersion = ((virtualKeyInfo & (1 << 24)) == 0);
			u32 altFlag = (isLeftControlKeyVersion) ? LeftAlt : RightAlt;
		        DEL_FLAGS(*controlKeysFlags, altFlag);
                        isControlKey = true;
		    } break;
		}

		if (isControlKey && *controlKeysFlags) {
		    u32 flags = *controlKeysFlags;
		    u32 bit = flags & 0x01;

		    OutputDebugStringA("| ");

		    if (bit) {
		        OutputDebugStringA("LeftShift | ");
		    }

		    flags = flags >> 1;
                    bit = flags & 0x01;
		    if (bit) {
		        OutputDebugStringA("RightShift | ");
		    }

		    flags = flags >> 1;
                    bit = flags & 0x01;
		    if (bit) {
		        OutputDebugStringA("LeftControl | ");
		    }

		    flags = flags >> 1;
                    bit = flags & 0x01;
		    if (bit) {
	                OutputDebugStringA("RightControl | ");
		    }

		    flags = flags >> 1;
                    bit = flags & 0x01;
		    if (bit) {
		        OutputDebugStringA("LeftAlt | ");
		    }

		    flags = flags >> 1;
                    bit = flags & 0x01;
		    if (bit) {
		        OutputDebugStringA("RightAlt | ");
		    }

		    OutputDebugStringA("\n");
		}

	    } break;

	    case WM_SYSKEYDOWN:
	    case WM_KEYDOWN: {
	        u32 virtualKey = (u32)event->wParam;
	        LPARAM virtualKeyInfo = event->lParam;
		u8 scanCode = (u8)(virtualKeyInfo >> 16);

		b32 keyIsDown  = ((virtualKeyInfo & (1 << 31)) == 0);
		b32 keyWasDown = ((virtualKeyInfo & (1 << 30)) != 0);

		State *state = (State *)memory.permanent;

                u32 *controlKeysFlags = &state->inputSystem.controlKeysFlags;
		
		b32 isControlKey = false;

		switch (virtualKey) {
		    case VK_SHIFT: {
			u32 virtualKeyExtended = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
			u32 shiftFlag = (virtualKeyExtended == VK_LSHIFT) ? LeftShift : RightShift;
		        ADD_FLAGS(*controlKeysFlags, shiftFlag);
                        isControlKey = true;
		    } break;

		    case VK_CONTROL: {
		        u32 isLeftControlKeyVersion = ((virtualKeyInfo & (1 << 24)) == 0);
			u32 controlFlag = (isLeftControlKeyVersion) ? LeftControl : RightControl;
		        ADD_FLAGS(*controlKeysFlags, controlFlag);
                        isControlKey = true;
		    } break;

		    case VK_MENU: {
		        u32 isLeftControlKeyVersion = ((virtualKeyInfo & (1 << 24)) == 0);
			u32 altFlag = (isLeftControlKeyVersion) ? LeftAlt : RightAlt;
		        ADD_FLAGS(*controlKeysFlags, altFlag);
                        isControlKey = true;
		    } break;

		    case VK_CAPITAL: {
			u16 isToggleOn = (u16)(GetKeyState(VK_CAPITAL) & 0x0001);
		        isToggleOn ? ADD_FLAGS(*controlKeysFlags, CapsLock) 
				   : DEL_FLAGS(*controlKeysFlags, CapsLock);
                        isControlKey = true;
		    } break;

		    default: {
			BYTE keyboardState[256];

                        ZeroMem(keyboardState, ARRAYLENGTH(keyboardState));

			if (AT_LEAST_ONE_FLAG_IS_SET(*controlKeysFlags, LRShifts|CapsLock)) {
                            keyboardState[VK_SHIFT] = 0b10000000;
			}

			WORD keyWord;
			b32 isChar = ToAscii((UINT)virtualKey, scanCode, keyboardState, &keyWord, false);

                        u32 key;

			if (isChar) {
			    key = (u32)keyWord;
                            state->inputSystem.isChar = true;
			} else {
			    key = (u32)noCharInputTable[virtualKey];
                            state->inputSystem.isChar = false;
			}

			textEditor.KeyProcessing(&memory, key, keyIsDown);
		    } break;
		}

		if (isControlKey && *controlKeysFlags) {
		    u32 flags = *controlKeysFlags;
		    u32 bit = flags & 0x01;

		    OutputDebugStringA("| ");

		    if (bit) {
		        OutputDebugStringA("LeftShift | ");
		    }

		    flags = flags >> 1;
                    bit = flags & 0x01;
		    if (bit) {
		        OutputDebugStringA("RightShift | ");
		    }

		    flags = flags >> 1;
                    bit = flags & 0x01;
		    if (bit) {
		        OutputDebugStringA("LeftControl | ");
		    }

		    flags = flags >> 1;
                    bit = flags & 0x01;
		    if (bit) {
	                OutputDebugStringA("RightControl | ");
		    }

		    flags = flags >> 1;
                    bit = flags & 0x01;
		    if (bit) {
		        OutputDebugStringA("LeftAlt | ");
		    }

		    flags = flags >> 1;
                    bit = flags & 0x01;
		    if (bit) {
		        OutputDebugStringA("RightAlt | ");
		    }

		    flags = flags >> 1;
                    bit = flags & 0x01;
		    if (bit) {
		        OutputDebugStringA("CapsLock | ");
		    }

		    OutputDebugStringA("\n");
		}
	    } break;

	    default: {
		DispatchMessageA(event);
	    } break;
	}
}

LRESULT CALLBACK WIN32_EventCallback(HWND windowHandle, UINT event, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    switch (event) {
	case WM_CLOSE: {
	    globalRunning = false;
	} break;

	case WM_DESTROY: {
	    globalRunning = false;
	} break;

	//TODO(JENH): Double clicking in the title bar doesn't work.
        case WM_SYSCOMMAND: {
	    if (wParam == SC_MAXIMIZE || wParam == SC_RESTORE) {
	        State *state = (State *)memory.permanent;

                textEditor.SetSubWindowsForResizing(&state->subWindowGridSystem, &state->miniBufferSW);
	    }

	    result = DefWindowProcA(windowHandle, event, wParam, lParam);
        } break;

	case WM_ENTERSIZEMOVE: {
            if (!memory.isInitialized) { break; }

	    State *state = (State *)memory.permanent;

            textEditor.SetSubWindowsForResizing(&state->subWindowGridSystem, &state->miniBufferSW);
	} break;

        case WM_SIZE: {
            if (!memory.isInitialized) { break; }

	    OutputDebugString("WM_SIZE\n");

	    u32 windowWidth  = LOWORD(lParam);
	    u32 windowHeight = HIWORD(lParam);

	    if (windowWidth > 0 && windowHeight > 0) {
	        v2_f32 windowDimensions = {(f32)windowWidth, (f32)windowHeight};

                textEditor.UpdateSubWindowsSize(&memory, windowDimensions);

	        WIN32_ResizeBuffer(&globalWindowBuffer, windowWidth, windowHeight);
	    }
	} break;

	case WM_PAINT: {
	    PAINTSTRUCT paint;

	    HDC deviceContext = BeginPaint(windowHandle, &paint);

	    WindowBuffer windowBuffer = {0};

            windowBuffer.width = globalWindowBuffer.width;
            windowBuffer.height = globalWindowBuffer.height;
            windowBuffer.pitch = globalWindowBuffer.pitch;
            windowBuffer.bytesPerPixel = globalWindowBuffer.bytesPerPixel;
            windowBuffer.memory = globalWindowBuffer.memory;

	    textEditor.Render(&memory, &windowBuffer);

            Dimensions dimensions = WIN32_GetWindowDimensions();

            WIN32_UpdateWindow(deviceContext, dimensions.width, dimensions.height);

	    EndPaint(windowHandle, &paint);
	} break;

	case WM_ACTIVATEAPP: {
	    b32 isBeingActivated = (b32)wParam;

	    if (isBeingActivated && memory.permanent) {
		State *state = (State *)memory.permanent;

		u32 controlKeysFlags = 0;
	        
                if (GetAsyncKeyState(VK_LSHIFT)   & 0x8000) { ADD_FLAGS(controlKeysFlags, LeftShift);    }
                if (GetAsyncKeyState(VK_RSHIFT)   & 0x8000) { ADD_FLAGS(controlKeysFlags, RightShift);   }
                if (GetAsyncKeyState(VK_LCONTROL) & 0x8000) { ADD_FLAGS(controlKeysFlags, LeftControl);  }
                if (GetAsyncKeyState(VK_RCONTROL) & 0x8000) { ADD_FLAGS(controlKeysFlags, RightControl); }
                if (GetAsyncKeyState(VK_LMENU)    & 0x8000) { ADD_FLAGS(controlKeysFlags, LeftAlt);      }
                if (GetAsyncKeyState(VK_RMENU)    & 0x8000) { ADD_FLAGS(controlKeysFlags, RightAlt);     }
                if (GetKeyState(VK_CAPITAL)       & 0x0001) { ADD_FLAGS(controlKeysFlags, CapsLock);     }

                state->inputSystem.controlKeysFlags = controlKeysFlags;
	    }

            ToggleTopMostIfWindowIsActive(windowHandle, isBeingActivated);

	} break;
 
	default: {
	    result = DefWindowProcA(windowHandle, event, wParam, lParam);
	} break;
    }

    return result;
}

Internal void WIN32_ResizeBuffer(WIN32_WindowBuffer *windowBuffer, i32 width, i32 height) {
    if (windowBuffer->memory) {
        VirtualFree(windowBuffer->memory, 0, MEM_RELEASE);
    }

    windowBuffer->width = width;
    windowBuffer->height = height;
    windowBuffer->bytesPerPixel = 4;
    windowBuffer->pitch = windowBuffer->width * windowBuffer->bytesPerPixel;

    windowBuffer->info.bmiHeader.biSize = sizeof(windowBuffer->info); 
    windowBuffer->info.bmiHeader.biWidth = windowBuffer->width;
    windowBuffer->info.bmiHeader.biHeight = -windowBuffer->height; 
    windowBuffer->info.bmiHeader.biPlanes = 1;
    windowBuffer->info.bmiHeader.biBitCount = (WORD)(windowBuffer->bytesPerPixel * 8);
    windowBuffer->info.bmiHeader.biCompression = BI_RGB;

    windowBuffer->memory = VirtualAlloc(0, windowBuffer->width * windowBuffer->height *
		                        windowBuffer->bytesPerPixel, MEM_COMMIT, PAGE_READWRITE);
}

Internal void WIN32_UpdateWindow(HDC deviceContext, i32 windowWidth, i32 windowHeight) {
    int xOffset = (globalWindowBuffer.width < windowWidth) ? (windowWidth - globalWindowBuffer.width) / 2 : 0;
    int yOffset = (globalWindowBuffer.height < windowHeight) ? (windowHeight - globalWindowBuffer.height) / 2 : 0;

    StretchDIBits(deviceContext, xOffset, yOffset, globalWindowBuffer.width, globalWindowBuffer.height, 0, 0,
		  globalWindowBuffer.width, globalWindowBuffer.height, globalWindowBuffer.memory, 
		  &globalWindowBuffer.info, DIB_RGB_COLORS, SRCCOPY);
}

u32 WIN32_ReadEntireFile(char *path, void *address) {
    u32 fileSize = 0;   
    HANDLE fileHandle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE) {
	LARGE_INTEGER fileSizeLI;
	if (GetFileSizeEx(fileHandle, &fileSizeLI)) {
	    DWORD bytesRead;
	    if (ReadFile(fileHandle, address, fileSizeLI.LowPart, &bytesRead, 0) && 
			 bytesRead == fileSizeLI.LowPart) {
	        fileSize = (u32)fileSizeLI.LowPart;
	    } else {
	    }
	} else {

	}

        CloseHandle(fileHandle);
    } else {

    }

    return fileSize;
}

b32 WIN32_WriteNewFile(char *newFilePath, void *fileMemory, u32 fileSize) {
    b32 result = false;

    HANDLE fileHandle = CreateFileA(newFilePath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

    if (fileHandle != INVALID_HANDLE_VALUE) {
	DWORD bytesWritten;
	if (WriteFile(fileHandle, fileMemory, fileSize, &bytesWritten, 0)) {
	    result = bytesWritten == fileSize;
	} else {

	}

        CloseHandle(fileHandle);
    } else {

    }

    return result;
}


void WIN32_FreeFileMemory(void *fileMemory) {
    VirtualFree(fileMemory, 0, MEM_RELEASE);
}

//TODO(JENH): Change the string functions to a null termitaded specific functions.
Internal void CommandLineArgsToCStandarA(char *commandLineArgs, char **argv, int *argc) {
    char *garbage = (char *)MAX_VALUE_PTR;

    char *scanner = commandLineArgs;

    for (;;) {
	++(*argc);

	char charsLookingFor1[] = " \t\0";

        scanner = FindSomeOtherCharInStringForward(scanner, garbage, charsLookingFor1, 
			                           ARRAYLENGTH(charsLookingFor1) - 1, 1);

	if (*scanner == '\"') {
	    *argv++ = ++scanner;

	    char charsLookingFor2[] = "\"\0";

            scanner = FindSomeCharInStringForward(scanner, garbage, charsLookingFor2, 
			                          ARRAYLENGTH(charsLookingFor2) - 1, 1);

	    *scanner++ = '\0';

	    if (*scanner == '\0') { return; }
	} else {
	    *argv++ = scanner;

            scanner = FindSomeCharInStringForward(scanner, garbage, charsLookingFor1, 
			                          ARRAYLENGTH(charsLookingFor1) - 1, 1);

	    if (*scanner == '\0') { return; }

	    *scanner++ = '\0';
	}
    }
}

Internal TextEditor LoadTextEditor(char *DLLPath, char *tempDLLPath) {
    TextEditor result = {0};

    result.DLLPath = DLLPath;

    CopyFile(DLLPath, tempDLLPath, FALSE);

    result.lastTimeWritten = WIN32_GetFileWriteTime(DLLPath);

    result.DLLHandle = LoadLibraryA(tempDLLPath);

    if (result.DLLHandle) {
        result.KeyProcessing = (T_KeyProcessing *)GetProcAddress(result.DLLHandle, "KeyProcessing");
        result.Render = (T_Render *)GetProcAddress(result.DLLHandle, "Render");
        result.UpdateSubWindowsSize = (T_UpdateSubWindowsSize *)GetProcAddress(result.DLLHandle, 
			                                                     "UpdateSubWindowsSize");
        result.InitializeMemory = (T_InitializeMemory *)GetProcAddress(result.DLLHandle, "InitializeMemory");
        result.SetSubWindowsForResizing = (T_SetSubWindowsForResizing *)GetProcAddress(result.DLLHandle, 
			                                                "SetSubWindowsForResizing");

	result.isValid = result.KeyProcessing && result.Render && 
		         result.InitializeMemory && result.UpdateSubWindowsSize && 
			 result.SetSubWindowsForResizing;
    }

    if (!result.isValid) {
	result.KeyProcessing = STUB_KeyProcessing;
        result.Render = STUB_Render;
        result.UpdateSubWindowsSize = STUB_UpdateSubWindowsSize;
        result.InitializeMemory = STUB_InitializeMemory;
        result.SetSubWindowsForResizing = STUB_SetSubWindowsForResizing;
    }

    return result;
}

Internal void UnloadTextEditor() {
    if (textEditor.DLLHandle) {
	FreeLibrary(textEditor.DLLHandle);
        textEditor.DLLHandle = 0;
    }

    textEditor.isValid = false;
    textEditor.KeyProcessing = STUB_KeyProcessing;
    textEditor.Render = STUB_Render;
}

Internal inline FILETIME WIN32_GetFileWriteTime(char *fileName) {
    FILETIME result = {0};
    WIN32_FILE_ATTRIBUTE_DATA fileData;

    if (GetFileAttributesExA(fileName, GetFileExInfoStandard, &fileData)) {
	result = fileData.ftLastWriteTime;
    }

    return result;
}

Internal Dimensions WIN32_GetWindowDimensions() {
    Dimensions result = {0};

    RECT rect;
    GetClientRect(globalWindow, &rect);
    
    result.width = rect.right - rect.left;
    result.height = rect.bottom - rect.top;

    return result;
}

void WIN32_CloseProgram() {
    globalRunning = false;
}

Internal inline void ToggleTopMostIfWindowIsActive(HWND windowHandle, b32 isBeingActivated) {
    if (globalFullScreen) {
	if (isBeingActivated) {
	    SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0, 
			 SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
	} else { 
	    SetWindowPos(windowHandle, HWND_NOTOPMOST, 0, 0, 0, 0, 
			 SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
	}
    }
}

void WIN32_ToggleFullNormalWindowSize() {
    if (globalFullScreen) {
        WIN32_NormalWindow();
        globalFullScreen = false;
    } else {
        WIN32_MaximizeWindow();
        globalFullScreen = true;
    }
}

Internal inline void WIN32_MaximizeWindow() {
    DWORD windowStyle = GetWindowLong(globalWindow, GWL_STYLE);
    globalWindowMaximizedFlag = windowStyle & WS_MAXIMIZE;
    SetWindowLongPtrA(globalWindow, GWL_STYLE, windowStyle & ~(WS_OVERLAPPEDWINDOW|globalWindowMaximizedFlag));
    SetWindowPos(globalWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
    ShowWindow(globalWindow, SW_SHOWMAXIMIZED);
}

Internal inline void WIN32_NormalWindow() {
    DWORD windowStyle = GetWindowLong(globalWindow, GWL_STYLE);
    DWORD newStyle = windowStyle|WS_OVERLAPPEDWINDOW;
    SetWindowLongPtrA(globalWindow, GWL_STYLE, newStyle);
    SetWindowPos(globalWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
    ShowWindow(globalWindow, SW_RESTORE);
    SetWindowLongPtrA(globalWindow, GWL_STYLE, (newStyle & ~WS_MAXIMIZE)|globalWindowMaximizedFlag);
}

u32 WIN32_GetOSPageSize() {
    SYSTEM_INFO systemInfo;

    GetSystemInfo(&systemInfo);

    u32 pageSize = (u32)systemInfo.dwPageSize;

    return pageSize;
}
