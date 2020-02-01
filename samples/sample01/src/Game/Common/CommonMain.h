#pragma once

#include <imgui.h>

void resourceManagerMenu(const char* name, BitEngine::ResourceManager *resMng) {
    constexpr float TO_MB = 1.0 / (1024 * 1024);
    if (ImGui::TreeNode(name)) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "RAM: %.2f GPU: %.2f", resMng->getCurrentRamUsage()* TO_MB, resMng->getCurrentGPUMemoryUsage()*TO_MB);
        ImGui::TreePop();
    }
}