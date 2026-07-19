#include "MyApp.h"
#include "V8Handler.h"




void MyApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             CefRefPtr<CefV8Context> context) {
    // 获取全局对象 window
    CefRefPtr<CefV8Value> global = context->GetGlobal();

    // 创建 callNative 函数
    CefRefPtr<CefV8Value> callNativeFunc =
        CefV8Value::CreateFunction("callNative", new V8Handler());

    // 绑定到 window.callNative
    global->SetValue("callNative", callNativeFunc, V8_PROPERTY_ATTRIBUTE_NONE);
}

void MyApp::OnContextReleased(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefV8Context> context) {
    // 清理工作（可选）
}