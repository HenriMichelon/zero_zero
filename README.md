# ZeroZero

ZeroZero is a 3D game engine based on Vulkan 1.3 & Jolt Physics 4.x

**Current state**


**Please note**
- This project was made for learning purpose
- This engine is not intended for general use, features are added as needed

**Vulkan extensions and third parties dependencies**
- Dynamic rendering (VK_KHR_dynamic_rendering)
- Shader object (VK_EXT_shader_object)
- volk https://github.com/zeux/volk


**Building tools needed**
- GCC/MINGW 11+ (C++ 23)
- CMake 3.22+
- Vulkan SDK 1.3+ 
- Git

**Building**
- On Microsoft Windows create a `.env.cmake` file with `set(VULKAN_SDK_PATH=c:\\path\\to\\vulkan\\version)` (for example `C:\\VulkanSDK\\1.3.280.0`)
- `cmake -B build -D CMAKE_BUILD_TYPE=Release` (add `-D GLFW_BUILD_WAYLAND=false` on Linux)
- `cmake --build build`

Released under the [MIT license](https://raw.githubusercontent.com/HenriMichelon/zero_zero/main/LICENSE.txt).
