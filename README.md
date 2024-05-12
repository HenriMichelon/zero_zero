# ZeroZero

ZeroZero is a 3D engine based on Vulkan 1.3 & Jolt Physics 4.x

**Vulkan extensions and third parties dependencies**
- Dynamic rendering (VK_KHR_dynamic_rendering)
- Shader object (VK_EXT_shader_object)
- volk https://github.com/zeux/volk
- VulkanMemoryAllocator https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
- GLM https://github.com/g-truc/glm
- stb https://github.com/nothings/stb
- fastgltf https://github.com/spnda/fastgltf

**Building tools needed**
- GCC/MINGW 11+ (C++ 23)
- CMake 3.22+
- Vulkan SDK 1.3+ 
- Git

**Building**
- If you have multiple version of the Vulkan SDK you can create a `.env.cmake` file with `set(VULKAN_SDK_PATH=c:\\path\\to\\vulkan\\version)` (for example `C:\\VulkanSDK\\1.3.280.0`) to select a particular Vulkan version
- `cmake -B build -D CMAKE_BUILD_TYPE=Release`
- `cmake --build build`

Released under the [MIT license](https://raw.githubusercontent.com/HenriMichelon/zero_zero/main/LICENSE.txt).
