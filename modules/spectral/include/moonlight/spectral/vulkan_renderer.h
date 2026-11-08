#pragma once

#include <memory>
#include <functional>
#include <cstdint>

#include <vulkan/vulkan.h>

struct SDL_Window;

namespace moonlight::atlas {
class Scene;
}

namespace moonlight::spectral {

struct ViewportCamera;

struct VulkanRenderContext {
    VkInstance instance{};
    VkPhysicalDevice physical_device{};
    VkDevice device{};
    VkQueue queue{};
    std::uint32_t queue_family{};
    VkRenderPass render_pass{};
    std::uint32_t image_count{};
    std::uint32_t min_image_count{};
};

class VulkanRenderer {
public:
    explicit VulkanRenderer(SDL_Window* window);
    ~VulkanRenderer();

    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;

    void render(
        const atlas::Scene& scene,
        const ViewportCamera& camera,
        const std::function<void(VkCommandBuffer)>& overlay = {}
    );
    [[nodiscard]] VulkanRenderContext context() const noexcept;
    void resize();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace moonlight::spectral
