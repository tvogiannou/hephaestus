# hephaestus
Simple toolset for setting up and experimenting with the [Vulkan API](https://www.khronos.org/vulkan/).


<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [hephaestus](#hephaestus)
  - [Overview](#overview)
  - [Requirements](#requirements)
  - [Adding the hephaestus lib to a project](#adding-the-hephaestus-lib-to-a-project)
    - [Build Instructions](#build-instructions)
      - [Windows/Linux](#windowslinux)
      - [Android](#android)
    - [CMake setup](#cmake-setup)
  - [Vulkan configuration](#vulkan-configuration)
  - [Vulkan initialization](#vulkan-initialization)
    - [Function Dispatcher](#function-dispatcher)
    - [Device Manager](#device-manager)
    - [Renderer](#renderer)
      - [Swap Chain Renderer](#swap-chain-renderer)
      - [Headless ("offscreen") Renderer](#headless-offscreen-renderer)
  - [Example Pipelines](#example-pipelines)
  - [Implementation Details](#implementation-details)
    - [Logging](#logging)
    - [Resource management](#resource-management)
    - [Synchronization](#synchronization)
  - [FAQ](#faq)

<!-- /code_chunk_output -->



## Overview
This repo contains a number of useful utilities that I have re-used in multiple occasions & platforms for setting up some very simple rendering with Vulkan. It is quite far from a "rendering framework" but simplifies some of the tedious setup needed when using Vulkan.

> Part of the code is based on the tutorials by [Pawel Lapinski](https://software.intel.com/en-us/articles/api-without-secrets-introduction-to-vulkan-preface) and [Alexander Overvoorde](https://vulkan-tutorial.com/Introduction), and of-course the [repo of examples by Sascha Willems](https://github.com/SaschaWillems/Vulkan).


Good starting points for using the library can be found in the [previewer app demo](https://github.com/tvogiannou/hephaestus/blob/master/demos/app/main.cpp) and the [headless renderer example](https://github.com/tvogiannou/hephaestus/blob/master/demos/headless/RenderOBJToImageFile.cpp).

## Requirements
The only external/third-party build dependency of the hephaestus library is the [Vulkan headers](https://github.com/KhronosGroup/Vulkan-Headers). The repo contains [version 1.2.333](https://github.com/tvogiannou/hephaestus/tree/work/external/vulkan-1.2.133) of the released hpp headers. 
The library is designed to work by dynamically loading the Vulkan library in the system, so another requirement is to have Vulkan installed. In Windows, the latest GPU drivers typically install Vulkan too; if that is not the case you can install the Vulkan SDK. In Linux there are packages available on most distributions, e.g. for Ubuntu `apt install libvulkan1 vulkan-utils` .


Even though not required, for development purposes it is highly recommended to download the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/). 



## Adding the hephaestus lib to a project
The most straightforward way to use the library is to add the source files to your build system. This involves defining a build configuration with all the hephaestus source files, e.g. for Visual Studio that would be another project, and making the Vulkan headers available. This allows for custom compiler options and targets.

To use hephaestus as a prebuilt library follow the [build instructions below]().

### Build Instructions
#### Windows/Linux

The code has been tested with the following compilers
- Visual Studio 2017
- GCC 5.4.0
- Clang 6.0.1

The repo has been built using [CMake](https://cmake.org/) and there are available scripts to use with it. The hephaestus library does not have any dependencies (other than Vulkan) and can be built with default options as is, for example

```bash
cd hephaestus                               # navigate to the hephaestus source directory
mkdir build && cd build                     # create build directory (in source)
cmake ..                                    # run the cmake to generate the platform specific build files
# cmake - G"Visual Studio 15 Win64" ..      # VS only, to generate x64 targets
cmake --build .                             # build the code (default config for VS)
```

#### Android
The library has been built and tested for arm64-v8a & armeabi-v7a using [Android NDK](https://developer.android.com/ndk) version 20.1.5948944. 
With CMake, hephaestus can be built using clang and the toolchain provided by the NDK

```bash
cmake  -DCMAKE_TOOLCHAIN_FILE=<NDK_LOCATION>/build/cmake/android.toolchain.cmake -DANDROID_TOOLCHAIN='clang' -DANDROID_ABI='arm64-v8a' -DANDROID_STL='c++_static' -DANDROID_PLATFORM=android-27 ..
```

This will built the hephaestus static lib for the target ABI and version. It can then be linked in a native Android Studio project.

### CMake setup
Instead of building the library, there are CMake scripts available that can be integrated directly to build systems designed on top of CMake. The following CMake commands describe different ways of including the hepheastus target in a CMake script.

```bash
# set this variable before processing the scripts to point to the Vulkan headers in the system
set(HEPHAESTUS_VULKAN_HEADERS_DIR <path-to-Vulkan-headers>)

# add the hephaestus static library target from an "out-of-source" directory
add_subdirectory(<path-to-hephaestus-source>
                 ${CMAKE_CURRENT_BINARY_DIR}/hephaestus-build
                 EXCLUDE_FROM_ALL)

# add the hephaestus static library target from an "in-source" directory 
# hephaestus.cmake is a simple script 
#include(<path-to-hephaestus-source>/hephaestus.cmake)

# include prebuilt lib
add_library(hephaestus STATIC IMPORTED)
set_target_properties(hephaestus PROPERTIES IMPORTED_LOCATION <path-to-hephaestus-lib>/hephaestus.a)
set_target_properties(hephaestus PROPERTIES INTERFACE_INCLUDE_DIRECTORIES <path-to-hephaestus-source>/hephaestus/include)

# link to the hephaestus target (static library)
target_link_libraries(myapp hephaestus)
```

## Vulkan configuration

The hephaestus library relies on a number of preprocessor definitions consumed by the Vulkan headers so code linking with the library **needs to include Vulkan only via the header provided `VulkanConfig.h`**.
The header is including the C++ API of Vulkan (vulkan.hpp) but the C API can also be used simply by including `VulkanConfig.h` before `vulkan.h`. See more details [below](#function-dispatcher).

## Vulkan initialization

### Function Dispatcher

The [Vulkan dispatcher](https://github.com/tvogiannou/hephaestus/blob/master/hephaestus/include/hephaestus/VulkanDispatcher.h) provides all the necessary setup for dynamically loading the Vulkan commands. The dispatcher also stores a global instance that is passed as the default dispatcher for functions to the vulkan.hpp header (so that there is no need to specify the dispatcher on every function call).

```c++
// example dispatcher initialization on Windows

hephaestus::VulkanDispatcher::ModuleType vulkanLib = LoadLibrary("vulkan-1.dll");   // load the Vulkan library

hephaestus::VulkanDispatcher::InitFromLibrary(vulkanLib);   // initialize the Vulkan loader
hephaestus::VulkanDispatcher::LoadGlobalFunctions();        // load any loader global functions
```
The "global" functions are the ones that do not refer to a (pre-created) Vulkan device and/or instance. The device & instance specific functions are loaded by the Device Manager as described in the following section.

The commands that will be loaded by the dispatcher are defined in [VulkanFunctions.inl](https://github.com/tvogiannou/hephaestus/blob/master/hephaestus/include/hephaestus/VulkanFunctions.inl).
The dispatcher header also exposes & resolves symbols for the same Vulkan commands from the vulkan.h header so that they can be used when is included instead of the hpp header.

> The resolved functions are declared in `hephaestus/include/hephaestus/VulkanFunctions.inl`.
> To add more functions: a) add their declaration in VulkanFunctions.inl using the utility macro and b) load them in the corresponding dispatcher method in `hephaestus/src/VulkanDispatcher.cpp`.

### Device Manager
The first actual object that needs to be created & initialized for hephaestus is a [device manager](https://github.com/tvogiannou/hephaestus/blob/master/hephaestus/include/hephaestus/VulkanDeviceManager.h). The device manager will internally create a Vulkan device and an instance, alongside any other other device only related data (e.g. queues), and will resolve device & instance specific Vulkan commands for the dispatcher. It can also optionally wire up Vulkan validation layer reporting to hephaestus logging.

The manager also holds the presentation surface (window) of the host platform, so it needs to be initialized with the native window handles. In case there is no window, the device manager will not initialize any presentation data.

```c++
// example device manager initialization on Windows using GLFW

hephaestus::VulkanDeviceManager deviceManager;

// set window handles
// if no window handles are defined then the device will be setup without a present surface & queue 
hephaestus::VulkanDeviceManager::PlatformWindowInfo platformWindowInfo;
{
    platformWindowInfo.instance = GetModuleHandle(NULL);
    platformWindowInfo.handle = glfwGetWin32Window(window.GetInfo().window);
}

// initialize device manager without validation layers
bool enableValidationLayers = false;
deviceManager.Init(platformWindowInfo, enableValidationLayers);
```

> NOTE: Working with multiple devices & instances is not currently supported.

### Renderer
A renderer is the main point of interaction between client code and the hephaestus library, taking care of most Vulkan setup for rendering a frame.
There are two types of available renderers, depending whether the device manager has been initialized with a window handle or not (see sections below).

 ```c++
// create & initialize a swap chain renderer
hephaestus::SwapChainRenderer renderer(deviceManager);
renderer.Init();
 ```

The renderer has API to record drawing commands from graphics "pipelines" during the update loop. Technically, a pipeline can be anything as long as it provides a method with the following signature to record the draw commands
```c++
void RecordDrawCommands(const VulkanUtils::FrameUpdateInfo& /*frameInfo*/) const;
```
where the `FrameUpdateInfo` struct is a container with necessary info for the recording commands
```c++
// Container with info for recording draw commands during a frame update
struct FrameUpdateInfo
{
    vk::CommandBuffer   drawCmdBuffer;
    vk::Framebuffer     framebuffer;
    vk::Image           image;
    vk::ImageView       view;
    vk::Extent2D        extent;
    vk::RenderPass      renderPass;
};
```
> Currently only a single render pass is supported for each of the available renderers.

The actual rendering is typically left to client code, however the library offers a number of [example pipelines](#example-pipelines) to user as reference.

#### Swap Chain Renderer
This renderer can handle the update for a present surface (window) via a [Vulkan swap chain](https://vulkan.lunarg.com/doc/view/1.0.26.0/linux/tutorial/html/05-init_swapchain.html). It is typically called during the frame update loop. 

```c++
// example frame update with the swap chain renderer 

SwapChainRenderer::RenderStats stats;
// call will block waiting on next available frame 
SwapChainRenderer::RenderStatus status = 
    SwapChainRenderer::RenderPipelines(renderer, stats, myPipeline);

// handle resizing (or any other error status)
if (status == SwapChainRenderer::RenderStatus::eRENDER_STATUS_RESIZE)
    // ...
```

#### Headless ("offscreen") Renderer
This renderer does not require a window and will render the resulting frame to an image buffer that can be retrieved later.

```C++
// render a frame using the headless renderer and copy it to a memory buffer

renderer.RenderPipeline(myPipeline);

// copy the rendered image to a buffer
uint32_t numChannels = 0u; // RGB/RGBA 
uint32_t width = 0u;
uint32_t height = 0u;
renderer.GetDstImageInfo(numChannels, width, height);

char* imgData = reinterpret_cast<char*>(malloc(numChannels * width * height));
renderer.GetDstImageData(imgData);
```

## Example Pipelines  
The library contains two pipelines that can be used as reference for writing more advanced ones:

- [TriMeshPipeline](https://github.com/tvogiannou/hephaestus/blob/master/hephaestus/include/hephaestus/TriMeshPipeline.h) for rendering simple triangle meshes.
- [PrimitivesPipeline](https://github.com/tvogiannou/hephaestus/blob/master/hephaestus/include/hephaestus/PrimitivesPipeline.h) for rendering lines.

> Pipelines are part of the code that is updated more often so it is discouraged to rely on their compatibility over time. 

The *TriMeshPipeline* is a generic example of a pipeline that can render textured triangle meshes of specific vertex format (already defined by the pipeline). Below is some sample code with an overview of the required operations for committing mesh data (mainly vertices & indices) that can be consumed by the pipeline.

```c++
// example (pseudo)code showing how to setup the TriMeshPipeline
// assumes there are accessible buffers with vertex & index data and a single texture

// setup shader DB
// this is a simple utility in the library to organize pre-compiled shaders
// it is basically an array of loaded shaders where the index is treated as an identifier
hephaestus::VulkanUtils::ShaderDB shaderDB;
{
    shaderDB.loadedShaders[ShaderType::eSHADER_VERTEX_PNTC] =
        hephaestus::VulkanUtils::CreateShaderModule(deviceManager, "../data/shaders/mesh/mesh.vert.spv");
    shaderDB.loadedShaders[ShaderType::eSHADER_FRAGMENT_PhongTexture] =
        hephaestus::VulkanUtils::CreateShaderModule(deviceManager, "../data/shaders/mesh/mesh.frag.spv");
}

hephaestus::TriMeshPipeline meshPipeline(renderer.GetDeviceManager());
{
    meshPipeline.CreateDescriptorPool();

    // allocate buffers for all data
    {
        const uint32_t vertexDataSize = ... // byte size of vertex data
        const uint32_t indexDataSize = ...  // byte size of triangle index data
        const uint32_t stageSize = ...      // big enough for updating buffers

        meshPipeline.CreateStageBuffer(stageSize);
        meshPipeline.CreateVertexBuffer(vertexDataSize);
        meshPipeline.CreateIndexBuffer(indexDataSize);
    }

    // create a new mesh in the pipeline
    TriMeshPipeline::MeshIDType meshId = outPipeline.MeshCreate();

    // update texture data
    outPipeline.MeshCreateTexture(meshId, width, height);
    VulkanUtils::TextureUpdateInfo textureUpdateInfo = ...         // setup some texture data
    meshPipeline.MeshSetTextureData(meshId, textureUpdateInfo);    // copy the data in the Vulkan buffers

    // upload vertex data
    VulkanUtils::BufferUpdateInfo vertexUpdateInfo = ...          // setup some vertex data
    meshPipeline.MeshSetVertexData(meshId, vertexUpdateInfo);     // copy the data in the Vulkan buffers

    // upload index data
    VulkanUtils::BufferUpdateInfo indexUpdateInfo = ...          // setup some index data
    meshPipeline.MeshSetIndexData(meshId, indexUpdateInfo);      // copy the data in the Vulkan buffers

    // utility for setting the shaders 
    hephaestus::VulkanGraphicsPipelineBase::ShaderParams shaderParams(shaderDB);
    {
        shaderParams.vertexShaderIndex = ShaderType::eSHADER_VERTEX_PNTC;               // index for the vertex shader
        shaderParams.fragmentShaderIndex = ShaderType::eSHADER_FRAGMENT_PhongTexture;   // index for the fragment shader
    }
    hephaestus::TriMeshPipeline::SetupParams params = {};                   // default pipeline params
    meshPipeline.SetupPipeline(renderPass, shaderParams, pipelineParams));  // setup the pipeline
}
```

## Implementation Details
### Logging
hephaestus uses a simple stateless [logger](https://github.com/tvogiannou/hephaestus/blob/master/hephaestus/include/hephaestus/Log.h) which simply forwards string messages to std output by default (and __android_log_print for Android), including any Vulkan validation layer messages if enabled. The logger can be completely disabled by re-building the lib with `HEPHAESTUS_DISABLE_LOGGER` defined, or redirected either by modifying the `Log.cpp` source file directly or using its API to set the log callback function.

```cpp
// custom log function
void CustomLog(const char* msg, hephaestus::Logger::MessageType type)
{
    // ...
}

hephaestus::Logger::SetCallback(CustomLog);
```

### Resource management
hephaestus uses throughout smart handles (`vk::UniqueHandle`) implemented in the vulkan.hpp which wrap around "naked" C types with some basic copying/moving semantics. This simplifies, to some extent, the release of Vulkan resources, but some extra care need to be taken in the order which handles are being released. 
To ease the trouble of managing these resources, most hephaestus types define a `Clear()` method for releasing the handles in a safe order (instead of relying in the default destructor behaviour). Note though that this usually requires most types to be non-copyable.

### Synchronization
As is, the library does not offer any extra layer of abstraction over Vulkan synchronization primitives. Every call that modifies device data in any way (e.g. copying data via a command buffer) will wait for the device to finish any previous job.  
This design decision follows the overall architecture of the library, i.e. keep it simple, as is targeted for experimental and relatively small Vulkan applications.

## FAQ
*(aka questions I keep asking myself...)*
- **How/why should I use this library?**  
The library is **not a rendering framework**. It mostly provides utilities for initializing & starting Vulkan in a system so that users can focus mostly on rendering centric features.
In terms of OpenGL it would be more akin to glut or glew than any other GL based rendering engine.

- **Why use the C++ header (vulkan.hpp)?**  
I do not have any strong opinions on the matter, just that when I initially started learning Vulkan it was noticeably easier to follow the code when using hpp types. There are some cases where I regretted doing so (in particular when dealing with the destructors of hpp types) and may change it in the future, but for now I can live with it.

- **Can I use the vulkan.h header?**  
Yes, see the sections on Vulkan [configuration](#vulkan-configuration) and the [dispatcher](#function-dispatcher). 

- **Can I enable/disable Vulkan exceptions?**  
Vulkan exceptions are disabled by default by defining  `VULKAN_HPP_NO_EXCEPTIONS` in VulkanConfig.h. To enable them back simply comment out the #define line.
Note, however, that the vulkan.hpp functions are declared with different return values when exceptions are enabled.

- **What is the need for a function dispatcher?**  
The dispatcher is a utility that follows the [official guideline](https://vulkan.lunarg.com/doc/view/1.1.70.1/windows/loader_and_layer_interface.html#user-content-best-application-performance-setup) from the Vulkan SDK on loading Vulkan commands dynamically for optimal stability & performance.
