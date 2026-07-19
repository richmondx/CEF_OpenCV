#ifndef RENDER_HANDLER_H
#define RENDER_HANDLER_H

#include "include/cef_render_handler.h"
#include <opencv2/opencv.hpp>
#include <d3d11.h>
#include <mutex>

class RenderHandler : public CefRenderHandler {
public:
    RenderHandler(int width, int height);
    ~RenderHandler();
    // CefRenderHandler methods
    void GetViewRect(CefRefPtr<CefBrowser> browser,
                     CefRect& rect) override;

    void OnPaint(CefRefPtr<CefBrowser> browser,
                 PaintElementType type,
                 const RectList& dirtyRects,
                 const void* buffer,
                 int width,
                 int height) override;

    // ★★★ OnAcceleratedPaint 的正确写法 ★★★
    virtual void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        const CefAcceleratedPaintInfo& info) override;

    bool InitD3D11();
    void CleanupD3D11();
    cv::Mat D3D11TextureToMat(void* share_handle, int width, int height);
    // 获取当前帧的 OpenCV Mat
    cv::Mat GetCurrentFrame();

private:
    int width_;
    int height_;
    int window_id_;
    cv::Mat current_frame_;
    std::mutex frame_mutex_;


    // ★★★ D3D11 设备 ★★★
    ID3D11Device* d3d_device_;
  
    ID3D11DeviceContext* d3d_context_;

    IMPLEMENT_REFCOUNTING(RenderHandler);
};

#endif // RENDER_HANDLER_H