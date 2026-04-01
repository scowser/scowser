#include "app/Application.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    // Must be called before QApplication construction
    // Chromium requires this for sandboxing
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
        QByteArray(qgetenv("QTWEBENGINE_CHROMIUM_FLAGS"))
        + " --enable-strict-mixed-content-checking"
          " --disable-remote-fonts");

    Application app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
