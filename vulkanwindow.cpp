#include "vulkanwindow.h"
#include "looprenderer.h"


VulkanWindow::VulkanWindow(bool dbg) : m_debug(dbg)
{
    
}

QVulkanWindowRenderer *VulkanWindow::createRenderer()
{
    m_renderer = new LoopRenderer(this);
    return m_renderer;
}
void VulkanWindow::setBpm(float f)
{
    if (m_renderer != nullptr) {
        m_renderer->setBpm(f);
    } 
}