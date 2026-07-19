#ifndef V8_HANDLER_H
#define V8_HANDLER_H

#include "include/cef_v8.h"
#include "include/cef_process_message.h"

class V8Handler : public CefV8Handler {
public:
    V8Handler() = default;

    bool Execute(const CefString& name,
                 CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments,
                 CefRefPtr<CefV8Value>& retval,
                 CefString& exception) override;

private:
    IMPLEMENT_REFCOUNTING(V8Handler);
};

#endif // V8_HANDLER_H