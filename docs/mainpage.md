User documentation
===========================================================================

ZeroZero is an experimental 3D engine based on [Vulkan 1.3](https://www.vulkan.org/) and [Jolt 5.x](https://github.com/jrouwe/JoltPhysics) made in Modern C++ for learning purpose.

- [GitHub repository](https://github.com/HenriMichelon/zero_zero) 
- [Class list](annotated.html)

Released under the [MIT license](https://raw.githubusercontent.com/HenriMichelon/zero_zero/main/LICENSE.txt).

Building ZeroZero
---------------------------------------------------------------------------
### Install the mandatory  dependencies
- [Vulkan SDK 1.3.x+](https://vulkan.lunarg.com/)

### Install the build tools
For Microsoft Windows :
- [LLVM 19+](https://github.com/mstorsjo/llvm-mingw/releases) or [MSVC 19+ (VS 2022)](https://visualstudio.microsoft.com/fr/)
- [CMake 3.29+](https://cmake.org/download/) ([Visual Studio](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170)) with a [build tool](https://github.com/ninja-build/ninja/releases)

For Linux and macOS :
we are waiting for your push request for ZeroZero porting to Linux and macOS.

### Build the shared library
- `cmake -B build -G Ninja -D CMAKE_BUILD_TYPE=Release` (adapt for your build tool)
- `cmake --build build`

Examples
---------------------------------------------------------------------------
- [Template project](https://github.com/HenriMichelon/zero_zero_template)
- [Examples repository](https://github.com/HenriMichelon/zero_zero_examples)

Contact
---------------------------------------------------------------------------
For more information, contact the project maintainers at [GitHub project page](https://github.com/HenriMichelon/zero_zero)

Vulkan extensions and third parties dependencies used
---------------------------------------------------------------------------
- [Dynamic rendering](https://docs.vulkan.org/samples/latest/samples/extensions/dynamic_rendering/README.html) (VK_KHR_dynamic_rendering)
- [Shader object](https://docs.vulkan.org/samples/latest/samples/extensions/shader_object/README.html) (VK_EXT_shader_object)
- [volk](https://github.com/zeux/volk) to load Vulkan functions
- [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) for Vulkan buffers allocations
- [glm](https://github.com/g-truc/glm) for mathematics
- [stb](https://github.com/nothings/stb) for image loading and glyph rendering
- [fastgltf](https://github.com/spnda/fastgltf) for glTF scene loading
- [Jolt Physics](https://github.com/jrouwe/JoltPhysics) for the physics system

