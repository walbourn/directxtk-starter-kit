//
// config.h
//

#pragma once

#if !defined(BUILD_DX11) && !defined(BUILD_DX12)
#error You must define BUILD_DX11 or BUILD_DX12
#endif

#if defined(BUILD_DX11) && defined(BUILD_DX12)
#error You must define only one of BUILD_DX11 or BUILD_DX12
#endif

// Default to fullscreen at startup
// #define DEFAULT_FULLSCREEN
