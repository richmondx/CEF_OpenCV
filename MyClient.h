#ifndef MY_CLIENT_H
#define MY_CLIENT_H

#include "include/cef_client.h"
#include "RenderHandler.h"
#include <windows.h>

// ★★★ 在类定义之前定义 WindowInfo ★★★
struct WindowInfo {
    int id = 0;
    HWND hwnd = nullptr;
    CefRefPtr<CefBrowser> browser;
    int width = 0;
    int height = 0;
};

class MyClient : public CefClient,
                 public CefLifeSpanHandler,
                 public CefLoadHandler {
public:
    explicit MyClient(CefRefPtr<RenderHandler> render_handler);

    // CefClient methods
    CefRefPtr<CefRenderHandler> GetRenderHandler() override {
        return render_handler_;
    }

    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
        return this;
    }

    CefRefPtr<CefLoadHandler> GetLoadHandler() override {
        return this;
    }
    cv::Mat CaptureWindowContent(int window_id);
    // CefLifeSpanHandler methods
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    bool DoClose(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefLoadHandler methods
    void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame,
                   int httpStatusCode) override;

    // CefProcessMessageHandler methods
    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefProcessId source_pid,
                                  CefRefPtr<CefProcessMessage> message) override;

    void AddWindow(int id, HWND hwnd, CefRefPtr<CefBrowser> browser, int w, int h);
    void RemoveWindow(CefRefPtr<CefBrowser> browser);

    WindowInfo* FindWindowByBrowser(CefRefPtr<CefBrowser> browser);
    WindowInfo* FindWindowById(int id);

private:
    std::vector<WindowInfo> windows_;
    std::mutex mutex_;
    CefRefPtr<RenderHandler> render_handler_;
    CefRefPtr<CefBrowser> browser_;

    IMPLEMENT_REFCOUNTING(MyClient);
};

#endif // MY_CLIENT_H