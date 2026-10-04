//
// Created by cmorg on 9/26/2026.
//

#include "PlatformWindows.h"

#if FOXFIRE_PLATFORM_WINDOWS == 1

PlatformWindows* PlatformWindows::self = nullptr;

void PlatformWindows::setupClock() {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clockFrequency = 1.0 / static_cast<double>(frequency.QuadPart);
    QueryPerformanceCounter(&startTime);
}

LRESULT CALLBACK win32ProcessMessage(HWND hwnd, unsigned int msg, WPARAM wParam, LPARAM lParam) {
    //These are overrides
    switch (msg) {
        case WM_ERASEBKGND:
            //Tells to OS that erasing will be handled by the engine to prevent flickering
            return 1;
        case WM_CLOSE:
            EngineEvents::closeApplication();
            return 1;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            RECT rect;
            GetClientRect(hwnd, &rect);
            const unsigned short width = rect.right - rect.left;
            const unsigned short height = rect.bottom - rect.top;
            EngineEvents::resizeApplication(EngineInputContext{width, height});
            break;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            const bool pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            auto key = static_cast<Keys>(wParam);
            const bool isExtended = (HIWORD(lParam) & KF_EXTENDED) == KF_EXTENDED;

            if (wParam == VK_MENU) {
                key = isExtended ? KEY_RALT : KEY_LALT;
            } else if (wParam == VK_SHIFT) {
                const unsigned int leftShift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
                const unsigned int scancode = (lParam & (0xFF << 16)) >> 16;
                key = scancode == leftShift ? KEY_LSHIFT : KEY_RSHIFT;
            } else if (wParam == VK_CONTROL) {
                key = isExtended ? KEY_RCONTROL : KEY_LCONTROL;
            }

            PlatformWindows::self->addKeyInput(MAX_BUTTONS, key, pressed);
            return 0;
        }
        case WM_MOUSEMOVE: {
            const int xPos = GET_X_LPARAM(lParam);
            const int yPos = GET_Y_LPARAM(lParam);
            PlatformWindows::self->addMouseInput(xPos, yPos, 0);
            break;
        }
        case WM_MOUSEWHEEL: {
            //Sets the mousewheel value to 1(UP) or -1(DOWN)
            int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (zDelta != 0) {
                zDelta = zDelta < 0 ? -1 : 1;
            }
            PlatformWindows::self->addMouseInput(0, 0, zDelta);
            break;
        }
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:  {
            const bool pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
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
                PlatformWindows::self->addKeyInput(mouseButton, MAX_KEYS, pressed);
            }
            break;
        }
        default:
            break;
    }

    //Any not handled here are defaulted to windows
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

PlatformWindows::PlatformWindows()
    : IPlatformState(sizeof(PlatformWindows))
{}

bool PlatformWindows::initialize(const String &applicationName, const int x, const int y, const int width, const int height) {
    IPlatformState::initialize(applicationName, x , y , width, height);

    self = this;

    instance = GetModuleHandleA(nullptr);
    HICON icon = LoadIcon(instance, IDI_APPLICATION);
    WNDCLASSA wc = {};
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = win32ProcessMessage;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = instance;
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
        instance,
        nullptr);

    if (handle == nullptr) {
        MessageBoxA(nullptr, "Failed to create window!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        printConsoleError("Failed to create window!", 0);
        return false;
    }

    hwnd = handle;

    //Whether to allow user input
    constexpr bool bAcceptInput = true;
    constexpr int showWindowCommandFlags = bAcceptInput ? SW_SHOW : SW_SHOWNOACTIVATE;
    //Initial Minimize SW_MINIMIZE : SW_SHOWMINNOACTIVE
    //Initial Maximize SW_SHOWMAXIMIZED : SW_MAXIMIZE
    ShowWindow(hwnd, showWindowCommandFlags);

    setupClock();

    return true;
}

void PlatformWindows::addKeyInput(Buttons button, Keys key, bool isPressed) {
    keyInputs.emplace(button, key, isPressed);
}

void PlatformWindows::addMouseInput(int x, int y, int z) {
    mouseInputs.emplace(x, y, z);
}

void PlatformWindows::printConsoleMessage(const String &message, const unsigned char color) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!(color > 4 || color < 0)) {
        static unsigned char colors[5] = {64, 4, 6, 2, 1};
        SetConsoleTextAttribute(consoleHandle, colors[color]);
    }

    OutputDebugStringA(message.c_str());

    cout << message << endl;
}

void PlatformWindows::printConsoleError(const String &message, const unsigned char color) {
    HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
    if (!(color > 4 || color < 0)) {
        static unsigned char colors[5] = {64, 4, 6, 2, 1};
        SetConsoleTextAttribute(consoleHandle, colors[color]);
    }

    OutputDebugStringA(message.c_str());

    cerr << message << endl;
}

bool PlatformWindows::createSurface() {
    VulkanContext* vulkanContext = &VulkanBackend::vulkanContext;

    VkWin32SurfaceCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    createInfo.hinstance = instance;
    createInfo.hwnd = hwnd;

    if (const VkResult result = vkCreateWin32SurfaceKHR(vulkanContext->getInstance(), &createInfo, vulkanContext->getAllocator(), &surface); result != VK_SUCCESS) {
        printConsoleError("Failed to create Vulkan surface for windows.", 0);
        return false;
    }

    vulkanContext->getSurface() = surface;
    return true;
}

double PlatformWindows::getAbsoluteTime() {
    if (clockFrequency == 0) {
        setupClock();
    }

    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    return clockFrequency * static_cast<double>(currentTime.QuadPart);
}

void PlatformWindows::shutdown() {
    if (hwnd != nullptr) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }

    IPlatformState::shutdown();

    self = nullptr;
}

bool PlatformWindows::processMessages() {
    MSG msg;
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return true;
}

void PlatformWindows::getRequiredExtensions(DynamicArray<const char *> &extensions) {
    extensions.push("VK_KHR_win32_surface");
}

void PlatformWindows::ff_sleep(const unsigned long ms) {
    Sleep(ms);
}

void *PlatformWindows::allocate(const unsigned long size, bool align) {
    return malloc(size);
}

void PlatformWindows::freeMemory(void *memory, bool align) {
    free(memory);
}

void PlatformWindows::clear(void *memory, const unsigned long size) {
    memset(memory, 0, size);
}

int PlatformWindows::getProcessorCount() {
    SYSTEM_INFO sysInfo{};
    GetSystemInfo(&sysInfo);
    Logger::logInfo(std::to_string(sysInfo.dwNumberOfProcessors) + " cores were found.");
    return sysInfo.dwNumberOfProcessors;
}

bool PlatformWindows::createThread(const ThreadFunction threadFunction, void *params, const bool autoDetach, Thread &outThread) {
    if (threadFunction == nullptr) return false;

    outThread.data = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(threadFunction), params, 0, reinterpret_cast<DWORD *>(&outThread.id));

    Logger::logDebug("Starting proces on thread " + std::to_string(outThread.id));

    if (!outThread.data) {
        return false;
    }
    if (autoDetach) {
        CloseHandle(outThread.data);
    }

    return true;
}

void PlatformWindows::destroyThread(Thread &thread) {
    if (!thread.data) return;

    DWORD exitCode = 0;
    GetExitCodeThread(thread.data, &exitCode);
    CloseHandle(thread.data);
    thread.data = nullptr;
    thread.id = 0;
}

void PlatformWindows::cancelThread(Thread &thread) {
    if (!thread.data) return;

    TerminateThread(thread.data, 0);
    thread.data = nullptr;
}

void PlatformWindows::detachThread(Thread &thread) {
    if (!thread.data) return;

    CloseHandle(thread.data);
    thread.data = nullptr;
}

bool PlatformWindows::isThreadActive(Thread &thread) {
    if (!thread.data) return false;

    DWORD exitCode = WaitForSingleObject(thread.data, 0);
    if (exitCode == WAIT_TIMEOUT) return true;

    return false;
}

void PlatformWindows::pauseThread(Thread &thread, const ULong ms) {
    ff_sleep(ms);
}

ULong PlatformWindows::getCurrentThreadId() {
    return GetCurrentThreadId();
}

bool PlatformWindows::createMutex(Mutex &outMutex) {
    outMutex.data = CreateMutex(nullptr, 0, nullptr);
    if (outMutex.data == nullptr) {
        Logger::logError("Failed to create mutex.");
        return false;
    }

    return true;
}

void PlatformWindows::destroyMutex(Mutex &mutex) {
    if (!mutex.data) return;

    CloseHandle(mutex.data);
    mutex.data = nullptr;
}

bool PlatformWindows::lockMutex(Mutex &mutex) {
    DWORD result = WaitForSingleObject(mutex.data, INFINITE);
    switch (result) {
        case WAIT_OBJECT_0: {
            return true;
        }
        case WAIT_ABANDONED: {
            Logger::logError("Failed to lock mutex.");
            return false;
        }
        default: break;
    }

    return true;
}

bool PlatformWindows::unlockMutex(Mutex &mutex) {
    if (!mutex.data) return false;

    const int result = ReleaseMutex(mutex.data);
    return result != 0;
}

#endif
