#include "MyClient.h"
#include "ipc_const.h"
#include <iostream>
#include <include/cef_app.h>

extern bool windowless_rendering_enabled;
void MyClient::AddWindow(int id, HWND hwnd,
    CefRefPtr<CefBrowser> browser,
    int w, int h) {
    std::lock_guard<std::mutex> lock(mutex_);
    windows_.push_back({ id, hwnd, browser, w, h });
    std::cout << "[Client] AddWindow id=" << id
        << " total=" << windows_.size() << std::endl;
}

void MyClient::RemoveWindow(CefRefPtr<CefBrowser> browser) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::remove_if(windows_.begin(), windows_.end(),
        [&](const WindowInfo& w) { return w.browser == browser; });
    if (it != windows_.end()) {
        windows_.erase(it, windows_.end());
        std::cout << "[Client] RemoveWindow, remain="
            << windows_.size() << std::endl;
    }
    if (windows_.empty()) {
        CefQuitMessageLoop();
    }
}

WindowInfo* MyClient::FindWindowByBrowser(CefRefPtr<CefBrowser> browser) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& w : windows_)
        if (w.browser->IsSame(browser)) return &w;
    return nullptr;
}

WindowInfo* MyClient::FindWindowById(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& w : windows_)
        if (w.id == id) return &w;
    return nullptr;
}

cv::Mat MyClient::CaptureWindowContent(int window_id) {
    WindowInfo* info = FindWindowById(window_id);
    if (!info || !info->hwnd || !IsWindow(info->hwnd)) {
        return cv::Mat();
    }

    // 获取客户区大小
    RECT rect;
    GetClientRect(info->hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    if (width <= 0 || height <= 0) {
        return cv::Mat();
    }

    // 使用 BitBlt 捕获窗口内容
    HDC hdcWindow = GetDC(info->hwnd);
    HDC hdcMem = CreateCompatibleDC(hdcWindow);

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, width, height);
    HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);

    BitBlt(hdcMem, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);

    // 转换为 OpenCV Mat
    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    cv::Mat mat(height, width, CV_8UC4);
    GetDIBits(hdcMem, hBitmap, 0, height, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // 清理
    SelectObject(hdcMem, oldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(info->hwnd, hdcWindow);

    // BGRA 转 BGR
    cv::Mat bgr;
    cv::cvtColor(mat, bgr, cv::COLOR_BGRA2BGR);

    // OpenCV 处理示例：灰度转换
    cv::Mat gray;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);

    std::cout << "[Client] Captured window " << window_id
        << ": " << width << "x" << height << std::endl;

    return gray;
}

MyClient::MyClient(CefRefPtr<RenderHandler> render_handler)
    : render_handler_(render_handler) {}

void MyClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    browser_ = browser;
    std::cout << "browse create success" << std::endl;
}

bool MyClient::DoClose(CefRefPtr<CefBrowser> browser) {
   
        return false;
   
}

void MyClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    if (windowless_rendering_enabled)

    {
        browser_ = nullptr;
        std::cout << " browse close" << std::endl;
    }
    else
    {
        RemoveWindow(browser);
    }
}

void MyClient::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         int httpStatusCode) {
    if (windowless_rendering_enabled)

    {
        std::cout << "page load complete status code : " << httpStatusCode << std::endl;
    }

    else
    {
        auto* info = FindWindowByBrowser(browser);
        int wid = info ? info->id : -1;
        std::cout << "[Window " << wid << "] Page loaded, status: "
            << httpStatusCode << std::endl;
    }
}

bool MyClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefProcessId source_pid,
                                        CefRefPtr<CefProcessMessage> message) {
    if (message->GetName() == IPC_JS_TO_BROWSER) {
        auto args = message->GetArgumentList();
        CefString text = args->GetString(0);

        if (windowless_rendering_enabled)
        {
            std::cout << "[Client] Received from JS: " << text.ToString() << std::endl;

            // 回传消息给 JS
            std::wstring jsCallback = L"onNativeResult('C++ received: " +
                text.ToWString() + L"')";
            frame->ExecuteJavaScript(jsCallback, frame->GetURL(), 0);
        }
        else
        {
            auto* wi = FindWindowByBrowser(browser);
            int wid = wi ? wi->id : -1;

            std::cout << "[Window " << wid << "] JS: "
                << text.ToString() << std::endl;

            std::wstring js = L"onNativeResult('Window " +
                std::to_wstring(wid) + L" recv: " +
                text.ToWString() + L"')";
            frame->ExecuteJavaScript(js, frame->GetURL(), 0);
        }

        return true;
    }
    return false;
}