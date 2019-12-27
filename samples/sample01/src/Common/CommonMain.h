
void render(RenderQueue* queue) {

    u32 count = queue->getCommandsCount();
    RenderCommand* commands = queue->getCommands();

    for (u32 i = 0; i < count; ++i) {
        switch (commands[i].command) {
        case SCENE_2D:
            commands[i].data.render2DScene.renderer->Render();
            break;
        }
    }

}



void resourceManagerMenu(const char* name, BitEngine::ResourceManager *resMng) {
    constexpr float TO_MB = 1.0 / (1024 * 1024);
    if (ImGui::TreeNode(name)) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "RAM: %.2f GPU: %.2f", resMng->getCurrentRamUsage()* TO_MB, resMng->getCurrentGPUMemoryUsage()*TO_MB);
        ImGui::TreePop();
    }
}
