/*
    MIT License

    Copyright (c) 2020 Błażej Szczygieł

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

#pragma once

#include "KeySequence.hpp"

#include <QMainWindow>

#include <functional>

class ExternalControl;
class X11ActiveWindow;
class X11GlobalHotkey;
class PowerSupply;

class QListWidgetItem;
class QSystemTrayIcon;
class QDoubleSpinBox;
class QListWidget;
class QToolButton;
class QCheckBox;
class QSettings;
class QAction;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    using OnQuitFn = std::function<void()>;

    struct AppSettings
    {
        bool modified = false;

        bool active = true;
        bool inactive = true;
        bool battery = true;
    };

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setOnQuitFn(const OnQuitFn &fn);

private:
    void onQuit();

    inline QListWidgetItem *getSelectedItem() const;

    void toggleBypass();

    void updateAppsList();

    void updateAppsFpsLater();
    void updateAppsFps();

    void changeCurrAppSettings();

    void appsListSelectionChanged();

    void setBypassHotkey();
    void setBypassDuration();

    bool registerHotkey();

    void quit();

private:
    void showEvent(QShowEvent *e) override;
    void hideEvent(QHideEvent *e) override;
    void closeEvent(QCloseEvent *e) override;

private:
    const std::unique_ptr<ExternalControl> m_externalControl;
    const std::unique_ptr<X11ActiveWindow> m_x11ActiveWindow;
    const std::unique_ptr<X11GlobalHotkey> m_x11GlobalHotkey;
    const std::unique_ptr<PowerSupply> m_powerSupply;

    QSettings *const m_settings;

    QSystemTrayIcon *const m_tray;

    QAction *m_bypassAct = nullptr;

    QCheckBox *const m_activeFpsChecked;
    QDoubleSpinBox *const m_activeFps;

    QCheckBox *const m_inactiveFpsChecked;
    QDoubleSpinBox *const m_inactiveFps;

    QCheckBox *const m_batteryFpsChecked;
    QDoubleSpinBox *const m_batteryFps;

    QToolButton *const m_refresh;
    QListWidget *const m_appsList;

    QWidget *const m_appSettingsWidget;
    QCheckBox *const m_appActiveEnabled;
    QCheckBox *const m_appInactiveEnabled;
    QCheckBox *const m_appBatteryEnabled;

    QHash<QString, AppSettings> m_appSettings;

    QTimer *const m_updateAppsFpsTimer;

    pid_t m_activeWindowPid = 0;

    QtKeySequence m_bypassHotkey;
    QTimer *const m_bypassTimer;

    QByteArray m_geo;

    OnQuitFn m_onQuitFn;
    bool m_onQuitDone = false;
};
