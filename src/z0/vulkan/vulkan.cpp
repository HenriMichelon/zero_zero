/*
* Copyright (c) 2024-2025 Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    HMODULE vulkanModule;
#endif
#include "z0/vulkan.h"

PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
PFN_vkAllocateMemory vkAllocateMemory;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
PFN_vkBindBufferMemory vkBindBufferMemory;
PFN_vkBindBufferMemory2 vkBindBufferMemory2;
PFN_vkBindImageMemory vkBindImageMemory;
PFN_vkBindImageMemory2 vkBindImageMemory2;
PFN_vkCmdBeginRendering vkCmdBeginRendering;
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
PFN_vkCmdBindPipeline vkCmdBindPipeline;
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
PFN_vkCmdBlitImage vkCmdBlitImage;
PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
PFN_vkCmdCopyImage vkCmdCopyImage;
PFN_vkCmdDispatch vkCmdDispatch;
PFN_vkCmdDraw vkCmdDraw;
PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
PFN_vkCmdEndRendering vkCmdEndRendering;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
PFN_vkCmdSetRasterizerDiscardEnable vkCmdSetRasterizerDiscardEnable;
PFN_vkCmdSetScissorWithCount vkCmdSetScissorWithCount;
PFN_vkCmdSetViewportWithCount vkCmdSetViewportWithCount;
PFN_vkCreateComputePipelines vkCreateComputePipelines;
PFN_vkCreateBuffer vkCreateBuffer;
PFN_vkCreateCommandPool vkCreateCommandPool;
PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
PFN_vkCreateDevice vkCreateDevice;
PFN_vkCreateFence vkCreateFence;
PFN_vkCreateImage vkCreateImage;
PFN_vkCreateImageView vkCreateImageView;
PFN_vkCreateInstance vkCreateInstance;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
PFN_vkCreateSampler vkCreateSampler;
PFN_vkCreateSemaphore vkCreateSemaphore;
PFN_vkCreateShaderModule vkCreateShaderModule;
PFN_vkDestroyBuffer vkDestroyBuffer;
PFN_vkDestroyCommandPool vkDestroyCommandPool;
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
PFN_vkDestroyDevice vkDestroyDevice;
PFN_vkDestroyFence vkDestroyFence;
PFN_vkDestroyImage vkDestroyImage;
PFN_vkDestroyImageView vkDestroyImageView;
PFN_vkDestroyInstance vkDestroyInstance;
PFN_vkDestroyPipeline vkDestroyPipeline;
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
PFN_vkDestroySampler vkDestroySampler;
PFN_vkDestroySemaphore vkDestroySemaphore;
PFN_vkDestroyShaderModule vkDestroyShaderModule;
PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
PFN_vkEndCommandBuffer vkEndCommandBuffer;
PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
PFN_vkFreeMemory vkFreeMemory;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
PFN_vkGetBufferMemoryRequirements2 vkGetBufferMemoryRequirements2;
PFN_vkGetDeviceBufferMemoryRequirements vkGetDeviceBufferMemoryRequirements;
PFN_vkGetDeviceImageMemoryRequirements vkGetDeviceImageMemoryRequirements;
PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
PFN_vkGetDeviceQueue vkGetDeviceQueue;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
PFN_vkGetImageMemoryRequirements2 vkGetImageMemoryRequirements2;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
PFN_vkGetPhysicalDeviceMemoryProperties2 vkGetPhysicalDeviceMemoryProperties2;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
PFN_vkCmdPushConstants vkCmdPushConstants;
PFN_vkQueueSubmit vkQueueSubmit;
PFN_vkQueueSubmit2 vkQueueSubmit2;
PFN_vkQueueWaitIdle vkQueueWaitIdle;
PFN_vkResetCommandBuffer vkResetCommandBuffer;
PFN_vkResetDescriptorPool vkResetDescriptorPool;
PFN_vkResetFences vkResetFences;
PFN_vkCmdSetColorWriteMaskEXT vkCmdSetColorWriteMaskEXT;
PFN_vkCmdSetCullMode vkCmdSetCullMode;
PFN_vkCmdSetDepthBiasEnable vkCmdSetDepthBiasEnable;
PFN_vkCmdSetDepthCompareOp vkCmdSetDepthCompareOp;
PFN_vkCmdSetFrontFace vkCmdSetFrontFace;
PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
PFN_vkCmdSetDepthTestEnable vkCmdSetDepthTestEnable;
PFN_vkCmdSetDepthWriteEnable vkCmdSetDepthWriteEnable;
PFN_vkCmdSetLogicOpEnableEXT vkCmdSetLogicOpEnableEXT;
PFN_vkCmdSetPrimitiveTopology vkCmdSetPrimitiveTopology;
PFN_vkCmdSetPrimitiveRestartEnable vkCmdSetPrimitiveRestartEnable;
PFN_vkCmdSetStencilTestEnable vkCmdSetStencilTestEnable;
PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
PFN_vkMapMemory vkMapMemory;
PFN_vkUnmapMemory vkUnmapMemory;
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
PFN_vkWaitForFences vkWaitForFences;

PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
PFN_vkQueuePresentKHR vkQueuePresentKHR;

PFN_vkCmdBindShadersEXT vkCmdBindShadersEXT;
PFN_vkCreateShadersEXT vkCreateShadersEXT;
PFN_vkDestroyShaderEXT vkDestroyShaderEXT;
PFN_vkCmdSetAlphaToCoverageEnableEXT vkCmdSetAlphaToCoverageEnableEXT;
PFN_vkCmdSetColorBlendEnableEXT vkCmdSetColorBlendEnableEXT;
PFN_vkCmdSetColorBlendEquationEXT vkCmdSetColorBlendEquationEXT;
PFN_vkCmdSetPolygonModeEXT vkCmdSetPolygonModeEXT;
PFN_vkCmdSetRasterizationSamplesEXT vkCmdSetRasterizationSamplesEXT;
PFN_vkCmdSetSampleMaskEXT vkCmdSetSampleMaskEXT;
PFN_vkCmdSetVertexInputEXT vkCmdSetVertexInputEXT;

void vulkanInitialize() {
#ifdef _WIN32
    vulkanModule = LoadLibraryA("vulkan-1.dll");
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)(void(*)(void))GetProcAddress(vulkanModule, "vkGetInstanceProcAddr");
#endif
    vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(nullptr, "vkCreateInstance");
    vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceLayerProperties");
}

void vulkanInitializeInstance(VkInstance instance) {
	vkCreateDevice = (PFN_vkCreateDevice)vkGetInstanceProcAddr(instance, "vkCreateDevice");
	vkDestroyInstance = (PFN_vkDestroyInstance)vkGetInstanceProcAddr(instance, "vkDestroyInstance");
	vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)vkGetInstanceProcAddr(instance, "vkEnumerateDeviceExtensionProperties");
	vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices");
	vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(instance, "vkGetDeviceProcAddr");
	vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures");
	vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFormatProperties");
	vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties");
	vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties");
	vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
	vkGetPhysicalDeviceMemoryProperties2 = (PFN_vkGetPhysicalDeviceMemoryProperties2)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties2");
	vkGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2");

	vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(instance, "vkDestroySurfaceKHR");
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
	vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
	vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
}

void vulkanInitializeDevice(VkDevice device) {
	vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)vkGetDeviceProcAddr(device, "vkAllocateCommandBuffers");
	vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)vkGetDeviceProcAddr(device, "vkAllocateDescriptorSets");
	vkAllocateMemory = (PFN_vkAllocateMemory)vkGetDeviceProcAddr(device, "vkAllocateMemory");
	vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)vkGetDeviceProcAddr(device, "vkBeginCommandBuffer");
	vkBindBufferMemory = (PFN_vkBindBufferMemory)vkGetDeviceProcAddr(device, "vkBindBufferMemory");
	vkBindImageMemory = (PFN_vkBindImageMemory)vkGetDeviceProcAddr(device, "vkBindImageMemory");
	vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)vkGetDeviceProcAddr(device, "vkCmdBindDescriptorSets");
	vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)vkGetDeviceProcAddr(device, "vkCmdBindIndexBuffer");
	vkCmdBindPipeline = (PFN_vkCmdBindPipeline)vkGetDeviceProcAddr(device, "vkCmdBindPipeline");
	vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)vkGetDeviceProcAddr(device, "vkCmdBindVertexBuffers");
	vkCmdBlitImage = (PFN_vkCmdBlitImage)vkGetDeviceProcAddr(device, "vkCmdBlitImage");
	vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)vkGetDeviceProcAddr(device, "vkCmdCopyBuffer");
	vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage)vkGetDeviceProcAddr(device, "vkCmdCopyBufferToImage");
	vkCmdCopyImage = (PFN_vkCmdCopyImage)vkGetDeviceProcAddr(device, "vkCmdCopyImage");
	vkCmdDispatch = (PFN_vkCmdDispatch)vkGetDeviceProcAddr(device, "vkCmdDispatch");
	vkCmdDraw = (PFN_vkCmdDraw)vkGetDeviceProcAddr(device, "vkCmdDraw");
	vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed)vkGetDeviceProcAddr(device, "vkCmdDrawIndexed");
	vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)vkGetDeviceProcAddr(device, "vkCmdPipelineBarrier");
	vkCmdPushConstants = (PFN_vkCmdPushConstants)vkGetDeviceProcAddr(device, "vkCmdPushConstants");
	vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias)vkGetDeviceProcAddr(device, "vkCmdSetDepthBias");
	vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth)vkGetDeviceProcAddr(device, "vkCmdSetLineWidth");
	vkCreateBuffer = (PFN_vkCreateBuffer)vkGetDeviceProcAddr(device, "vkCreateBuffer");
	vkCreateCommandPool = (PFN_vkCreateCommandPool)vkGetDeviceProcAddr(device, "vkCreateCommandPool");
	vkCreateComputePipelines = (PFN_vkCreateComputePipelines)vkGetDeviceProcAddr(device, "vkCreateComputePipelines");
	vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool)vkGetDeviceProcAddr(device, "vkCreateDescriptorPool");
	vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)vkGetDeviceProcAddr(device, "vkCreateDescriptorSetLayout");
	vkCreateFence = (PFN_vkCreateFence)vkGetDeviceProcAddr(device, "vkCreateFence");
	vkCreateImage = (PFN_vkCreateImage)vkGetDeviceProcAddr(device, "vkCreateImage");
	vkCreateImageView = (PFN_vkCreateImageView)vkGetDeviceProcAddr(device, "vkCreateImageView");
	vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout)vkGetDeviceProcAddr(device, "vkCreatePipelineLayout");
	vkCreateSampler = (PFN_vkCreateSampler)vkGetDeviceProcAddr(device, "vkCreateSampler");
	vkCreateSemaphore = (PFN_vkCreateSemaphore)vkGetDeviceProcAddr(device, "vkCreateSemaphore");
	vkCreateShaderModule = (PFN_vkCreateShaderModule)vkGetDeviceProcAddr(device, "vkCreateShaderModule");
	vkDestroyBuffer = (PFN_vkDestroyBuffer)vkGetDeviceProcAddr(device, "vkDestroyBuffer");
	vkDestroyCommandPool = (PFN_vkDestroyCommandPool)vkGetDeviceProcAddr(device, "vkDestroyCommandPool");
	vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool)vkGetDeviceProcAddr(device, "vkDestroyDescriptorPool");
	vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)vkGetDeviceProcAddr(device, "vkDestroyDescriptorSetLayout");
	vkDestroyDevice = (PFN_vkDestroyDevice)vkGetDeviceProcAddr(device, "vkDestroyDevice");
	vkDestroyFence = (PFN_vkDestroyFence)vkGetDeviceProcAddr(device, "vkDestroyFence");
	vkDestroyImage = (PFN_vkDestroyImage)vkGetDeviceProcAddr(device, "vkDestroyImage");
	vkDestroyImageView = (PFN_vkDestroyImageView)vkGetDeviceProcAddr(device, "vkDestroyImageView");
	vkDestroyPipeline = (PFN_vkDestroyPipeline)vkGetDeviceProcAddr(device, "vkDestroyPipeline");
	vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout)vkGetDeviceProcAddr(device, "vkDestroyPipelineLayout");
	vkDestroySampler = (PFN_vkDestroySampler)vkGetDeviceProcAddr(device, "vkDestroySampler");
	vkDestroySemaphore = (PFN_vkDestroySemaphore)vkGetDeviceProcAddr(device, "vkDestroySemaphore");
	vkDestroyShaderModule = (PFN_vkDestroyShaderModule)vkGetDeviceProcAddr(device, "vkDestroyShaderModule");
	vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle)vkGetDeviceProcAddr(device, "vkDeviceWaitIdle");
	vkEndCommandBuffer = (PFN_vkEndCommandBuffer)vkGetDeviceProcAddr(device, "vkEndCommandBuffer");
	vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)vkGetDeviceProcAddr(device, "vkFlushMappedMemoryRanges");
	vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets)vkGetDeviceProcAddr(device, "vkFreeDescriptorSets");
	vkFreeMemory = (PFN_vkFreeMemory)vkGetDeviceProcAddr(device, "vkFreeMemory");
	vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)vkGetDeviceProcAddr(device, "vkGetBufferMemoryRequirements");
	vkGetDeviceQueue = (PFN_vkGetDeviceQueue)vkGetDeviceProcAddr(device, "vkGetDeviceQueue");
	vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)vkGetDeviceProcAddr(device, "vkGetImageMemoryRequirements");
	vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)vkGetDeviceProcAddr(device, "vkInvalidateMappedMemoryRanges");
	vkMapMemory = (PFN_vkMapMemory)vkGetDeviceProcAddr(device, "vkMapMemory");
	vkQueueSubmit = (PFN_vkQueueSubmit)vkGetDeviceProcAddr(device, "vkQueueSubmit");
	vkQueueSubmit2 = (PFN_vkQueueSubmit2)vkGetDeviceProcAddr(device, "vkQueueSubmit2");
	vkQueueWaitIdle = (PFN_vkQueueWaitIdle)vkGetDeviceProcAddr(device, "vkQueueWaitIdle");
	vkResetCommandBuffer = (PFN_vkResetCommandBuffer)vkGetDeviceProcAddr(device, "vkResetCommandBuffer");
	vkResetDescriptorPool = (PFN_vkResetDescriptorPool)vkGetDeviceProcAddr(device, "vkResetDescriptorPool");
	vkResetFences = (PFN_vkResetFences)vkGetDeviceProcAddr(device, "vkResetFences");
	vkUnmapMemory = (PFN_vkUnmapMemory)vkGetDeviceProcAddr(device, "vkUnmapMemory");
	vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)vkGetDeviceProcAddr(device, "vkUpdateDescriptorSets");
	vkWaitForFences = (PFN_vkWaitForFences)vkGetDeviceProcAddr(device, "vkWaitForFences");
	vkBindBufferMemory2 = (PFN_vkBindBufferMemory2)vkGetDeviceProcAddr(device, "vkBindBufferMemory2");
	vkBindImageMemory2 = (PFN_vkBindImageMemory2)vkGetDeviceProcAddr(device, "vkBindImageMemory2");
	vkGetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2)vkGetDeviceProcAddr(device, "vkGetBufferMemoryRequirements2");
	vkGetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2)vkGetDeviceProcAddr(device, "vkGetImageMemoryRequirements2");
	vkCmdBeginRendering = (PFN_vkCmdBeginRendering)vkGetDeviceProcAddr(device, "vkCmdBeginRendering");
	vkCmdEndRendering = (PFN_vkCmdEndRendering)vkGetDeviceProcAddr(device, "vkCmdEndRendering");
	vkCmdSetCullMode = (PFN_vkCmdSetCullMode)vkGetDeviceProcAddr(device, "vkCmdSetCullMode");
	vkCmdSetDepthBiasEnable = (PFN_vkCmdSetDepthBiasEnable)vkGetDeviceProcAddr(device, "vkCmdSetDepthBiasEnable");
	vkCmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp)vkGetDeviceProcAddr(device, "vkCmdSetDepthCompareOp");
	vkCmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable)vkGetDeviceProcAddr(device, "vkCmdSetDepthTestEnable");
	vkCmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable)vkGetDeviceProcAddr(device, "vkCmdSetDepthWriteEnable");
	vkCmdSetFrontFace = (PFN_vkCmdSetFrontFace)vkGetDeviceProcAddr(device, "vkCmdSetFrontFace");
	vkCmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable)vkGetDeviceProcAddr(device, "vkCmdSetPrimitiveRestartEnable");
	vkCmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology)vkGetDeviceProcAddr(device, "vkCmdSetPrimitiveTopology");
	vkCmdSetRasterizerDiscardEnable = (PFN_vkCmdSetRasterizerDiscardEnable)vkGetDeviceProcAddr(device, "vkCmdSetRasterizerDiscardEnable");
	vkCmdSetScissorWithCount = (PFN_vkCmdSetScissorWithCount)vkGetDeviceProcAddr(device, "vkCmdSetScissorWithCount");
	vkCmdSetStencilTestEnable = (PFN_vkCmdSetStencilTestEnable)vkGetDeviceProcAddr(device, "vkCmdSetStencilTestEnable");
	vkCmdSetViewportWithCount = (PFN_vkCmdSetViewportWithCount)vkGetDeviceProcAddr(device, "vkCmdSetViewportWithCount");
	vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)vkGetDeviceProcAddr(device, "vkGetDeviceBufferMemoryRequirements");
	vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)vkGetDeviceProcAddr(device, "vkGetDeviceImageMemoryRequirements");

	vkCmdBindShadersEXT = (PFN_vkCmdBindShadersEXT)vkGetDeviceProcAddr(device, "vkCmdBindShadersEXT");
	vkCreateShadersEXT = (PFN_vkCreateShadersEXT)vkGetDeviceProcAddr(device, "vkCreateShadersEXT");
	vkDestroyShaderEXT = (PFN_vkDestroyShaderEXT)vkGetDeviceProcAddr(device, "vkDestroyShaderEXT");

    vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR");
	vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR");
	vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)vkGetDeviceProcAddr(device, "vkDestroySwapchainKHR");
	vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR");
	vkQueuePresentKHR = (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(device, "vkQueuePresentKHR");
    
	vkCmdSetAlphaToCoverageEnableEXT = (PFN_vkCmdSetAlphaToCoverageEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetAlphaToCoverageEnableEXT");
	vkCmdSetColorBlendEnableEXT = (PFN_vkCmdSetColorBlendEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetColorBlendEnableEXT");
	vkCmdSetColorBlendEquationEXT = (PFN_vkCmdSetColorBlendEquationEXT)vkGetDeviceProcAddr(device, "vkCmdSetColorBlendEquationEXT");
	vkCmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT)vkGetDeviceProcAddr(device, "vkCmdSetColorWriteMaskEXT");
	vkCmdSetLogicOpEnableEXT = (PFN_vkCmdSetLogicOpEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetLogicOpEnableEXT");
	vkCmdSetPolygonModeEXT = (PFN_vkCmdSetPolygonModeEXT)vkGetDeviceProcAddr(device, "vkCmdSetPolygonModeEXT");
	vkCmdSetRasterizationSamplesEXT = (PFN_vkCmdSetRasterizationSamplesEXT)vkGetDeviceProcAddr(device, "vkCmdSetRasterizationSamplesEXT");
	vkCmdSetSampleMaskEXT = (PFN_vkCmdSetSampleMaskEXT)vkGetDeviceProcAddr(device, "vkCmdSetSampleMaskEXT");
	vkCmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT)vkGetDeviceProcAddr(device, "vkCmdSetVertexInputEXT");
}

void vulkanFinalize() {
    FreeLibrary(vulkanModule);
}