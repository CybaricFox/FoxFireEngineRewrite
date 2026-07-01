//
// Created by cmorg on 6/30/2026.
//

#include "Platform.h"
#include "src/defines.h"

#if FOXFIRE_PLATFORM_WINDOWS == 1

#include <iostream>
#include <ostream>

#include "windows.h"
#include "windowsx.h"

struct InternalState {
    HINSTANCE instance;
    HWND hwnd;
};

static float clockFrequency;
static LARGE_INTEGER startTime;

LRESULT CALLBACK win32_process_message(HWND hwnd, unsigned int msg, WPARAM wParam, LPARAM lParam) {
    //These are overrides
    switch (msg) {
        case WM_ERASEBKGND:
            //Tells to OS that erasing will be handled by the engine to prevent flickering
            return 1;
        case WM_CLOSE:
            exit(0);
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            RECT rect;
            GetClientRect(hwnd, &rect);
            unsigned int width = rect.right - rect.left;
            unsigned int height = rect.bottom - rect.top;
            break;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            bool pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            break;
        }
        case WM_MOUSEMOVE: {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            break;
        }
        case WM_MOUSEWHEEL: {
            //Sets the mousewheel value to 1(UP) or -1(DOWN)
            if (int zDelta = GET_WHEEL_DELTA_WPARAM(wParam); zDelta != 0) {
                zDelta = zDelta < 0 ? -1 : 1;
            }
            break;
        }
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:  {
            bool pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            break;
        }
        default:
            break;
    }

    //Any not handled here are defaulted to windows
    return DefWindowProcA(hwnd, msg, wParam, lParam);
};

Platform::Platform(const std::string &applicationName, const int x, const int y, const int width, const int height)
    : platformState{}
{
    platformState.unknownState = malloc( sizeof(InternalState) );
    const auto castedState = static_cast<InternalState*>(platformState.unknownState);

    castedState->instance = GetModuleHandleA(nullptr);

    HICON icon = LoadIcon(castedState->instance, IDI_APPLICATION);
    WNDCLASSA wc = {};
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = win32_process_message;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = castedState->instance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = "FoxFire_Window";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(nullptr, "Failed to register window!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        printConsoleError("Failed to register window!", 0);
    }

    int windowX = x;
    int windowY = y;
    int windowWidth = width;
    int windowHeight = height;

    //Overlapped makes the border appear
    int windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    constexpr int windowExStyle = WS_EX_APPWINDOW;

    windowStyle |= WS_MAXIMIZEBOX; //Allow maximize to appear on border
    windowStyle |= WS_MINIMIZEBOX; //Allow minimize to appear on border
    windowStyle |= WS_THICKFRAME;

    //get the border size
    RECT border = {0, 0, 0, 0};
    AdjustWindowRectEx(&border, windowStyle, false, windowExStyle);

    //Prevents negative values
    windowX = windowX + border.left;
    windowY = windowY + border.top;

    //Include the border in the width and height
    windowWidth += border.right - border.left;
    windowHeight += border.bottom - border.top;

    HWND handle = CreateWindowExA(
        windowExStyle,
        "FoxFire_Window",
        applicationName.c_str(),
        windowStyle,
        windowX,
        windowY,
        windowWidth,
        windowHeight,
        nullptr,
        nullptr,
        castedState->instance,
        nullptr);

    if (handle == nullptr) {
        MessageBoxA(nullptr, "Failed to create window!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        printConsoleError("Failed to create window!", 0);
    } else {
        castedState->hwnd = handle;
    }

    //Whether to allow user input
    constexpr bool bAcceptInput = true;
    constexpr int showWindowCommandFlags = bAcceptInput ? SW_SHOW : SW_SHOWNOACTIVATE;
    //Initial Minimize SW_MINIMIZE : SW_SHOWMINNOACTIVE
    //Initial Maximize SW_SHOWMAXIMIZED : SW_MAXIMIZE
    ShowWindow(castedState->hwnd, showWindowCommandFlags);

    //Setup clock
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clockFrequency = 1 / frequency.QuadPart;
    QueryPerformanceCounter(&startTime);
}

void Platform::printConsoleMessage(const char* message, const unsigned char color) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!(color > 4 || color < 0)) {
        static unsigned char colors[5] = {64, 4, 6, 2, 1};
        SetConsoleTextAttribute(consoleHandle, colors[color]);
    }

    OutputDebugStringA(message);

    std::cout << message << std::endl;
}

void Platform::printConsoleError(const char *message, const unsigned char color) {
    HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
    if (!(color > 4 || color < 0)) {
        static unsigned char colors[5] = {64, 4, 6, 2, 1};
        SetConsoleTextAttribute(consoleHandle, colors[color]);
    }

    OutputDebugStringA(message);

    std::cerr << message << std::endl;

    if (color == 0) exit(-1);
}

float Platform::getAbsoluteTime() const {
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    return currentTime.QuadPart * clockFrequency;
}

Platform::~Platform() {
    if (const auto state = static_cast<InternalState *>(platformState.unknownState); state->hwnd != nullptr) {
        DestroyWindow(state->hwnd);
        state->hwnd = nullptr;
    }
}

//Tells the platform to process the windows messages to OS.
bool Platform::processMessages() {
    MSG msg;
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return true;
}

#endif


#if FOXFIRE_PLATFORM_LINUX == 1
    Platform::~Platform(){};

    Platform::Platform(PlatformState *state, std::string applicationName, int x, int y, int width, int height) {

    }


#endif



