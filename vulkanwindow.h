#ifndef VULKANWINDOW_H
#define VULKANWINDOW_H

#include "looprenderer.h"
#include <QVulkanWindow>
#include <QWidget>

class LoopRenderer;

class VulkanWindow : public QVulkanWindow
{
    Q_OBJECT
public:
    VulkanWindow(bool dbg);

    QVulkanWindowRenderer *createRenderer() override;

public slots:
    void setBpm(float);

signals:
    void toggleFullScreen();

protected:
    void mouseDoubleClickEvent(QMouseEvent *e);

private:
    bool m_debug;
    LoopRenderer *m_renderer = nullptr;
};

#endif