# ZeroZero

ZeroZero is a 3D game engine based on [Vulkan 1.4](https://www.vulkan.org/) and [Jolt 5.x](https://github.com/jrouwe/JoltPhysics) made in Modern C++ for learning purpose.

[User documentation](https://henrimichelon.github.io/ZeroZero/)

Released under the [MIT license](https://github.com/HenriMichelon/zero_zero/blob/main/LICENSE.txt).

Features
---------------------------------------------------------------------------
Some of the features actually implemented or under construction :

- Vulkan based forward rendering system with optional depth pre-pass and multisampling.
- Scene tree with classic, object-oriented node system (inspired by [Godot](https://docs.godotengine.org), including the signal system).
- Physic system based on [Jolt Physics](https://github.com/jrouwe/JoltPhysics).
- PBR materials & shader.
- Shader-based materials.
- HDR tone mapping.
- Frustum culling.
- [In-game debug](https://henrimichelon.github.io/ZeroZero/md_004_debug_renderer.html)
- [Blender add-on](https://henrimichelon.github.io/ZeroZero/md_003_blender_add_on.html)
- JPEG/PNG and HDRi skybox.
- Directional lights, omni (point) lights and spotlights.
- Image based lighting for HDRi skybox.
- Cascaded shadow maps for directional lights.
- Cubemap shadow maps for omni lights.
- [JSON scene](https://henrimichelon.github.io/ZeroZero/md_002_file_formats.html) files.
- [glTF](https://henrimichelon.github.io/ZeroZero/md_002_file_formats.html) support.
- [ZRes](https://henrimichelon.github.io/ZeroZero/md_002_file_formats.html) binary file format for better loading time and decreased VRAM usage.
- UI framework
- Animations

Screenshots
---------------------------------------------------------------------------
Example level with tunnel, doors and rooms (with [Space Colony Modular Kit Bash](https://www.fab.com/listings/13206d95-b723-4ff3-a1ce-577d8259480b),
source code in the [ZeroZero examples](https://github.com/HenriMichelon/zero_zero_examples)):

[![Example level](https://img.youtube.com/vi/qW5M_U54oBU/0.jpg)](https://www.youtube.com/watch?v=qW5M_U54oBU)


Classic Sponza with one OmniLight:
![screenshot_sponza.png](https://henrimichelon.github.io/ZeroZero/screenshot_sponza.png)

[VR Room](https://sketchfab.com/3d-models/unreal-vr-room-01-f7c42add167045a2bcb88d921ea9fd61) with one DirectionalLight:
![screenshot_vr_room.png](https://henrimichelon.github.io/ZeroZero/screenshot_vr_room.png)

[Cerberus](https://sketchfab.com/3d-models/cerberusffvii-gun-model-by-andrew-maximov-d08c461f8217491892ad5dd29b436c90) with IBL :
![screenshot_cerberus.png](https://henrimichelon.github.io/ZeroZero/screenshot_cerberus.png)

[Collision objects debug](https://henrimichelon.github.io/ZeroZero/md_004_debug_renderer.html) :
![screenshot_debug.png](https://henrimichelon.github.io/ZeroZero/screenshot_debug.png)


Vulkan extensions and third parties dependencies used
---------------------------------------------------------------------------
- [Dynamic rendering](https://docs.vulkan.org/samples/latest/samples/extensions/dynamic_rendering/README.html) (VK_KHR_dynamic_rendering)
- [Shader object](https://docs.vulkan.org/samples/latest/samples/extensions/shader_object/README.html) (VK_EXT_shader_object)
- [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) for Vulkan buffers allocations
- [glm](https://github.com/g-truc/glm) for mathematics
- [stb](https://github.com/nothings/stb) for image loading and glyph rendering
- [fastgltf](https://github.com/spnda/fastgltf) for glTF scene loading
- [Jolt Physics](https://github.com/jrouwe/JoltPhysics) for the physics system
- [meshoptimizer](https://github.com/zeux/meshoptimizer) for meshes optimization
 