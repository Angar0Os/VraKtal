#ifndef VRAKTAL_CORE_RHI_WINDOW_VK_H
#define VRAKTAL_CORE_RHI_WINDOW_VK_H
#pragma once

#include <core/rhi/window.h>

#include <GLFW/glfw3.h>
#include <string>

struct GLFWwindow;

namespace core::rhi::vulkan
{
    class WindowVulkan final : public Window
    {
    public:
        WindowVulkan(uint32_t _width, uint32_t _height, const char* _title, bool _resizable = true);
        ~WindowVulkan() override;

        void PollEvents() override;
        bool ShouldClose() const override;

        std::pair<uint32_t, uint32_t> Size() const override;
        const char* Title() const override { return m_title.c_str(); }

        GLFWwindow* GlfwHandle() const { return m_window; }

    private:
        GLFWwindow* m_window;
        std::string m_title;
        uint32_t m_width = 0, m_height = 0;
    };
}

#endif //VRAKTAL_CORE_RHI_WINDOW_VK_H
