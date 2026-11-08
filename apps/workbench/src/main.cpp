#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <algorithm>
#include <exception>
#include <stdexcept>

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
#include "moonlight/atlas/atlas.h"
#include "moonlight/core/runtime.h"
#include "moonlight/spectral/spectral.h"
#include "moonlight/spectral/vulkan_renderer.h"

namespace {

constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 720;

} // namespace

int main(int, char**) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL initialization failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Moonlight Editor",
        kWindowWidth,
        kWindowHeight,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );
    if (window == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    auto scene = moonlight::atlas::create_default_scene();
    moonlight::spectral::ViewportCamera camera;
    bool running = true;
    try {
        moonlight::spectral::VulkanRenderer renderer{window};
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplSDL3_InitForVulkan(window);
        const auto vulkan = renderer.context();
        ImGui_ImplVulkan_InitInfo imgui_vulkan{};
        imgui_vulkan.ApiVersion = VK_API_VERSION_1_0;
        imgui_vulkan.Instance = vulkan.instance;
        imgui_vulkan.PhysicalDevice = vulkan.physical_device;
        imgui_vulkan.Device = vulkan.device;
        imgui_vulkan.QueueFamily = vulkan.queue_family;
        imgui_vulkan.Queue = vulkan.queue;
        imgui_vulkan.DescriptorPoolSize = 1000;
        imgui_vulkan.MinImageCount = vulkan.min_image_count;
        imgui_vulkan.ImageCount = vulkan.image_count;
        imgui_vulkan.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        imgui_vulkan.RenderPass = vulkan.render_pass;
        if (!ImGui_ImplVulkan_Init(&imgui_vulkan)) {
            throw std::runtime_error("Dear ImGui Vulkan initialization failed");
        }
        while (running) {
            SDL_Event event{};
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL3_ProcessEvent(&event);
                if (event.type == SDL_EVENT_QUIT) running = false;
                if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) renderer.resize();
            }
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
            ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize);
            ImGui::Begin(
                "Scene View",
                nullptr,
                ImGuiWindowFlags_NoBackground
                | ImGuiWindowFlags_NoBringToFrontOnFocus
            );
            ImGui::Text("Atlas scene: %s", scene.name().c_str());
            ImGui::Text("Spectral Vulkan sphere renderer");
            ImGui::Separator();
            ImGui::TextWrapped(
                "The sphere is rendered by Spectral from Atlas scene data. "
                "This window is dockable and can share the Workbench with tools."
            );
            ImGui::End();

            ImGui::Begin("Hierarchy");
            for (const auto& entity : scene.entities()) {
                ImGui::Selectable(entity.name.c_str());
            }
            ImGui::End();

            ImGui::Begin("Inspector");
            if (!scene.entities().empty()) {
                const auto& entity = scene.entities().front();
                ImGui::Text("Selected: %s", entity.name.c_str());
                ImGui::Text("Entity ID: %u", entity.id);
                ImGui::Text(
                    "Position: %.2f, %.2f, %.2f",
                    entity.transform.position.x,
                    entity.transform.position.y,
                    entity.transform.position.z
                );
                ImGui::Text("Geometry: Sphere");
                ImGui::Text("Radius: %.2f m", entity.geometry_radius);
            }
            ImGui::End();

            ImGui::Render();
            renderer.render(
                scene,
                camera,
                [](VkCommandBuffer command_buffer) {
                    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
                }
            );
        }
        vkDeviceWaitIdle(vulkan.device);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    } catch (const std::exception& error) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Vulkan renderer failed: %s", error.what());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
