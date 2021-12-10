/*
    MIT License

    Copyright (c) 2020-2021 Błażej Szczygieł

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "MainWindow.hpp"

#include <QApplication>
#include <QMessageBox>
#include <QDebug>

#include <filesystem>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

int main(int argc, char *argv[])
{
    qInstallMessageHandler([](QtMsgType t, const QMessageLogContext &c, const QString &s) {
        fprintf(stderr, "%s\n", qUtf8Printable(qFormatLogMessage(t, c, s)));
        fflush(stderr);
    });

    QApplication app(argc, argv);
    app.setApplicationName(VK_LAYER_FLIMES_GUI_NAME);
    app.setApplicationDisplayName("GUI for vk-layer-flimes external control");
    app.setApplicationVersion(VK_LAYER_FLIMES_GUI_VERSION);
    app.setWindowIcon(QIcon::fromTheme("vk_layer_flimes_gui"));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app.setFallbackSessionManagementEnabled(false);
#endif
    app.setQuitOnLastWindowClosed(false);

    const auto tmpPath = filesystem::temp_directory_path().concat(("/" + app.applicationName() + "." + QString(getenv("USER"))).toStdString());
    if (filesystem::is_fifo(tmpPath))
    {
        const int fd = open(tmpPath.c_str(), O_WRONLY | O_NONBLOCK);
        if (fd > -1)
        {
            close(fd);
            QMessageBox::warning(
                nullptr,
                QString(),
                "Application is already running",
                QMessageBox::Ok
            );
            return -1;
        }
    }
    else
    {
        filesystem::remove(tmpPath);
        mkfifo(tmpPath.c_str(), 0600);
    }

    MainWindow w;
    if (const int fd = open(tmpPath.c_str(), O_RDONLY | O_NONBLOCK); fd > -1)
    {
        w.setOnQuitFn([=] {
            close(fd);
            filesystem::remove(tmpPath);
        });
    }

    return app.exec();
}
