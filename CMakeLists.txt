cmake_minimum_required(VERSION 3.10)
project(CEF_OpenCV_OSR LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# 1. 手动指定 CEF 路径（请修改为你本机的实际路径！）
# ============================================================
# ★★★ 请将下面的路径改为你电脑上 CEF 150 的实际路径 ★★★
set(CEF_ROOT "E:/CEF_OpenCV_Demo/cef_binary_150.0.11")

# CEF 头文件
set(CEF_INCLUDE_DIR ${CEF_ROOT})

# CEF 库文件（根据平台选择）


 set(CMAKE_BUILD_TYPE Debug)

if(WIN32)
    set(CEF_LIBRARY_PATH ${CEF_ROOT}/${CMAKE_BUILD_TYPE})
    set(CEF_LIBRARIES
        ${CEF_LIBRARY_PATH}/libcef.lib
        ${CEF_LIBRARY_PATH}/libcef_dll_wrapper.lib
    )
elseif(APPLE)
    set(CEF_LIBRARY_PATH ${CEF_ROOT}/${CMAKE_BUILD_TYPE})
    set(CEF_LIBRARIES
        ${CEF_LIBRARY_PATH}/libcef.dylib
        ${CEF_LIBRARY_PATH}/libcef_dll_wrapper.a
    )
else()
    set(CEF_LIBRARY_PATH ${CEF_ROOT}/${CMAKE_BUILD_TYPE})
    set(CEF_LIBRARIES
        ${CEF_LIBRARY_PATH}/libcef.so
        ${CEF_LIBRARY_PATH}/libcef_dll_wrapper.a
    )
endif()

# CEF 额外依赖
if(WIN32)
    set(CEF_EXTRA_LIBS
        comctl32.lib
        rpcrt4.lib
        shlwapi.lib
        ws2_32.lib
        dbghelp.lib
        psapi.lib
        version.lib
        winmm.lib
        imm32.lib
    )
elseif(LINUX)
    set(CEF_EXTRA_LIBS
        X11 Xcomposite Xcursor Xdamage Xext Xfixes Xi Xrandr Xrender Xtst
        atk-bridge-2.0 atspi cairo cups dbus-1 expat fontconfig freetype
        gdk_pixbuf-2.0 gio-2.0 glib-2.0 gmodule-2.0 gobject-2.0 gtk-3
        pango-1.0 pangocairo-1.0 pangoft2-1.0 pthread xkbcommon xkbcommon-x11 z
    )
endif()

# ============================================================
# 2. 手动指定 OpenCV 路径（请修改为你本机的实际路径！）
# ============================================================
# ★★★ 请将下面的路径改为你电脑上 OpenCV 的实际路径 ★★★
set(OPENCV_DIR "E:/CEF_OpenCV_Demo/opencv")

# OpenCV 头文件
set(OpenCV_INCLUDE_DIR ${OPENCV_DIR}/include)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIB_POSTFIX "d")
else()
    set(LIB_POSTFIX "")
endif()

# OpenCV 库文件
if(WIN32)
    set(OpenCV_LIB_PATH ${OPENCV_DIR}/lib)
    set(OpenCV_LIBS
        ${OpenCV_LIB_PATH}/opencv_world490${LIB_POSTFIX}.lib
    )
else()
    set(OpenCV_LIB_PATH ${OPENCV_DIR}/lib)
    set(OpenCV_LIBS
        ${OpenCV_LIB_PATH}/libopencv_core.so
        ${OpenCV_LIB_PATH}/libopencv_imgproc.so
        ${OpenCV_LIB_PATH}/libopencv_highgui.so
        ${OpenCV_LIB_PATH}/libopencv_imgcodecs.so
    )
endif()

# ============================================================
# 3. 源码列表
# ============================================================
set(SOURCES
    main.cpp
    MyApp.cpp
    MyClient.cpp
    RenderHandler.cpp
    V8Handler.cpp
)


# ============================================================
# 4. 构建可执行文件
# ============================================================
add_executable(${PROJECT_NAME} ${SOURCES})

set(HEADERS
    MyApp.h
    MyClient.h
   RenderHandler.h
   V8Handler.h
)
source_group("Header Files" FILES ${HEADERS}) 

target_sources(${PROJECT_NAME} PRIVATE ${HEADERS} ${SOURCES})
# 包含目录
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CEF_INCLUDE_DIR}
        ${OpenCV_INCLUDE_DIR}
)




# 链接库
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${CEF_LIBRARIES}
        ${CEF_EXTRA_LIBS}
        ${OpenCV_LIBS}
        gdi32.lib
        d3d11.lib     
        dxgi.lib
)

# ============================================================
# 5. 复制 CEF 运行时文件到输出目录
# ============================================================
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_ROOT}/${CMAKE_BUILD_TYPE}/"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
        COMMENT "Copying CEF runtime files..."
    )

     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${OPENCV_DIR}/bin/"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
        COMMENT "Copying opencv runtime files..."
    )

     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_ROOT}/Resources/"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
        COMMENT "Copying CEF Resources files..."
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/html"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/html"
    COMMENT "Copying HTML files..."
)
else()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_ROOT}/${CMAKE_BUILD_TYPE}/"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
        COMMENT "Copying CEF runtime files..."
    )
endif()

# CEF 编译定义
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        
        PSAPI_VERSION=1
         
         NOMINMAX
         _UNICODE
        UNICODE
)

#md 换成mt
if (MSVC)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif(MSVC)



# ============================================================
# 7. 打印配置摘要
# ============================================================
message(STATUS "========== Build Configuration ==========")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "CEF Path: ${CEF_ROOT}")
message(STATUS "CEF Libraries: ${CEF_LIBRARIES}")
message(STATUS "OpenCV Path: ${OPENCV_DIR}")
message(STATUS "OpenCV Include: ${OpenCV_INCLUDE_DIR}")
message(STATUS "OpenCV Libraries: ${OpenCV_LIBS}")
message(STATUS "Sources: ${SOURCES}")
message(STATUS "========================================")