//
// Created by cmorg on 6/30/2026.
//

#include "Platform.h"
#include "src/defines.h"

#include <iostream>
#include <ostream>

struct KeyState {
    Buttons button;
    Keys key;
    bool bIsPressed;
};
struct MouseState {
    short x;
    short y;
    char z;
};

std::vector<KeyState> keyInputs{};
std::vector<MouseState> mouseInputs{};

void Platform::processInputs(IInputSystem &inputSystem) {
    for (const KeyState& input : keyInputs) {
        if (input.key == MAX_KEYS) {
            inputSystem.processButton(input.button, input.bIsPressed);
        } else {
            inputSystem.processKey(input.key, input.bIsPressed);
        }
    }
    keyInputs.clear();

    for (const MouseState& mouse : mouseInputs) {
        if (mouse.z != 0) {
            inputSystem.processMouseScroll(mouse.z);
        } else {
            inputSystem.processMouseMove(mouse.x, mouse.y);
        }
    }
    mouseInputs.clear();
}

Platform::Platform()
    : platformState{}
{

}

#if FOXFIRE_PLATFORM_WINDOWS == 1

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
            auto key = static_cast<Keys>(wParam);
            keyInputs.emplace_back(MAX_BUTTONS, key, pressed);
            break;
        }
        case WM_MOUSEMOVE: {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            mouseInputs.emplace_back(xPos, yPos, 0);
            break;
        }
        case WM_MOUSEWHEEL: {
            //Sets the mousewheel value to 1(UP) or -1(DOWN)
            int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (zDelta != 0) {
                zDelta = zDelta < 0 ? -1 : 1;
            }
            mouseInputs.emplace_back(0, 0, zDelta);
            break;
        }
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:  {
            bool pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            Buttons mouseButton = MAX_BUTTONS;
            switch (msg) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    mouseButton = BUTTON_LEFT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    mouseButton = BUTTON_MIDDLE;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    mouseButton = BUTTON_RIGHT;
                    break;
                default:
                    break;
            }
            if (mouseButton != MAX_BUTTONS) {
                keyInputs.emplace_back(mouseButton, MAX_KEYS, pressed);
            }
            break;
        }
        default:
            break;
    }

    //Any not handled here are defaulted to windows
    return DefWindowProcA(hwnd, msg, wParam, lParam);
};

bool Platform::initialize(const String &applicationName, int x, int y, int width, int height) {
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
        return false;
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
        return false;
    }

    castedState->hwnd = handle;

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

    return true;
}

void Platform::printConsoleMessage(const char* message, const unsigned char color) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!(color > 4 || color < 0)) {
        static unsigned char colors[5] = {64, 4, 6, 2, 1};
        SetConsoleTextAttribute(consoleHandle, colors[color]);
    }

    OutputDebugStringA(message);

    cout << message << endl;
}

void Platform::printConsoleError(const char *message, const unsigned char color) {
    HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
    if (!(color > 4 || color < 0)) {
        static unsigned char colors[5] = {64, 4, 6, 2, 1};
        SetConsoleTextAttribute(consoleHandle, colors[color]);
    }

    OutputDebugStringA(message);

    cerr << message << endl;
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

    free(platformState.unknownState);
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
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <sys/time.h>

#if _POSIX_C_SOURCE >= 199309L
    #include <ctime>
#else
    #include <unistd.h>
#endif

struct InternalState {
    Display* display;
    xcb_connection_t* connection;
    xcb_window_t window;
    xcb_screen_t* screen;
    xcb_atom_t wm_protocols;
    xcb_atom_t wm_delete_win;
};

    Platform::~Platform() {
        const InternalState* castedState = static_cast<InternalState *>(platformState.unknownState);

        //Turn on repeating keys since this seems to toggle it in the os itself
        XAutoRepeatOn(castedState->display);

        xcb_destroy_window(castedState->connection, castedState->window);

        free(platformState.unknownState);
    }

void Platform::printConsoleMessage(const char *message, const unsigned char color) {
        String output;

        if (!(color > 4 || color < 0)) {
            const char* colors[] = {"0;41", "1;31", "1;33", "1;32", "1;34"};
            output = "\033[" + String(colors[color]) + "m" + message + "\033[0m";
        } else {
            output = message;
        }

        cout << output << endl;
}

void Platform::printConsoleError(const char *message, unsigned char color) {
        String output;

        if (!(color > 4 || color < 0)) {
            const char* colors[] = {"0;41", "1;31", "1;33", "1;32", "1;34"};
            output = "\033[" + String(colors[color]) + "m" + message + "\033[0m";
        } else {
            output = message;
        }

        cerr << output << endl;

        if (color == 0) exit(-1);
}

bool Platform::processMessages() {
        const auto state = static_cast<InternalState *>(platformState.unknownState);

        xcb_generic_event_t* event;
        xcb_client_message_event_t* message;
        bool quitFlag = false;

        //Poll events until null is returned
        while (event != nullptr) {
            event = xcb_poll_for_event(state->connection);
            if (event == nullptr) break;

            //input events
            switch (event->response_type & ~0x80) {
                case XCB_KEY_PRESS:
                case XCB_KEY_RELEASE: {
                    break;
                }
                case XCB_BUTTON_PRESS:
                case XCB_BUTTON_RELEASE: {
                    break;
                }
                case XCB_MOTION_NOTIFY: {
                    break;
                }
                case XCB_CONFIGURE_NOTIFY: {
                    break;
                }
                case XCB_CLIENT_MESSAGE: {
                    message = reinterpret_cast<xcb_client_message_event_t *>(event);
                    //Close window
                    if (message->data.data32[0] == state->wm_delete_win) {
                        quitFlag = true;
                    }
                    break;
                }
                default: {
                    break;
                }
            }

            free(event);
        }

        return !quitFlag;
}

float Platform::getAbsoluteTime() const {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        return now.tv_sec + now.tv_nsec * 0.000000001;
}

Platform::Platform(const std::string& applicationName, const int x, const int y, const int width, const int height)
        : platformState{}
    {
        platformState.unknownState = malloc(sizeof(InternalState));
        const auto castedState = static_cast<InternalState *>(platformState.unknownState);

        castedState->display = XOpenDisplay(nullptr);

        //Turn off repeating keys
        XAutoRepeatOff(castedState->display);

        castedState->connection = XGetXCBConnection(castedState->display);

        if (xcb_connection_has_error(castedState->connection)) {
            printConsoleError("Failed to connect to X server via XCB", 0);
        }

        //Data from server
        const struct xcb_setup_t* setup = xcb_get_setup(castedState->connection);

        //Loop through screens
        xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
        int screen = 0;
        for (int i = screen; i > 0; i--) {
            xcb_screen_next(&iter);
        }

        //Assign screens
        castedState->screen = iter.data;

        castedState->window = xcb_generate_id(castedState->connection);

        //Register events
        //Fills background color
        constexpr unsigned int eventMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
        //Keyboard and mouse
        constexpr unsigned int eventValues = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                                             XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                                             XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |
                                             XCB_EVENT_MASK_STRUCTURE_NOTIFY;
        //Background color, followed by events
        const unsigned int values[] = {castedState->screen->black_pixel, eventValues};

        //Create the window
        xcb_void_cookie_t cookie = xcb_create_window(
            castedState->connection,
            XCB_COPY_FROM_PARENT,
            castedState->window,
            castedState->screen->root,
            x,
            y,
            width,
            height,
            0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            castedState->screen->root_visual,
            eventMask,
            values
            );

        //Change the title
        xcb_change_property(
            castedState->connection,
            XCB_PROP_MODE_REPLACE,
            castedState->window,
            XCB_ATOM_WM_NAME,
            XCB_ATOM_STRING,
            8,
            applicationName.length(),
            applicationName.c_str()
        );

        const xcb_intern_atom_cookie_t deleteCookie = xcb_intern_atom(castedState->connection, 0, 16, "WM_DELETE_WINDOW");
        const xcb_intern_atom_cookie_t protocolCookie = xcb_intern_atom(castedState->connection, 0, 12, "WM_PROTOCOLS");
        const xcb_intern_atom_reply_t* deleteReply = xcb_intern_atom_reply(castedState->connection, deleteCookie, nullptr);
        const xcb_intern_atom_reply_t* protocolReply = xcb_intern_atom_reply(castedState->connection, protocolCookie, nullptr);
        castedState->wm_delete_win = deleteReply->atom;
        castedState->wm_protocols = protocolReply->atom;

        xcb_change_property(
            castedState->connection,
            XCB_PROP_MODE_REPLACE,
            castedState->window,
            protocolReply->atom,
            4,
            32,
            1,
            &deleteReply->atom
        );

        xcb_map_window(castedState->connection, castedState->window);

        if (const int streamResult = xcb_flush(castedState->connection); streamResult <= 0) {
            printConsoleError(("Failed to flush XCB connection" + std::to_string(streamResult)).c_str(), 0);
        }
    }


#endif



