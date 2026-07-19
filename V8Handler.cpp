#include "V8Handler.h"
#include "ipc_const.h"
#include <iostream>

bool V8Handler::Execute(const CefString& name,
                        CefRefPtr<CefV8Value> object,
                        const CefV8ValueList& arguments,
                        CefRefPtr<CefV8Value>& retval,
                        CefString& exception) {
    if (name == "callNative") {
        // 检查参数数量
        if (arguments.size() < 1 || !arguments[0]->IsString()) {
            exception = "callNative requires a string argument";
            return true;
        }

        // 获取 JS 传入的参数
        CefString message = arguments[0]->GetStringValue();
        std::cout << "[Renderer] JS call callNative: " << message.ToString() << std::endl;

        // 发送 IPC 消息到 Browser 进程
        CefRefPtr<CefProcessMessage> msg =
            CefProcessMessage::Create(IPC_JS_TO_BROWSER);
        msg->GetArgumentList()->SetString(0, message);

        // 获取当前上下文和浏览器，发送消息
        CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
        if (context && context->GetBrowser()&& context->GetBrowser()->GetMainFrame()) {
            context->GetBrowser()->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);
            std::cout << "[Renderer] IPC messgat to Browser proces" << std::endl;
        }

        // 返回 undefined
        retval = CefV8Value::CreateUndefined();
        return true;
    }

    return false;
}