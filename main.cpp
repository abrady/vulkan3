#include <cstring>
#include <iostream>
#include "Vulkan.h"
#include <GLFW/glfw3.h>


void createVertexBuffer(VkHandles vk, std::vector<Vertex> vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory) {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    vk.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(vk.device, stagingBufferMemory, 0, bufferSize, 0, &data); {
        memcpy(data, vertices.data(), (size_t) bufferSize);
    } vkUnmapMemory(vk.device, stagingBufferMemory);

    vk.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    vk.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(vk.device, stagingBuffer, nullptr);
    vkFreeMemory(vk.device, stagingBufferMemory, nullptr);
}

void createIndexBuffer(VkHandles vk, std::vector<uint16_t> indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory) {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    vk.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(vk.device, stagingBufferMemory, 0, bufferSize, 0, &data); {
        memcpy(data, indices.data(), (size_t) bufferSize);
    } vkUnmapMemory(vk.device, stagingBufferMemory);

    vk.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    vk.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(vk.device, stagingBuffer, nullptr);
    vkFreeMemory(vk.device, stagingBufferMemory, nullptr);
}

void recordCommandBuffer(Vulkan &v, VkFramebuffer framebuffer, VkBuffer vertexBuffer, VkBuffer indexBuffer, size_t numVerts) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkFrame cf = v.render.getCF();
    if (vkBeginCommandBuffer(cf.commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = v.render.renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = v.present.swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(cf.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); {
        vkCmdBindPipeline(cf.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, v.render.graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) v.present.swapChainExtent.width;
        viewport.height = (float) v.present.swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cf.commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = v.present.swapChainExtent;
        vkCmdSetScissor(cf.commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cf.commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(cf.commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(cf.commandBuffer, static_cast<uint32_t>(numVerts), 1, 0, 0, 0);

    } vkCmdEndRenderPass(cf.commandBuffer);

    if (vkEndCommandBuffer(cf.commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void recordCommandBuffer(Vulkan &v, VkFramebuffer framebuffer, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t numIndices) {
    auto commandBuffer = v.render.getCF().commandBuffer;
    auto swapChainExtent = v.present.swapChainExtent;
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = v.render.renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, v.render.graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) swapChainExtent.width;
        viewport.height = (float) swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(commandBuffer, numIndices, 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void drawFrame(Vulkan &v, VkBuffer vertexBuffer, VkBuffer indexBuffer, size_t numIndices) {
    auto &h = v.handles;
    auto &r = v.render;

    uint32_t imageIndex = v.waitAndPrepForNextFrame();
    VkFramebuffer framebuffer = v.render.swapChainFramebuffers[imageIndex];

    recordCommandBuffer(v, framebuffer, vertexBuffer, indexBuffer, numIndices);

    v.submitAndPresent(imageIndex);
}

int main(int, char**){
    Vulkan vulkan = createVulkan("Hello, Vulkan!", true, "shaders/vertshaders/passthru.spf", "shaders/fragshaders/passthru.spf");
    std::cout << "Hello, from Vulkan!\n";

    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    createVertexBuffer(vulkan.handles, vertices, vertexBuffer, vertexBufferMemory);
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    createIndexBuffer(vulkan.handles, indices, indexBuffer, indexBufferMemory);

    while (!glfwWindowShouldClose(vulkan.handles.window)) {
        glfwPollEvents();
        drawFrame(vulkan, vertexBuffer, indexBuffer, indices.size());
    }

    return 0;
}