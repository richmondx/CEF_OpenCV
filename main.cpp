
#include "include/cef_app.h"
#include "include/cef_client.h"
#include "MyApp.h"
#include "MyClient.h"
#include "RenderHandler.h"
#include <iostream>
#include <thread>
#include <chrono>
bool windowless_rendering_enabled = true;
std::wstring relativewurl = L"html/index.html";
std::wstring fullurl = L"E:/CEF_OpenCV_Demo/html/index.html";

// 获取可执行文件所在目录
std::wstring GetExecutableDir() {
    wchar_t path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    std::wstring exePath(path);
    size_t pos = exePath.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        return exePath.substr(0, pos);
    }
    return L".";
}

// 构建文件 URL
std::wstring BuildFileUrl(const std::wstring& relativePath) {
    std::wstring baseDir = GetExecutableDir();
    std::wstring fullPath = baseDir + L"/" + relativePath;

    // 将反斜杠替换为正斜杠
    std::replace(fullPath.begin(), fullPath.end(), L'\\', L'/');

    return L"file:///" + fullPath;
}

// 全局浏览器实例指针
CefRefPtr<CefBrowser> g_browser = nullptr;
HINSTANCE GetCodeModuleHandle() {
    HMODULE hModule = nullptr;
    ::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(GetCodeModuleHandle),
        &hModule);
    return hModule;
}


// 窗口过程函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        return 0;
    case WM_SIZE: {
        CefBrowser *browser =
            (CefBrowser*)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (browser && browser->GetHost()) {
            HWND browser_hwnd = browser->GetHost()->GetWindowHandle();
            if (browser_hwnd) {
                RECT rect;
                GetClientRect(hwnd, &rect);
                SetWindowPos(browser_hwnd, NULL,
                    rect.left, rect.top,
                    rect.right - rect.left,
                    rect.bottom - rect.top,
                    SWP_NOZORDER);
            }
        }
        return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// 窗口配置
struct WindowConfig {
    int id;
    int width;
    int height;
    std::wstring title;
    std::wstring url;
    int x;
    int y;
};

// 创建窗口
HWND CreateWindowWithBrowser(HINSTANCE hInstance,
    const WindowConfig& config,
    CefRefPtr<MyClient> client) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSEX wc = { 0 };
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = L"CEFWindowClass";
        RegisterClassEx(&wc);
        registered = true;
    }

    HWND hwnd = CreateWindowEx(
        0,
        L"CEFWindowClass",
        config.title.c_str(),
        WS_OVERLAPPEDWINDOW,
        config.x, config.y,
        config.width, config.height,
        NULL, NULL,
        hInstance,
        NULL
    );

    if (!hwnd) {
        std::cerr << "[Window " << config.id << "] CreateWindow failed" << std::endl;
        return NULL;
    }

    RECT rect;
    GetClientRect(hwnd, &rect);

    CefWindowInfo window_info;
    CefRect cefRect(rect.left, rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top);
    window_info.SetAsChild(hwnd, cefRect);

    CefBrowserSettings browser_settings;

    CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(
        window_info, client.get(),
        config.url,
        browser_settings,
        nullptr,
        nullptr
    );

    if (browser) {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)browser.get());
        client->AddWindow(config.id, hwnd, browser, config.width, config.height);

        std::cout << "[Window " << config.id << "] Created: "
            << config.width << "x" << config.height
            << " at (" << config.x << "," << config.y << ")" << std::endl;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    return hwnd;
}

int main(int argc, char* argv[]) {
    std::cout << "=== CEF OSR + OpenCV  ===" << std::endl;
    std::cout << "start..." << std::endl;
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // CEF 主参数
    CefMainArgs main_args(GetCodeModuleHandle());
    CefRefPtr<MyApp> app(new MyApp);

    // 处理子进程
    int exit_code = CefExecuteProcess(main_args, app.get(), nullptr);
    if (exit_code >= 0) {
        return exit_code;
    }

    // CEF 设置
    CefSettings settings;
    settings.no_sandbox = true;
    settings.windowless_rendering_enabled = windowless_rendering_enabled;
    settings.multi_threaded_message_loop = false;
    
    // 初始化 CEF
    if (!CefInitialize(main_args, settings, app.get(), nullptr)) {
        std::cerr << "CEF failed!" << std::endl;
        return -1;
    }

    // 创建离屏渲染窗口信息
    CefWindowInfo window_info;
    window_info.SetAsWindowless(nullptr);
    window_info.shared_texture_enabled = true;
    // 浏览器设置
    CefBrowserSettings browser_settings;

    // 创建 RenderHandler 和 Client
    CefRefPtr<RenderHandler> render_handler(new RenderHandler(1280, 720));
    CefRefPtr<MyClient> client(new MyClient(render_handler));

   // auto url=BuildFileUrl(relativewurl.c_str());
    // 创建浏览器（同步方式）
    if (windowless_rendering_enabled)
    {
        g_browser = CefBrowserHost::CreateBrowserSync(
            window_info, client.get(),
            fullurl.c_str(),
            browser_settings,
            nullptr,
            nullptr);

        if (!g_browser) {
            std::cerr << "browser create failed!" << std::endl;
            CefShutdown();
            return -1;
        }
    }
    else
    {
        std::vector<WindowConfig> configs = {
       {1, 1100, 780, L"Window 1 - Main",    fullurl, 80, 40}
        };

        for (const auto& config : configs) {
            CreateWindowWithBrowser(GetCodeModuleHandle(), config, client);
        }

        std::cout << "All windows created. Press Ctrl+C to exit." << std::endl;
    }

   

    bool running = true;
    // 控制台消息循环（替代 CefRunMessageLoop）
    while (true) {
        // 处理 CEF 消息
        CefDoMessageLoopWork();

        if (windowless_rendering_enabled)
        {
            if (g_browser) {
                g_browser->GetHost()->Invalidate(PET_VIEW);  // 强制触发 OnPaint
            }
        }
        // 每 10ms 轮询一次，避免 CPU 满载
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // 关闭 CEF
    if (windowless_rendering_enabled)
    {
        g_browser = nullptr;
    }
    CefShutdown();

    std::cout << "exit" << std::endl;
    return 0;
}