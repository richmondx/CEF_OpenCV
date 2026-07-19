#include "RenderHandler.h"
#include <iostream>
#include <d3d11.h>
#include <d3d11_1.h> 

#include <dxgi1_2.h>

RenderHandler::RenderHandler(int width, int height)
    : width_(width), height_(height) {
    current_frame_ = cv::Mat(height, width, CV_8UC4, cv::Scalar(0, 0, 0, 255));

    InitD3D11();
    
}

bool RenderHandler::InitD3D11() {

    UINT create_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;


    D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // 使用默认适配器
        D3D_DRIVER_TYPE_HARDWARE,  // 硬件加速
        nullptr,                    // 无软件驱动
        create_flags,  // BGRA 支持
        feature_levels,
        ARRAYSIZE(feature_levels),                 // 使用默认特征级别
        D3D11_SDK_VERSION,
        &d3d_device_,
        nullptr,                    // 返回的特征级别
        &d3d_context_
    );

    if (FAILED(hr)) {
        std::cerr << "[RenderHandler " << window_id_
            << "] D3D11CreateDevice failed: " << std::hex << hr << std::dec << std::endl;
        return false;
    }

    std::cout << "[RenderHandler " << window_id_
        << "] D3D11 initialized successfully" << std::endl;
    return true;
}

void RenderHandler::CleanupD3D11() {
    if (d3d_context_) {
        d3d_context_->Release();
        d3d_context_ = nullptr;
    }
    if (d3d_device_) {
        d3d_device_->Release();
        d3d_device_ = nullptr;
    }
}

cv::Mat RenderHandler::D3D11TextureToMat(void* share_handle, int width, int height) {
    if (!d3d_device_ || !d3d_context_ || !share_handle) {
        return cv::Mat();
    }

    // 1. 从共享句柄获取 D3D11 纹理
    ID3D11Texture2D* shared_texture = nullptr;
    ID3D11Device1* device1 = nullptr;
    HRESULT hr = d3d_device_->QueryInterface(
        __uuidof(ID3D11Device1),
        (void**)&device1
    );
    hr = device1->OpenSharedResource1(share_handle,
        __uuidof(ID3D11Texture2D),
        (void**)&shared_texture);


    if (FAILED(hr)) {
        std::cerr << "[RenderHandler " << window_id_
            << "] OpenSharedResource failed" << std::endl;
        return cv::Mat();
    }

    // 2. 创建 staging texture（CPU 可读）
    D3D11_TEXTURE2D_DESC desc;
    shared_texture->GetDesc(&desc);
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.MiscFlags = 0;

    ID3D11Texture2D* staging_texture = nullptr;
    hr = d3d_device_->CreateTexture2D(&desc, nullptr, &staging_texture);
    if (FAILED(hr)) {
        shared_texture->Release();
        std::cerr << "[RenderHandler " << window_id_
            << "] Create staging texture failed" << std::endl;
        return cv::Mat();
    }

    // 3. 拷贝 GPU 纹理到 staging texture
    d3d_context_->CopyResource(staging_texture, shared_texture);

    // 4. 映射 staging texture 到 CPU 内存
    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = d3d_context_->Map(staging_texture, 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) {
        staging_texture->Release();
        shared_texture->Release();
        std::cerr << "[RenderHandler " << window_id_
            << "] Map staging texture failed" << std::endl;
        return cv::Mat();
    }

    // 5. 创建 OpenCV Mat（BGRA 格式）
    cv::Mat result(height, width, CV_8UC4);

    memcpy(result.data, mapped.pData, desc.Height * mapped.RowPitch);

    // 6. 清理
    d3d_context_->Unmap(staging_texture, 0);
    staging_texture->Release();
    shared_texture->Release();

    return result;
}

RenderHandler::~RenderHandler() {
    CleanupD3D11();
}

void RenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser,
                                CefRect& rect) {
    rect = CefRect(0, 0, width_, height_);
}

// ★★★ OnAcceleratedPaint 的正确实现 ★★★
void RenderHandler::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
    PaintElementType type,
    const RectList& dirtyRects,
    const CefAcceleratedPaintInfo& info) {
    // shared_handle 是一个 HANDLE，指向共享的 D3D11 纹理
    // 在 OSR 模式下，CEF 通过 GPU 渲染并将结果通过此句柄共享

    if (!info.shared_texture_handle) {
        return;
    }

    // 控制台输出调试信息（降低频率避免刷屏）
    static int frameCount = 0;
    frameCount++;

    // ★★★ 通过 shared_handle 获取纹理数据 ★★★
    // 需要 D3D11 设备来打开共享纹理
    // 这里只是示意，实际需要完整的 D3D11 上下文



    // 实际使用中，你需要：
    // 1. 创建一个 D3D11 设备
    // 2. 使用 shared_handle 打开共享纹理
    // 3. 将纹理数据拷贝到 CPU 可读的 staging texture
    // 4. 转换为 OpenCV Mat

    // 示例伪代码（需要 D3D11 支持）：
    
    ID3D11Device* device = d3d_device_;
    ID3D11Texture2D* sharedTexture = nullptr;
    HANDLE handle = static_cast<HANDLE>(info.shared_texture_handle);
    ID3D11Device1* device1 = nullptr;

    HRESULT hr = device->QueryInterface(
        __uuidof(ID3D11Device1),
        (void**)&device1
    );
     hr = device1->OpenSharedResource1(handle,
                                            __uuidof(ID3D11Texture2D),
                                            (void**)&sharedTexture);
    if (SUCCEEDED(hr)) {
        // 创建 staging texture
        D3D11_TEXTURE2D_DESC desc;
        sharedTexture->GetDesc(&desc);
        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.BindFlags = 0;
        desc.MiscFlags = 0;
        ID3D11Texture2D* stagingTexture = nullptr;
       hr= device->CreateTexture2D(&desc, nullptr, &stagingTexture);

        // 拷贝
        ID3D11DeviceContext* context = d3d_context_;
        context->CopyResource(stagingTexture, sharedTexture);

        // 映射到 CPU
        D3D11_MAPPED_SUBRESOURCE mapped;
        context->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapped);

        // 创建 OpenCV Mat

        cv::Mat mat(desc.Height, desc.Width, CV_8UC4);
        memcpy(mat.data, mapped.pData, desc.Height * mapped.RowPitch);

        context->Unmap(stagingTexture, 0);

        // 保存帧
        {
            
         


            cv::imshow("123", mat);

            if (frameCount % 300 == 0) {
                std::cout << "[OSR] process " << frameCount << " frame ("
                    << desc.Width << "x" << desc.Height << ")" << std::endl;


            }
            
        }

        stagingTexture->Release();
        sharedTexture->Release();
    }
    else
    {
        if (FAILED(hr)) {
            // 常见错误码：
            // E_INVALIDARG = 0x80070057  → handle 无效
            // DXGI_ERROR_INVALID_CALL = 0x887A0001 → 设备不匹配
            // E_OUTOFMEMORY = 0x8007000E → 内存不足

            LPVOID lpMsgBuf;
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                hr,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf,
                0, NULL
            );

            std::cerr << "Error: " << (LPCTSTR)lpMsgBuf << std::endl;
            LocalFree(lpMsgBuf);
        }
    }
    
}

void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser,
                            PaintElementType type,
                            const RectList& dirtyRects,
                            const void* buffer,
                            int width,
                            int height) {
    std::lock_guard<std::mutex> lock(frame_mutex_);

    // CEF OSR 默认输出 BGRA 格式
    cv::Mat bgra(height, width, CV_8UC4, (void*)buffer);

   

    // 保存处理后的图像
    current_frame_ = bgra.clone();

    // 控制台输出调试信息（降低频率避免刷屏）
    static int frameCount = 0;
    frameCount++;
    cv::imshow("123", current_frame_);
    
    if (frameCount % 300 == 0) {
        std::cout << "[OSR] process " << frameCount << " frame (" 
                  << width << "x" << height << ")" << std::endl;

       
    }
}

cv::Mat RenderHandler::GetCurrentFrame() {
    std::lock_guard<std::mutex> lock(frame_mutex_);
    return current_frame_.clone();
}