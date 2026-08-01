//
// Created by cmorg on 6/30/2026.
//

#include "Platform.h"
#include "src/defines.h"

#include <iostream>
#include <ostream>

#include "src/modules/engine/Memory/DynamicArray.h"

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

DynamicArray<KeyState> keyInputs;
DynamicArray<MouseState> mouseInputs;

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

#if FOXFIRE_PLATFORM_WINDOWS == 1

#include "windows.h"
#include "windowsx.h"
#include "../Renderer/Vulkan/VulkanBackend.h"
#include "vulkan/vulkan.h"
#include "vulkan/vulkan_win32.h"

struct InternalState {
    HINSTANCE instance;
    HWND hwnd;
    VkSurfaceKHR surface;
};

static double clockFrequency = 0;
static LARGE_INTEGER startTime{};

void setupClock() {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clockFrequency = 1.0 / static_cast<double>(frequency.QuadPart);
    QueryPerformanceCounter(&startTime);
}

LRESULT CALLBACK win32_process_message(HWND hwnd, unsigned int msg, WPARAM wParam, LPARAM lParam) {
    //These are overrides
    switch (msg) {
        case WM_ERASEBKGND:
            //Tells to OS that erasing will be handled by the engine to prevent flickering
            return 1;
        case WM_CLOSE:
            EngineEvents::callEvent(QUIT, EngineInputContext{});
            return 1;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            RECT rect;
            GetClientRect(hwnd, &rect);
            const unsigned short width = rect.right - rect.left;
            const unsigned short height = rect.bottom - rect.top;
            EngineEvents::callEvent(RESIZED, EngineInputContext{width, height});
            break;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            bool pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            auto key = static_cast<Keys>(wParam);
            bool isExtended = (HIWORD(lParam) & KF_EXTENDED) == KF_EXTENDED;

            if (wParam == VK_MENU) {
                key = isExtended ? KEY_RALT : KEY_LALT;
            } else if (wParam == VK_SHIFT) {
                const unsigned int leftShift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
                const unsigned int scancode = (lParam & (0xFF << 16)) >> 16;
                key = scancode == leftShift ? KEY_LSHIFT : KEY_RSHIFT;
            } else if (wParam == VK_CONTROL) {
                key = isExtended ? KEY_RCONTROL : KEY_LCONTROL;
            }

            keyInputs.emplace(MAX_BUTTONS, key, pressed);
            return 0;
        }
        case WM_MOUSEMOVE: {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            mouseInputs.emplace(xPos, yPos, 0);
            break;
        }
        case WM_MOUSEWHEEL: {
            //Sets the mousewheel value to 1(UP) or -1(DOWN)
            int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (zDelta != 0) {
                zDelta = zDelta < 0 ? -1 : 1;
            }
            mouseInputs.emplace(0, 0, zDelta);
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
                keyInputs.emplace(mouseButton, MAX_KEYS, pressed);
            }
            break;
        }
        default:
            break;
    }

    //Any not handled here are defaulted to windows
    return DefWindowProcA(hwnd, msg, wParam, lParam);
};

bool Platform::initialize(const String &applicationName, const int x, const int y, const int width, const int height) {
    platformState.unknownState = malloc( sizeof(InternalState) );
    const auto castedState = static_cast<InternalState*>(platformState.unknownState);

    keyInputs.initialize();
    mouseInputs.initialize();

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

    setupClock();

    return true;
}

void Platform::ff_sleep(const unsigned long ms) {
    Sleep(ms);
}

void Platform::getRequiredExtensions(DynamicArray<const char*>& extensions) {
    extensions.push("VK_KHR_win32_surface");
}

bool Platform::createSurface() {
    VulkanContext* vulkanContext = &VulkanBackend::vulkanContext;
    const auto state = static_cast<InternalState *>(platformState.unknownState);

    VkWin32SurfaceCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    createInfo.hinstance = state->instance;
    createInfo.hwnd = state->hwnd;

    if (const VkResult result = vkCreateWin32SurfaceKHR(vulkanContext->getInstance(), &createInfo, nullptr, &state->surface); result != VK_SUCCESS) {
        printConsoleError("Failed to create Vulkan surface for windwos.", 0);
        return false;
    }

    vulkanContext->getSurface() = state->surface;
    return true;
}

void Platform::printConsoleMessage(const String& message, const unsigned char color) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!(color > 4 || color < 0)) {
        static unsigned char colors[5] = {64, 4, 6, 2, 1};
        SetConsoleTextAttribute(consoleHandle, colors[color]);
    }

    OutputDebugStringA(message.c_str());

    cout << message << endl;
}

void Platform::printConsoleError(const String& message, const unsigned char color) {
    HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
    if (!(color > 4 || color < 0)) {
        static unsigned char colors[5] = {64, 4, 6, 2, 1};
        SetConsoleTextAttribute(consoleHandle, colors[color]);
    }

    OutputDebugStringA(message.c_str());

    cerr << message << endl;
}

double Platform::getAbsoluteTime() {
    if (clockFrequency == 0) {
        setupClock();
    }

    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    return clockFrequency * static_cast<double>(currentTime.QuadPart);
}

Platform::~Platform() {
    if (platformState.unknownState != nullptr) shutdown();
}

void Platform::shutdown() {
    if (const auto state = static_cast<InternalState *>(platformState.unknownState); state->hwnd != nullptr) {
        DestroyWindow(state->hwnd);
        state->hwnd = nullptr;
    }

    keyInputs.shutdown();
    mouseInputs.shutdown();

    free(platformState.unknownState);
    platformState.unknownState = nullptr;
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
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <sys/time.h>

#if _POSIX_C_SOURCE >= 199309L
    #include <ctime>
#else
    #include <unistd.h>
#endif

#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>
#include "src/modules/engine/Renderer/Vulkan/VulkanBackend.h"

struct InternalState {
    Display* display;
    xcb_connection_t* connection;
    xcb_window_t window;
    xcb_screen_t* screen;
    xcb_atom_t wm_protocols;
    xcb_atom_t wm_delete_win;
    VkSurfaceKHR surface;
};

    Platform::~Platform() {
        if (platformState.unknownState) shutdown();
    }

void Platform::printConsoleMessage(const String &message, const unsigned char color) {
        String output;

        if (!(color > 4 || color < 0)) {
            const char* colors[] = {"0;41", "1;31", "1;33", "1;32", "1;34"};
            output = "\033[" + String(colors[color]) + "m" + message + "\033[0m";
        } else {
            output = message;
        }

        cout << output << endl;
    }

void Platform::printConsoleError(const String &message, const unsigned char color) {
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

bool Platform::createSurface() {
        VulkanContext* vulkanContext = &VulkanBackend::vulkanContext;
        const auto state = static_cast<InternalState *>(platformState.unknownState);

        VkXcbSurfaceCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};
        createInfo.connection = state->connection;
        createInfo.window = state->window;

        if (const VkResult result = vkCreateXcbSurfaceKHR(vulkanContext->getInstance(), &createInfo, nullptr, &state->surface); result != VK_SUCCESS) {
            printConsoleError("Failed to create Vulkan surface for linux.", 0);
            return false;
        }

        vulkanContext->getSurface() = state->surface;
        return true;
    }

Keys translateKeycode(unsigned int keyCode) {
        switch (keyCode) {
        case XK_BackSpace:
            return KEY_BACKSPACE;
        case XK_Return:
            return KEY_ENTER;
        case XK_Tab:
            return KEY_TAB;
            //case XK_Shift: return KEY_SHIFT;
            //case XK_Control: return KEY_CONTROL;

        case XK_Pause:
            return KEY_PAUSE;
        case XK_Caps_Lock:
            return KEY_CAPITAL;

        case XK_Escape:
            return KEY_ESCAPE;

            // Not supported
            // case : return KEY_CONVERT;
            // case : return KEY_NONCONVERT;
            // case : return KEY_ACCEPT;

        case XK_Mode_switch:
            return KEY_MODECHANGE;

        case XK_space:
            return KEY_SPACE;
        case XK_Prior:
            return KEY_PRIOR;
        case XK_Next:
            return KEY_NEXT;
        case XK_End:
            return KEY_END;
        case XK_Home:
            return KEY_HOME;
        case XK_Left:
            return KEY_LEFT;
        case XK_Up:
            return KEY_UP;
        case XK_Right:
            return KEY_RIGHT;
        case XK_Down:
            return KEY_DOWN;
        case XK_Select:
            return KEY_SELECT;
        case XK_Print:
            return KEY_PRINT;
        case XK_Execute:
            return KEY_EXECUTE;
        // case XK_snapshot: return KEY_SNAPSHOT; // not supported
        case XK_Insert:
            return KEY_INSERT;
        case XK_Delete:
            return KEY_DELETE;
        case XK_Help:
            return KEY_HELP;

        case XK_Meta_L:
            return KEY_LWIN;
        case XK_Meta_R:
            return KEY_RWIN;
            // case XK_apps: return KEY_APPS; // not supported

            // case XK_sleep: return KEY_SLEEP; //not supported

        case XK_KP_0:
            return KEY_NUMPAD0;
        case XK_KP_1:
            return KEY_NUMPAD1;
        case XK_KP_2:
            return KEY_NUMPAD2;
        case XK_KP_3:
            return KEY_NUMPAD3;
        case XK_KP_4:
            return KEY_NUMPAD4;
        case XK_KP_5:
            return KEY_NUMPAD5;
        case XK_KP_6:
            return KEY_NUMPAD6;
        case XK_KP_7:
            return KEY_NUMPAD7;
        case XK_KP_8:
            return KEY_NUMPAD8;
        case XK_KP_9:
            return KEY_NUMPAD9;
        case XK_multiply:
            return KEY_MULTIPLY;
        case XK_KP_Add:
            return KEY_ADD;
        case XK_KP_Separator:
            return KEY_SEPARATOR;
        case XK_KP_Subtract:
            return KEY_SUBTRACT;
        case XK_KP_Decimal:
            return KEY_DECIMAL;
        case XK_KP_Divide:
            return KEY_DIVIDE;
        case XK_F1:
            return KEY_F1;
        case XK_F2:
            return KEY_F2;
        case XK_F3:
            return KEY_F3;
        case XK_F4:
            return KEY_F4;
        case XK_F5:
            return KEY_F5;
        case XK_F6:
            return KEY_F6;
        case XK_F7:
            return KEY_F7;
        case XK_F8:
            return KEY_F8;
        case XK_F9:
            return KEY_F9;
        case XK_F10:
            return KEY_F10;
        case XK_F11:
            return KEY_F11;
        case XK_F12:
            return KEY_F12;
        case XK_F13:
            return KEY_F13;
        case XK_F14:
            return KEY_F14;
        case XK_F15:
            return KEY_F15;
        case XK_F16:
            return KEY_F16;
        case XK_F17:
            return KEY_F17;
        case XK_F18:
            return KEY_F18;
        case XK_F19:
            return KEY_F19;
        case XK_F20:
            return KEY_F20;
        case XK_F21:
            return KEY_F21;
        case XK_F22:
            return KEY_F22;
        case XK_F23:
            return KEY_F23;
        case XK_F24:
            return KEY_F24;

        case XK_Num_Lock:
            return KEY_NUMLOCK;
        case XK_Scroll_Lock:
            return KEY_SCROLL;

        case XK_KP_Equal:
            return KEY_NUMPAD_EQUAL;

        case XK_Shift_L:
            return KEY_LSHIFT;
        case XK_Shift_R:
            return KEY_RSHIFT;
        case XK_Control_L:
            return KEY_LCONTROL;
        case XK_Control_R:
            return KEY_RCONTROL;
        case XK_Alt_L:
            return KEY_LALT;
        case XK_Alt_R:
            return KEY_RALT;
        case XK_semicolon:
            return KEY_SEMICOLON;
        case XK_plus:
            return KEY_PLUS;
        case XK_comma:
            return KEY_COMMA;
        case XK_minus:
            return KEY_MINUS;
        case XK_period:
            return KEY_PERIOD;
        case XK_slash:
            return KEY_SLASH;
        case XK_grave:
            return KEY_GRAVE;
        case XK_a:
        case XK_A:
            return KEY_A;
        case XK_b:
        case XK_B:
            return KEY_B;
        case XK_c:
        case XK_C:
            return KEY_C;
        case XK_d:
        case XK_D:
            return KEY_D;
        case XK_e:
        case XK_E:
            return KEY_E;
        case XK_f:
        case XK_F:
            return KEY_F;
        case XK_g:
        case XK_G:
            return KEY_G;
        case XK_h:
        case XK_H:
            return KEY_H;
        case XK_i:
        case XK_I:
            return KEY_I;
        case XK_j:
        case XK_J:
            return KEY_J;
        case XK_k:
        case XK_K:
            return KEY_K;
        case XK_l:
        case XK_L:
            return KEY_L;
        case XK_m:
        case XK_M:
            return KEY_M;
        case XK_n:
        case XK_N:
            return KEY_N;
        case XK_o:
        case XK_O:
            return KEY_O;
        case XK_p:
        case XK_P:
            return KEY_P;
        case XK_q:
        case XK_Q:
            return KEY_Q;
        case XK_r:
        case XK_R:
            return KEY_R;
        case XK_s:
        case XK_S:
            return KEY_S;
        case XK_t:
        case XK_T:
            return KEY_T;
        case XK_u:
        case XK_U:
            return KEY_U;
        case XK_v:
        case XK_V:
            return KEY_V;
        case XK_w:
        case XK_W:
            return KEY_W;
        case XK_x:
        case XK_X:
            return KEY_X;
        case XK_y:
        case XK_Y:
            return KEY_Y;
        case XK_z:
        case XK_Z:
            return KEY_Z;
        default:
            return MAX_KEYS;
    }
    }

bool Platform::processMessages() {
        const auto state = static_cast<InternalState *>(platformState.unknownState);

        xcb_generic_event_t* event = nullptr;
        xcb_client_message_event_t* message;
        bool quitFlag = false;

        //Poll events until null is returned
        while ((event = xcb_poll_for_event(state->connection)) != nullptr) {
            //input events
            switch (event->response_type & ~0x80) {
                case XCB_KEY_PRESS:
                case XCB_KEY_RELEASE: {
                    auto* keyEvent = reinterpret_cast<xcb_key_press_event_t *>(event);
                    bool pressed = event->response_type == XCB_KEY_PRESS;
                    const xcb_keycode_t code = keyEvent->detail;
                    const KeySym keySym = XkbKeycodeToKeysym(
                        state->display,
                        code,
                        0,
                        code & ShiftMask ? 1 : 0
                    );
                    Keys key  = translateKeycode(keySym);

                    if (key == MAX_KEYS) {
                        Logger::logWarn("Linux failed to translate a key! Input will be ignored!");
                        break;
                    }

                    keyInputs.emplace(MAX_BUTTONS, key, pressed);
                    break;
                }
                case XCB_BUTTON_PRESS:
                case XCB_BUTTON_RELEASE: {
                    const auto* mouseEvent = reinterpret_cast<xcb_button_press_event_t *>(event);
                    bool pressed = event->response_type == XCB_BUTTON_PRESS;
                    Buttons mouseButton = MAX_BUTTONS;
                    switch (mouseEvent->detail) {
                        case XCB_BUTTON_INDEX_1:
                            mouseButton = BUTTON_LEFT;
                            break;
                        case XCB_BUTTON_INDEX_2:
                            mouseButton = BUTTON_MIDDLE;
                            break;
                        case XCB_BUTTON_INDEX_3:
                            mouseButton = BUTTON_RIGHT;
                            break;
                        default: break;
                    }

                    if (mouseButton != MAX_BUTTONS) {
                        keyInputs.emplace(mouseButton, MAX_KEYS, pressed);
                    }
                    break;
                }
                case XCB_MOTION_NOTIFY: {
                    auto* moveEvent = reinterpret_cast<xcb_motion_notify_event_t *>(event);
                    mouseInputs.emplace(moveEvent->event_x, moveEvent->event_y, 0);
                    break;
                }
                case XCB_CONFIGURE_NOTIFY: {
                    const auto* configureEvent = reinterpret_cast<xcb_configure_notify_event_t *>(event);
                    const unsigned short x = configureEvent->width;
                    const unsigned short y = configureEvent->height;
                    EngineEvents::callEvent(RESIZED, EngineInputContext{x, y});
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


void Platform::getRequiredExtensions(DynamicArray<const char*>& extensions) {
        extensions.push("VK_KHR_xcb_surface");
    }

double Platform::getAbsoluteTime() {
        timespec now{};
        clock_gettime(CLOCK_MONOTONIC, &now);
        return static_cast<double>(now.tv_sec) + static_cast<double>(now.tv_nsec) * 0.000000001;
    }

void ff_sleep(unsigned long ms) {
#if _POSIX_C_SOURCE >= 199309L
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000 * 1000;
        nanosleep(&ts, 0);
#else
        if (ms >= 1000) {
            sleep(ms / 1000);
        }
        usleep((ms % 1000) * 1000);
#endif
        }

bool Platform::initialize(const String &applicationName, const int x, const int y, const int width, const int height) {
        platformState.unknownState = malloc(sizeof(InternalState));
        const auto castedState = static_cast<InternalState *>(platformState.unknownState);

        castedState->display = XOpenDisplay(nullptr);
        XSetEventQueueOwner(castedState->display, XCBOwnsEventQueue);

        //Turn off repeating keys
        XAutoRepeatOff(castedState->display);

        castedState->connection = XGetXCBConnection(castedState->display);

        if (xcb_connection_has_error(castedState->connection)) {
            printConsoleError("Failed to connect to X server via XCB", 0);
            return false;
        }

        //Data from server
        const struct xcb_setup_t* setup = xcb_get_setup(castedState->connection);

        //Loop through screens
        xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
        constexpr int screen = 0;
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
            static_cast<int16_t>(x),
            static_cast<int16_t>(y),
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
            printConsoleError("Failed to flush XCB connection" + std::to_string(streamResult), 0);
            return false;
        }

        return true;
    }

void Platform::shutdown() {
        const InternalState* castedState = static_cast<InternalState *>(platformState.unknownState);

        //Turn on repeating keys since this seems to toggle it in the os itself
        XAutoRepeatOn(castedState->display);

        xcb_destroy_window(castedState->connection, castedState->window);

        keyInputs.shutdown();
        mouseInputs.shutdown();

        free(platformState.unknownState);
        platformState.unknownState = nullptr;
    }


#endif



