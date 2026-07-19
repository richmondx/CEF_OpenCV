#ifndef MY_APP_H
#define MY_APP_H

#include "include/cef_app.h"

class MyApp : public CefApp,
              public CefRenderProcessHandler {
public:
    MyApp() = default;

    // CefApp methods
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return this;
    }

    // CefRenderProcessHandler methods
    void OnContextCreated(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) override;

    void OnContextReleased(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           CefRefPtr<CefV8Context> context) override;

private:
    IMPLEMENT_REFCOUNTING(MyApp);
};

#endif // MY_APP_H