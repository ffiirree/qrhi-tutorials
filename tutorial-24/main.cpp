#include "rhi-widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    RhiWidget win{};
    win.show();

    return QApplication::exec();
}
