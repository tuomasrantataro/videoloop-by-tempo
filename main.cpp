#include <QApplication>
#include <QVulkanInstance>
#include <QLoggingCategory>
#include "gui.h"
#include "vulkanwindow.h"

Q_LOGGING_CATEGORY(lcVk, "qt.vulkan")

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const bool dbg = qEnvironmentVariableIntValue("QT_VK_DEBUG");

    QVulkanInstance inst;

    if (dbg) {
        QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));

#ifndef Q_OS_ANDROID
        inst.setLayers(QByteArrayList() << "VK_LAYER_LUNARG_standard_validation");
#else
        inst.setLayers(QByteArrayList()
                    << "VK_LAYER_GOOGLE_threading"
                    << "VK_LAYER_LUNARG_parameter_validation"
                    << "VK_LAYER_LUNARG_object_tracker"
                    << "VK_LAYER_LUNARG_core_validation"
                    << "VK_LAYER_LUNARG_image"
                    << "VK_LAYER_LUNARG_swapchain"
                    << "VK_LAYER_GOOGLE_unique_objects");
#endif
    }

    if (!inst.create())
            qFatal("Failed to create Vulkan instance: %d", inst.errorCode());

    VulkanWindow *vulkanWindow = new VulkanWindow(dbg);
    vulkanWindow->setVulkanInstance(&inst);

    MainWindow mainWindow(vulkanWindow);
    mainWindow.resize(1024, 768);
    mainWindow.show();

    return app.exec();
}