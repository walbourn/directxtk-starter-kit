# DirectX Tool Kit Starter Kit

This repo contains a 'starter kit' project the [DirectX Tool Kit for DirectX 11](https://github.com/Microsoft/DirectXTK) and [DirectX Tool Kit for DirectX 12](https://github.com/Microsoft/DirectXTK12). It's intended to showcase various parts of the tool kit as well as provide working examples of integration code.

## Visual Studio

The projects are for Visual Studio 2019. They make use of [NuGet](https://www.nuget.org/) for the required dependencies.

## CMake

The CMake projects make use of [VC++ Package Manager](https://vcpkg.io/) for required dependencies.

The following package is required for the *DirectX 11* version x86/x64 (in order to support Windows 7):

```
vcpkg install directxtk[xaudio2redist]
```

The following package is required for the *DirectX 11* version ARM64:

```
vcpkg install directxtk[xaudio2-9]
```

The following package is required for the *DirectX 12* version:

```
vcpkg install directxtk12
```

> The ``x86-windows``, ``x64-windows``, and ``arm64-windows`` triplets are supported.

The **CMakeSettings.json** file uses the environment variable ``VCPKG_ROOT`` for the ``cmakeToolchain`` variable to point to the proper location. It should be set to something like ``D:\vcpkg``

## Directories

* ``DX11\``: Contains the DirectX 11 project file, DeviceResources, RenderTexture, etc.

* ``DX12\``: Contains the DirectX 12 project file, DeviceResources, RenderTexture, etc.

* ``Shared\``: Contains the shared demonstration code including the 'main loop'.

* ``Utility\``: Contains the shared utility code like GameComponents and StepTimer.

## Notices

All source code for this package are subject to the terms of the [MIT License](http://opensource.org/licenses/MIT).

The content files used in this starter kit are from XNA Game Studio samples and starter kits published under the [Microsoft Public License (MS-PL)](https://opensource.org/licenses/MS-PL).

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft trademarks or logos is subject to and must follow [Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general). Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship. Any use of third-party trademarks or logos are subject to those third-party's policies.
