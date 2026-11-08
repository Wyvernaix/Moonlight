#include <moonlight/spectral/vulkan_renderer.h>

#include <moonlight/atlas/atlas.h>
#include <moonlight/spectral/spectral.h>

#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace moonlight::spectral {

namespace {

struct Vertex {
    float position[3];
    float color[3];
};

struct PushConstants {
    float transform[16];
};

[[noreturn]] void fail(const char* message) {
    throw std::runtime_error(message);
}

void check(VkResult result, const char* message) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error(message);
    }
}

std::vector<char> read_shader(const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Unable to open compiled Vulkan shader");
    }
    const auto size = file.tellg();
    std::vector<char> bytes(static_cast<std::size_t>(size));
    file.seekg(0);
    file.read(bytes.data(), size);
    return bytes;
}

VkShaderModule shader_module(VkDevice device, const char* path) {
    const auto code = read_shader(path);
    VkShaderModuleCreateInfo info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    info.codeSize = code.size();
    info.pCode = reinterpret_cast<const std::uint32_t*>(code.data());
    VkShaderModule module{};
    check(vkCreateShaderModule(device, &info, nullptr, &module), "Shader module creation failed");
    return module;
}

std::array<float, 16> multiply(
    const std::array<float, 16>& a,
    const std::array<float, 16>& b
) {
    std::array<float, 16> result{};
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            for (int i = 0; i < 4; ++i) {
                result[column * 4 + row] += a[i * 4 + row] * b[column * 4 + i];
            }
        }
    }
    return result;
}

std::array<float, 16> camera_matrix(const ViewportCamera& camera, float aspect) {
    const float cos_yaw = std::cos(camera.yaw);
    const float sin_yaw = std::sin(camera.yaw);
    const float cos_pitch = std::cos(camera.pitch);
    const float sin_pitch = std::sin(camera.pitch);
    const float eye_x = camera.target.x + camera.distance * cos_pitch * cos_yaw;
    const float eye_y = camera.target.y + camera.distance * sin_pitch;
    const float eye_z = camera.target.z + camera.distance * cos_pitch * sin_yaw;
    const float fx = camera.target.x - eye_x;
    const float fy = camera.target.y - eye_y;
    const float fz = camera.target.z - eye_z;
    const float length = std::sqrt(fx * fx + fy * fy + fz * fz);
    const float zx = fx / length;
    const float zy = fy / length;
    const float zz = fz / length;
    const float xx = zy;
    const float xy = 0.0F;
    const float xz = -zx;
    const float x_length = std::sqrt(xx * xx + xz * xz);
    const float rx = xx / x_length;
    const float ry = xy;
    const float rz = xz / x_length;
    const float ux = ry * zz - rz * zy;
    const float uy = rz * zx - rx * zz;
    const float uz = rx * zy - ry * zx;
    const float fov = 1.0F;
    const float focal = 1.0F / std::tan(fov * 0.5F);
    const float near_plane = 0.1F;
    const float far_plane = 100.0F;
    return {
        focal / aspect * rx, focal / aspect * ry, focal / aspect * rz, 0.0F,
        focal * ux, focal * uy, focal * uz, 0.0F,
        -zx, -zy, -zz, 0.0F,
        -(rx * eye_x + ry * eye_y + rz * eye_z),
        -(ux * eye_x + uy * eye_y + uz * eye_z),
        near_plane / (near_plane - far_plane),
        1.0F,
    };
}

std::array<float, 16> projection(float aspect) {
    const float f = 1.0F / std::tan(0.5F);
    return {
        f / aspect, 0.0F, 0.0F, 0.0F,
        0.0F, -f, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, -1.0F,
        0.0F, 0.0F, 0.1F, 0.0F,
    };
}

void sphere_mesh(std::vector<Vertex>& vertices, std::vector<std::uint32_t>& indices) {
    constexpr std::uint32_t rings = 24;
    constexpr std::uint32_t segments = 32;
    for (std::uint32_t ring = 0; ring <= rings; ++ring) {
        const float v = static_cast<float>(ring) / static_cast<float>(rings);
        const float phi = v * 3.14159265F;
        for (std::uint32_t segment = 0; segment <= segments; ++segment) {
            const float u = static_cast<float>(segment) / static_cast<float>(segments);
            const float theta = u * 6.28318530F;
            const float x = std::sin(phi) * std::cos(theta);
            const float y = std::cos(phi);
            const float z = std::sin(phi) * std::sin(theta);
            vertices.push_back({{x, y, z}, {0.24F + 0.5F * x, 0.45F + 0.35F * y, 0.85F}});
        }
    }
    for (std::uint32_t ring = 0; ring < rings; ++ring) {
        for (std::uint32_t segment = 0; segment < segments; ++segment) {
            const auto row = segments + 1;
            const auto a = ring * row + segment;
            const auto b = a + row;
            indices.insert(indices.end(), {a, b, a + 1, b, b + 1, a + 1});
        }
    }
}

} // namespace

class VulkanRenderer::Impl {
public:
    explicit Impl(SDL_Window* window) : window_(window) {
        if (window_ == nullptr) {
            fail("Vulkan renderer requires an SDL window");
        }
        create_instance();
        if (!SDL_Vulkan_CreateSurface(window_, instance_, nullptr, &surface_)) {
            fail("SDL could not create the Vulkan surface");
        }
        select_device();
        create_swapchain();
        create_render_pass();
        create_pipeline();
        create_frame_resources();
    }

    ~Impl() {
        if (device_ != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(device_);
        }
        for (auto framebuffer : framebuffers_) vkDestroyFramebuffer(device_, framebuffer, nullptr);
        if (pipeline_ != VK_NULL_HANDLE) vkDestroyPipeline(device_, pipeline_, nullptr);
        if (pipeline_layout_ != VK_NULL_HANDLE) vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
        if (render_pass_ != VK_NULL_HANDLE) vkDestroyRenderPass(device_, render_pass_, nullptr);
        if (vertex_buffer_ != VK_NULL_HANDLE) vkDestroyBuffer(device_, vertex_buffer_, nullptr);
        if (index_buffer_ != VK_NULL_HANDLE) vkDestroyBuffer(device_, index_buffer_, nullptr);
        if (vertex_memory_ != VK_NULL_HANDLE) vkFreeMemory(device_, vertex_memory_, nullptr);
        if (index_memory_ != VK_NULL_HANDLE) vkFreeMemory(device_, index_memory_, nullptr);
        for (auto view : image_views_) vkDestroyImageView(device_, view, nullptr);
        if (swapchain_ != VK_NULL_HANDLE) vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        if (image_available_ != VK_NULL_HANDLE) vkDestroySemaphore(device_, image_available_, nullptr);
        if (render_finished_ != VK_NULL_HANDLE) vkDestroySemaphore(device_, render_finished_, nullptr);
        if (fence_ != VK_NULL_HANDLE) vkDestroyFence(device_, fence_, nullptr);
        if (device_ != VK_NULL_HANDLE) vkDestroyDevice(device_, nullptr);
        if (surface_ != VK_NULL_HANDLE) vkDestroySurfaceKHR(instance_, surface_, nullptr);
        if (instance_ != VK_NULL_HANDLE) vkDestroyInstance(instance_, nullptr);
    }

    void render(
        const atlas::Scene& scene,
        const ViewportCamera& camera,
        const std::function<void(VkCommandBuffer)>& overlay
    ) {
        if (scene.entities().empty()) return;
        check(vkWaitForFences(device_, 1, &fence_, VK_TRUE, std::numeric_limits<std::uint64_t>::max()),
              "Vulkan frame fence wait failed");
        check(vkResetFences(device_, 1, &fence_), "Vulkan frame fence reset failed");
        std::uint32_t image_index{};
        const auto acquire = vkAcquireNextImageKHR(
            device_, swapchain_, std::numeric_limits<std::uint64_t>::max(),
            image_available_, VK_NULL_HANDLE, &image_index
        );
        if (acquire == VK_ERROR_OUT_OF_DATE_KHR) { resize(); return; }
        check(acquire, "Vulkan swapchain image acquire failed");
        check(vkResetCommandBuffer(command_buffer_, 0), "Vulkan command buffer reset failed");
        VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        check(vkBeginCommandBuffer(command_buffer_, &begin), "Vulkan command buffer begin failed");
        const std::array<float, 16> model{
            1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 0.0F, scene.entities().front().transform.position.x,
            scene.entities().front().transform.position.y,
            scene.entities().front().transform.position.z, 1.0F
        };
        const auto view = camera_matrix(camera, extent_.width / static_cast<float>(extent_.height));
        const auto mvp = multiply(projection(extent_.width / static_cast<float>(extent_.height)),
                                  multiply(view, model));
        VkRenderPassBeginInfo pass{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        pass.renderPass = render_pass_;
        pass.framebuffer = framebuffers_[image_index];
        pass.renderArea.extent = extent_;
        const VkClearValue clear{{{0.035F, 0.045F, 0.075F, 1.0F}}};
        pass.clearValueCount = 1;
        pass.pClearValues = &clear;
        vkCmdBeginRenderPass(command_buffer_, &pass, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(command_buffer_, 0, 1, &vertex_buffer_, &offset);
        vkCmdBindIndexBuffer(command_buffer_, index_buffer_, 0, VK_INDEX_TYPE_UINT32);
        vkCmdPushConstants(command_buffer_, pipeline_layout_, VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(PushConstants), mvp.data());
        vkCmdDrawIndexed(command_buffer_, index_count_, 1, 0, 0, 0);
        if (overlay) {
            overlay(command_buffer_);
        }
        vkCmdEndRenderPass(command_buffer_);
        check(vkEndCommandBuffer(command_buffer_), "Vulkan command buffer end failed");
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &image_available_;
        submit.pWaitDstStageMask = &wait_stage;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &command_buffer_;
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &render_finished_;
        check(vkQueueSubmit(queue_, 1, &submit, fence_), "Vulkan queue submit failed");
        VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &render_finished_;
        present.swapchainCount = 1;
        present.pSwapchains = &swapchain_;
        present.pImageIndices = &image_index;
        const auto result = vkQueuePresentKHR(queue_, &present);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) resize();
        else check(result, "Vulkan present failed");
    }

    VulkanRenderContext context() const noexcept {
        return {
            .instance = instance_,
            .physical_device = physical_device_,
            .device = device_,
            .queue = queue_,
            .queue_family = queue_family_,
            .render_pass = render_pass_,
            .image_count = image_count_,
            .min_image_count = min_image_count_,
        };
    }

    void resize() {
        vkDeviceWaitIdle(device_);
        for (auto framebuffer : framebuffers_) vkDestroyFramebuffer(device_, framebuffer, nullptr);
        for (auto view : image_views_) vkDestroyImageView(device_, view, nullptr);
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        create_swapchain();
        create_framebuffers();
    }

private:
    SDL_Window* window_{};
    VkInstance instance_{};
    VkSurfaceKHR surface_{};
    VkPhysicalDevice physical_device_{};
    VkDevice device_{};
    VkQueue queue_{};
    std::uint32_t queue_family_{};
    VkSwapchainKHR swapchain_{};
    VkFormat format_{};
    VkExtent2D extent_{};
    std::vector<VkImageView> image_views_;
    std::vector<VkFramebuffer> framebuffers_;
    VkRenderPass render_pass_{};
    VkPipelineLayout pipeline_layout_{};
    VkPipeline pipeline_{};
    VkCommandPool command_pool_{};
    VkCommandBuffer command_buffer_{};
    VkSemaphore image_available_{};
    VkSemaphore render_finished_{};
    VkFence fence_{};
    VkBuffer vertex_buffer_{};
    VkDeviceMemory vertex_memory_{};
    VkBuffer index_buffer_{};
    VkDeviceMemory index_memory_{};
    std::uint32_t index_count_{};
    std::uint32_t image_count_{};
    std::uint32_t min_image_count_{};

    void create_instance() {
        Uint32 count{};
        const auto* extensions = SDL_Vulkan_GetInstanceExtensions(&count);
        if (extensions == nullptr) fail("SDL Vulkan instance extensions unavailable");
        VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        app.pApplicationName = "Moonlight Workbench";
        app.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        app.pEngineName = "Moonlight";
        app.engineVersion = VK_MAKE_VERSION(0, 1, 0);
        app.apiVersion = VK_API_VERSION_1_0;
        VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        info.pApplicationInfo = &app;
        info.enabledExtensionCount = count;
        info.ppEnabledExtensionNames = extensions;
        check(vkCreateInstance(&info, nullptr, &instance_), "Vulkan instance creation failed");
    }

    void select_device() {
        std::uint32_t count{};
        vkEnumeratePhysicalDevices(instance_, &count, nullptr);
        if (count == 0) fail("No Vulkan physical device is available");
        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(instance_, &count, devices.data());
        for (auto candidate : devices) {
            std::uint32_t queue_count{};
            vkGetPhysicalDeviceQueueFamilyProperties(candidate, &queue_count, nullptr);
            std::vector<VkQueueFamilyProperties> queues(queue_count);
            vkGetPhysicalDeviceQueueFamilyProperties(candidate, &queue_count, queues.data());
            for (std::uint32_t i = 0; i < queue_count; ++i) {
                VkBool32 present = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(candidate, i, surface_, &present);
                if ((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && present) {
                    physical_device_ = candidate;
                    queue_family_ = i;
                    break;
                }
            }
            if (physical_device_ != VK_NULL_HANDLE) break;
        }
        if (physical_device_ == VK_NULL_HANDLE) fail("No Vulkan graphics/present queue is available");
        const float priority = 1.0F;
        VkDeviceQueueCreateInfo queue{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queue.queueFamilyIndex = queue_family_;
        queue.queueCount = 1;
        queue.pQueuePriorities = &priority;
        const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        VkDeviceCreateInfo info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        info.queueCreateInfoCount = 1;
        info.pQueueCreateInfos = &queue;
        info.enabledExtensionCount = 1;
        info.ppEnabledExtensionNames = extensions;
        check(vkCreateDevice(physical_device_, &info, nullptr, &device_), "Vulkan device creation failed");
        vkGetDeviceQueue(device_, queue_family_, 0, &queue_);
    }

    void create_swapchain() {
        VkSurfaceCapabilitiesKHR caps{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &caps);
        std::uint32_t format_count{};
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, formats.data());
        format_ = formats.front().format;
        extent_ = caps.currentExtent;
        if (extent_.width == std::numeric_limits<std::uint32_t>::max()) {
            int width{}, height{};
            SDL_GetWindowSizeInPixels(window_, &width, &height);
            extent_ = {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
        }
        std::uint32_t present_count{};
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_, &present_count, nullptr);
        VkSwapchainCreateInfoKHR info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        info.surface = surface_;
        min_image_count_ = std::max(caps.minImageCount + 1U, 2U);
        if (caps.maxImageCount != 0) min_image_count_ = std::min(min_image_count_, caps.maxImageCount);
        info.minImageCount = min_image_count_;
        info.imageFormat = format_;
        info.imageColorSpace = formats.front().colorSpace;
        info.imageExtent = extent_;
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.preTransform = caps.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        info.clipped = VK_TRUE;
        check(vkCreateSwapchainKHR(device_, &info, nullptr, &swapchain_), "Vulkan swapchain creation failed");
        vkGetSwapchainImagesKHR(device_, swapchain_, &image_count_, nullptr);
        std::vector<VkImage> images(image_count_);
        vkGetSwapchainImagesKHR(device_, swapchain_, &image_count_, images.data());
        image_views_.resize(image_count_);
        for (std::uint32_t i = 0; i < image_count_; ++i) {
            VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            view.image = images[i];
            view.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view.format = format_;
            view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view.subresourceRange.levelCount = 1;
            view.subresourceRange.layerCount = 1;
            check(vkCreateImageView(device_, &view, nullptr, &image_views_[i]), "Vulkan image view creation failed");
        }
        create_framebuffers();
    }

    void create_render_pass() {
        VkAttachmentDescription color{};
        color.format = format_;
        color.samples = VK_SAMPLE_COUNT_1_BIT;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference reference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &reference;
        VkRenderPassCreateInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        info.attachmentCount = 1;
        info.pAttachments = &color;
        info.subpassCount = 1;
        info.pSubpasses = &subpass;
        check(vkCreateRenderPass(device_, &info, nullptr, &render_pass_), "Vulkan render pass creation failed");
    }

    void create_pipeline() {
        const auto vert = shader_module(device_, MOONLIGHT_SPECTRAL_VERT);
        const auto frag = shader_module(device_, MOONLIGHT_SPECTRAL_FRAG);
        VkPipelineShaderStageCreateInfo stages[2]{};
        stages[0] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vert, "main"};
        stages[1] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, frag, "main"};
        VkVertexInputBindingDescription binding{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
        VkVertexInputAttributeDescription attributes[] = {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3}};
        VkPipelineVertexInputStateCreateInfo input{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        input.vertexBindingDescriptionCount = 1; input.pVertexBindingDescriptions = &binding;
        input.vertexAttributeDescriptionCount = 2; input.pVertexAttributeDescriptions = attributes;
        VkPipelineInputAssemblyStateCreateInfo assembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkViewport viewport{0, 0, static_cast<float>(extent_.width), static_cast<float>(extent_.height), 0, 1};
        VkRect2D scissor{{0, 0}, extent_};
        VkPipelineViewportStateCreateInfo viewport_state{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewport_state.viewportCount = 1; viewport_state.pViewports = &viewport;
        viewport_state.scissorCount = 1; viewport_state.pScissors = &scissor;
        VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        raster.polygonMode = VK_POLYGON_MODE_FILL; raster.cullMode = VK_CULL_MODE_BACK_BIT; raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; raster.lineWidth = 1;
        VkPipelineMultisampleStateCreateInfo multisample{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        VkPipelineColorBlendAttachmentState blend{};
        blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        VkPipelineColorBlendStateCreateInfo color{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        color.attachmentCount = 1; color.pAttachments = &blend;
        VkPushConstantRange push{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants)};
        VkPipelineLayoutCreateInfo layout{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        layout.pushConstantRangeCount = 1; layout.pPushConstantRanges = &push;
        check(vkCreatePipelineLayout(device_, &layout, nullptr, &pipeline_layout_), "Vulkan pipeline layout creation failed");
        VkGraphicsPipelineCreateInfo info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        info.stageCount = 2; info.pStages = stages; info.pVertexInputState = &input; info.pInputAssemblyState = &assembly;
        info.pViewportState = &viewport_state; info.pRasterizationState = &raster; info.pMultisampleState = &multisample;
        info.pColorBlendState = &color; info.layout = pipeline_layout_; info.renderPass = render_pass_; info.subpass = 0;
        check(vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline_), "Vulkan graphics pipeline creation failed");
        vkDestroyShaderModule(device_, vert, nullptr);
        vkDestroyShaderModule(device_, frag, nullptr);
    }

    std::uint32_t memory_type(std::uint32_t bits, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memory{};
        vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory);
        for (std::uint32_t i = 0; i < memory.memoryTypeCount; ++i)
            if ((bits & (1U << i)) != 0 && (memory.memoryTypes[i].propertyFlags & properties) == properties) return i;
        fail("No compatible Vulkan memory type");
    }

    void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VkDeviceMemory& memory, const void* data) {
        VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        info.size = size; info.usage = usage; info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        check(vkCreateBuffer(device_, &info, nullptr, &buffer), "Vulkan buffer creation failed");
        VkMemoryRequirements requirements{};
        vkGetBufferMemoryRequirements(device_, buffer, &requirements);
        VkMemoryAllocateInfo allocation{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocation.allocationSize = requirements.size;
        allocation.memoryTypeIndex = memory_type(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        check(vkAllocateMemory(device_, &allocation, nullptr, &memory), "Vulkan buffer allocation failed");
        check(vkBindBufferMemory(device_, buffer, memory, 0), "Vulkan buffer bind failed");
        void* mapped{};
        vkMapMemory(device_, memory, 0, size, 0, &mapped);
        std::memcpy(mapped, data, static_cast<std::size_t>(size));
        vkUnmapMemory(device_, memory);
    }

    void create_frame_resources() {
        VkCommandPoolCreateInfo pool{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        pool.queueFamilyIndex = queue_family_;
        check(vkCreateCommandPool(device_, &pool, nullptr, &command_pool_), "Vulkan command pool creation failed");
        VkCommandBufferAllocateInfo allocation{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocation.commandPool = command_pool_; allocation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; allocation.commandBufferCount = 1;
        check(vkAllocateCommandBuffers(device_, &allocation, &command_buffer_), "Vulkan command buffer allocation failed");
        std::vector<Vertex> vertices; std::vector<std::uint32_t> indices; sphere_mesh(vertices, indices);
        index_count_ = static_cast<std::uint32_t>(indices.size());
        create_buffer(vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_buffer_, vertex_memory_, vertices.data());
        create_buffer(indices.size() * sizeof(std::uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_buffer_, index_memory_, indices.data());
        VkSemaphoreCreateInfo semaphore{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(device_, &semaphore, nullptr, &image_available_);
        vkCreateSemaphore(device_, &semaphore, nullptr, &render_finished_);
        VkFenceCreateInfo fence{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO}; fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        check(vkCreateFence(device_, &fence, nullptr, &fence_), "Vulkan fence creation failed");
    }

    void create_framebuffers() {
        if (render_pass_ == VK_NULL_HANDLE) return;
        framebuffers_.resize(image_views_.size());
        for (std::size_t i = 0; i < image_views_.size(); ++i) {
            VkFramebufferCreateInfo info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            info.renderPass = render_pass_; info.attachmentCount = 1; info.pAttachments = &image_views_[i];
            info.width = extent_.width; info.height = extent_.height; info.layers = 1;
            check(vkCreateFramebuffer(device_, &info, nullptr, &framebuffers_[i]), "Vulkan framebuffer creation failed");
        }
    }
};

VulkanRenderer::VulkanRenderer(SDL_Window* window) : impl_(std::make_unique<Impl>(window)) {}
VulkanRenderer::~VulkanRenderer() = default;
void VulkanRenderer::render(
    const atlas::Scene& scene,
    const ViewportCamera& camera,
    const std::function<void(VkCommandBuffer)>& overlay
) {
    impl_->render(scene, camera, overlay);
}
VulkanRenderContext VulkanRenderer::context() const noexcept { return impl_->context(); }
void VulkanRenderer::resize() { impl_->resize(); }

} // namespace moonlight::spectral
