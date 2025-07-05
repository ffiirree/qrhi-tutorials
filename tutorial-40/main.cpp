#include "rhi-widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    RhiWidget win{};
    win.load(R"(C:\Users\ice_q\Downloads\hand_animation_test\scene.gltf)");
    win.show();

    return QApplication::exec();
}
