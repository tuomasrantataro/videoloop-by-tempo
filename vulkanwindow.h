#ifndef VULKANWINDOW_H
#define VULKANWINDOW_H

#include "looprenderer.h"
#include <QVulkanWindow>

class LoopRenderer;

class VulkanWindow : public QVulkanWindow
{
public:
    VulkanWindow(bool dbg);

    QVulkanWindowRenderer *createRenderer() override;

public slots:
    void setBpm(float);

private:
    bool m_debug;
    LoopRenderer *m_renderer = nullptr;
};

#endif