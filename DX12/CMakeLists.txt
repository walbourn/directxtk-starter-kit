# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.21)

if(POLICY CMP0162)
   cmake_policy(SET CMP0162 NEW)
endif()

project (DirectXTK12StarterKit
  DESCRIPTION "DirectX Tool Kit for DX12 Starter Kit"
  LANGUAGES CXX)

option(ENABLE_CODE_ANALYSIS "Use Static Code Analysis on build" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(../build/CompilerAndLinker.cmake)

set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)

set(SHARED_SOURCE
    ../Shared/config.h
    ../Shared/Game.cpp
    ../Shared/Game.h
    ../Shared/Main.cpp
    )

set(SHARED_UTILITY
    ../Utility/GameComponents.h
    ../Utility/GameComponents.cpp
    ../Utility/GameServices.h
    ../Utility/GameServices.cpp
    ../Utility/StepTimer.h
    )

set(DX12_UTILITY
    d3dx12.h
    DeviceResources.cpp
    DeviceResources.h
    RenderTexture.h
    RenderTexture.cpp
    )

add_executable(${PROJECT_NAME} WIN32)

target_sources(${PROJECT_NAME}
    PRIVATE
        ${SHARED_SOURCE}
        ${SHARED_UTILITY}
        ${DX12_UTILITY}
        pch.h
)

target_include_directories(${PROJECT_NAME} PRIVATE ./ ../Shared/ ../Utility/)

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.h)

target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d12.lib dxgi.lib dxguid.lib uuid.lib
    kernel32.lib user32.lib
    comdlg32.lib advapi32.lib shell32.lib
    ole32.lib oleaut32.lib
    runtimeobject.lib
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /Wall /EHsc /GR)
endif()

find_package(directxmath CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectXMath)

find_package(directx-headers CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectX-Headers Microsoft::DirectX-Guids)

find_package(directx12-agility CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectX12-Core Microsoft::DirectX12-Layers)

find_package(directx-dxc CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectXShaderCompiler)

find_package(winpixevent CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::WinPixEventRuntime)

find_package(directxtk12 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectXTK12)

target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMPILER_DEFINES})
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_SWITCHES})
target_link_options(${PROJECT_NAME} PRIVATE ${LINKER_SWITCHES})

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|IntelLLVM")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wpedantic -Wextra
        "-Wno-c++98-compat" "-Wno-c++98-compat-pedantic"
        "-Wno-exit-time-destructors" "-Wno-global-constructors" "-Wno-language-extension-token"
        "-Wno-missing-prototypes" "-Wno-missing-variable-declarations" "-Wno-reserved-id-macro"
        "-Wno-float-equal" "-Wno-switch-enum" "-Wno-tautological-type-limit-compare")

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0)
        target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-unsafe-buffer-usage")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE
         "/wd4061" "/wd4365" "/wd4514" "/wd4571" "/wd4668" "/wd4710" "/wd4820" "/wd5039" "/wd5045"
         "/wd4265" "/wd4625" "/wd4626" "/wd4986" "/wd5204" "/wd5220")

    if(ENABLE_CODE_ANALYSIS)
        target_compile_options(${PROJECT_NAME} PRIVATE /analyze)
    endif()
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE BUILD_DX12)

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE UNICODE _WIN32_WINNT=0x0A00)
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E $<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>>,copy,true>
    $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
    )

if(TARGET Microsoft::DirectX12-Agility)
   file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/D3D12")
   add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Core,IMPORTED_LOCATION_RELEASE> $<TARGET_FILE_DIR:${PROJECT_NAME}>/D3D12
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Layers,IMPORTED_LOCATION_DEBUG> $<TARGET_FILE_DIR:${PROJECT_NAME}>/D3D12
      COMMAND ${CMAKE_COMMAND} -E rm -f $<TARGET_FILE_DIR:${PROJECT_NAME}>/D3D12Core.dll
      COMMAND ${CMAKE_COMMAND} -E rm -f $<TARGET_FILE_DIR:${PROJECT_NAME}>/d3d12SDKLayers.dll
      COMMAND_EXPAND_LISTS
      )
endif()
