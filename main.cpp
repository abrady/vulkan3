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

void createIndexBuffer(VkHandles vk, std::vector<uint32_t> indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory) {
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

struct Model {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    size_t numIndices;

    void draw(VkCommandBuffer commandBuffer) {
        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, numIndices, 1, 0, 0, 0);
    }
};

Model createModel(Vulkan &vulkan, std::vector<Vertex> vertices, std::vector<uint32_t> indices) {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    createVertexBuffer(vulkan.handles, vertices, vertexBuffer, vertexBufferMemory);
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    createIndexBuffer(vulkan.handles, indices, indexBuffer, indexBufferMemory);
    return {vertexBuffer, vertexBufferMemory, indexBuffer, indexBufferMemory, indices.size()};
}

void recordCommandBuffer(Vulkan &v, uint32_t frameIndex, std::vector<Model> &models) {
    VkCommandBuffer commandBuffer = v.render.beginRenderpass(v.present, frameIndex); {

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, v.render.graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) v.present.swapChainExtent.width;
        viewport.height = (float) v.present.swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = v.present.swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        for (int i = 0; i < models.size(); i++) {
            auto &model = models[i];
            model.draw(commandBuffer);
        }
    } vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void drawFrame(Vulkan &v, std::vector<Model> &models) {
    auto &h = v.handles;
    auto &r = v.render;

    uint32_t imageIndex = v.waitAndPrepForNextFrame();
    recordCommandBuffer(v, imageIndex, models);

    v.submitAndPresent(imageIndex);
}

int main(int, char**){
    Vulkan vulkan = createVulkan("Hello, Vulkan!", true, "shaders/vert/passthru.spv", "shaders/frag/passthru.spv");
    std::cout << "Hello, from Vulkan!\n";

    const std::vector<Vertex> vertices0 = {
        {{-0.75f, -0.75f, 0.f}, {1.0f, 0.0f, 0.0f}},
        {{0.25f, -0.75f, 0.f}, {0.0f, 1.0f, 0.0f}},
        {{0.25f, 0.25f, 0.f}, {0.0f, 0.0f, 1.0f}},
        {{-0.75f, 0.25f, 0.f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint32_t> indices0 = { 
        0, 1, 2, 2, 3, 0
    };

    const std::vector<Vertex> vertices1 = {
        {{-0.5f, -0.5f, .5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, .5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, .5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f, .5f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint32_t> indices1 = { 
        0, 1, 2, 2, 3, 0
    };
    std::vector<Model> models = {createModel(vulkan, vertices0, indices0), createModel(vulkan, vertices1, indices1)};

    while (!glfwWindowShouldClose(vulkan.handles.window)) {
        glfwPollEvents();
        drawFrame(vulkan, models);
    }

    return 0;
}
