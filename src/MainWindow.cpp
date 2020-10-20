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

#include "MainWindow.hpp"
#include "ExternalControl.hpp"
#include "X11ActiveWindow.hpp"
#include "PowerSupply.hpp"

#include <QSystemTrayIcon>
#include <QDoubleSpinBox>
#include <QStandardPaths>
#include <QApplication>
#include <QFormLayout>
#include <QListWidget>
#include <QToolButton>
#include <QMessageBox>
#include <QBoxLayout>
#include <QCheckBox>
#include <QSettings>
#include <qevent.h>
#include <QLabel>
#include <QDebug>
#include <QTimer>
#include <QMenu>

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_externalControl(make_unique<ExternalControl>())
    , m_x11ActiveWindow(make_unique<X11ActiveWindow>())
    , m_powerSupply(make_unique<PowerSupply>())
    , m_settings(new QSettings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" + QCoreApplication::applicationName() + ".ini", QSettings::IniFormat, this))
    , m_tray(new QSystemTrayIcon(this))
    , m_activeFpsChecked(new QCheckBox("Active"))
    , m_activeFps(new QDoubleSpinBox)
    , m_inactiveFpsChecked(new QCheckBox("Inactive"))
    , m_inactiveFps(new QDoubleSpinBox)
    , m_batteryFpsChecked(new QCheckBox("Battery"))
    , m_batteryFps(new QDoubleSpinBox)
    , m_refresh(new QToolButton)
    , m_appsList(new QListWidget)
    , m_appSettingsWidget(new QWidget)
    , m_appActiveEnabled(new QCheckBox(m_activeFpsChecked->text()))
    , m_appInactiveEnabled(new QCheckBox(m_inactiveFpsChecked->text()))
    , m_appBatteryEnabled(new QCheckBox(m_batteryFpsChecked->text()))
    , m_updateAppsFpsTimer(new QTimer(this))
{
    for (auto &&group : m_settings->childGroups())
    {
        auto &settings = m_appSettings[group];
        settings.modified = true;
        settings.active = m_settings->value(group + "/Active", true).toBool();
        settings.inactive = m_settings->value(group + "/Inactive", true).toBool();
        settings.battery = m_settings->value(group + "/Battery", true).toBool();
    }

    auto w = new QWidget;

    auto menu = new QMenu(this);
    menu->addAction("Show", this, [this] {
        if (!isVisible())
        {
            show();
        }
        else
        {
            raise();
            activateWindow();
        }
    });
    menu->addSeparator();
    auto quitAct = menu->addAction("Quit", this, &MainWindow::quit);

    quitAct->setShortcut(QKeySequence("Ctrl+Q"));
    addAction(quitAct);

    m_tray->setIcon(QApplication::windowIcon());
    m_tray->setContextMenu(menu);
    m_tray->show();

    m_activeFpsChecked->setChecked(m_settings->value("ActiveFpsChecked").toBool());
    m_activeFps->setDecimals(3);
    m_activeFps->setRange(1.0, 1000.0);
    m_activeFps->setSuffix(" FPS");
    m_activeFps->setValue(m_settings->value("ActiveFps", 60.0).toDouble());
    m_activeFps->setEnabled(m_activeFpsChecked->isChecked());

    if (m_x11ActiveWindow->isOk())
    {
        m_inactiveFpsChecked->setToolTip("Some X11 applications might not set the \"_NET_WM_PID\" property.\n"
                                         "All these applications will be treated as inactive.\n"
                                         "To workaround the issue, unset \"Inactive\" for this application.");
        m_inactiveFpsChecked->setChecked(m_settings->value("InactiveFpsChecked").toBool());
        m_inactiveFps->setDecimals(3);
        m_inactiveFps->setRange(1.0, 1000.0);
        m_inactiveFps->setSuffix(" FPS");
        m_inactiveFps->setValue(m_settings->value("InactiveFps", 20.0).toDouble());
        m_inactiveFps->setEnabled(m_inactiveFpsChecked->isChecked());
    }

    if (m_powerSupply->isOk())
    {
        m_batteryFpsChecked->setChecked(m_settings->value("BatteryFpsChecked").toBool());
        m_batteryFps->setDecimals(3);
        m_batteryFps->setRange(1.0, 1000.0);
        m_batteryFps->setSuffix(" FPS");
        m_batteryFps->setValue(m_settings->value("BatteryFps", 30.0).toDouble());
        m_batteryFps->setEnabled(m_batteryFpsChecked->isChecked());
    }

    m_refresh->setIcon(QIcon::fromTheme("view-refresh"));
    m_refresh->setToolTip("Refresh");

    m_appSettingsWidget->hide();

    m_appActiveEnabled->setToolTip("Allow FPS limit if application is active");
    m_appInactiveEnabled->setToolTip("Allow FPS limit if application is inactive");
    m_appBatteryEnabled->setToolTip("Allow FPS limit if system runs on battery");

    m_updateAppsFpsTimer->setInterval(125);

    auto hLine = new QFrame;
    hLine->setFrameShape(QFrame::HLine);
    hLine->setFrameShadow(QFrame::Sunken);

    auto vLine1 = new QFrame;
    vLine1->setFrameShape(QFrame::VLine);
    vLine1->setFrameShadow(QFrame::Sunken);

    auto vLine2 = new QFrame;
    vLine2->setFrameShape(QFrame::VLine);
    vLine2->setFrameShadow(QFrame::Sunken);

    auto appSettingsLayout = new QHBoxLayout(m_appSettingsWidget);
    appSettingsLayout->setMargin(0);
    appSettingsLayout->addWidget(vLine1);
    appSettingsLayout->addStretch();
    appSettingsLayout->addWidget(m_appActiveEnabled);
    appSettingsLayout->addWidget(m_appInactiveEnabled);
    appSettingsLayout->addWidget(m_appBatteryEnabled);
    appSettingsLayout->addStretch();
    appSettingsLayout->addWidget(vLine2);

    auto topLayout = new QFormLayout;
    topLayout->addRow(m_activeFpsChecked, m_activeFps);
    if (m_x11ActiveWindow->isOk())
        topLayout->addRow(m_inactiveFpsChecked, m_inactiveFps);
    if (m_powerSupply->isOk())
        topLayout->addRow(m_batteryFpsChecked, m_batteryFps);

    auto bottomLayout = new QGridLayout;
    bottomLayout->addWidget(hLine, 0, 0, 1, 3);
    bottomLayout->addWidget(new QLabel("Available applications"), 1, 0, 1, 1);
    bottomLayout->addWidget(m_appSettingsWidget, 1, 1, 1, 1);
    bottomLayout->addWidget(m_refresh, 1, 2, 1, 1);
    bottomLayout->addWidget(m_appsList, 2, 0, 1, 3);

    auto mainLayout = new QVBoxLayout(w);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);

    connect(m_externalControl.get(), &ExternalControl::applicationsChanged,
            this, &MainWindow::updateAppsList);
    connect(m_x11ActiveWindow.get(), &X11ActiveWindow::activeWindowPidChanged,
            this, [this](pid_t pid) {
        m_activeWindowPid = pid;
        updateAppsFps();
    });
    connect(m_powerSupply.get(), &PowerSupply::powerSourceChanged,
            this, &MainWindow::updateAppsFpsLater);

    connect(m_tray, &QSystemTrayIcon::activated,
            this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger)
            setVisible(!isVisible());
    });

    connect(m_activeFpsChecked, &QCheckBox::toggled,
            m_activeFps, &QDoubleSpinBox::setEnabled);
    connect(m_inactiveFpsChecked, &QCheckBox::toggled,
            m_inactiveFps, &QDoubleSpinBox::setEnabled);
    connect(m_batteryFpsChecked, &QCheckBox::toggled,
            m_batteryFps, &QDoubleSpinBox::setEnabled);

    connect(m_refresh, &QToolButton::clicked,
            m_externalControl.get(), &ExternalControl::refresh);

    connect(m_activeFpsChecked, &QCheckBox::toggled,
            this, &MainWindow::updateAppsFpsLater);
    connect(m_activeFps, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::updateAppsFpsLater);
    connect(m_inactiveFpsChecked, &QCheckBox::toggled,
            this, &MainWindow::updateAppsFpsLater);
    connect(m_inactiveFps, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::updateAppsFpsLater);
    connect(m_batteryFpsChecked, &QCheckBox::toggled,
            this, &MainWindow::updateAppsFpsLater);
    connect(m_batteryFps, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::updateAppsFpsLater);

    connect(m_appsList->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::appsListSelectionChanged);

    connect(m_appActiveEnabled, &QCheckBox::toggled,
            this, &MainWindow::changeCurrAppSettings);
    connect(m_appInactiveEnabled, &QCheckBox::toggled,
            this, &MainWindow::changeCurrAppSettings);
    connect(m_appBatteryEnabled, &QCheckBox::toggled,
            this, &MainWindow::changeCurrAppSettings);

    connect(m_updateAppsFpsTimer, &QTimer::timeout,
            this, &MainWindow::updateAppsFps);

    updateAppsList();

    QCoreApplication::instance()->installNativeEventFilter(m_x11ActiveWindow.get());

    setCentralWidget(w);

    m_geo = QByteArray::fromBase64(m_settings->value("Geometry").toByteArray());

    if (!m_externalControl->isOk())
    {
        QTimer::singleShot(0, this, [this] {
            QMessageBox::critical(
                this,
                QString(),
                "Can't get apllications list",
                QMessageBox::Ok
            );
            QCoreApplication::quit();
        });
    }

    if (m_settings->value("Visible", true).toBool())
        show();
}
MainWindow::~MainWindow()
{
    QCoreApplication::instance()->removeNativeEventFilter(m_x11ActiveWindow.get());

    m_settings->setValue("Visible", isVisible());

    hide();

    if (m_externalControl->isOk() && (m_inactiveFpsChecked->isChecked() || m_batteryFpsChecked->isChecked()))
    {
        const double fps = m_activeFpsChecked->isChecked()
            ? m_activeFps->value()
            : 0.0
        ;
        for (auto &&app : m_externalControl->applications())
            m_externalControl->setFps(app, m_appSettings[app.name].active ? fps : 0.0);
    }

    m_settings->setValue("ActiveFpsChecked", m_activeFpsChecked->isChecked());
    m_settings->setValue("ActiveFps", m_activeFps->value());
    if (m_x11ActiveWindow->isOk())
    {
        m_settings->setValue("InactiveFpsChecked", m_inactiveFpsChecked->isChecked());
        m_settings->setValue("InactiveFps", m_inactiveFps->value());
    }
    if (m_powerSupply->isOk())
    {
        m_settings->setValue("BatteryFpsChecked", m_batteryFpsChecked->isChecked());
        m_settings->setValue("BatteryFps", m_batteryFps->value());
    }
    m_settings->setValue("Geometry", m_geo.toBase64().constData());

    for (auto it = m_appSettings.begin(), itEnd = m_appSettings.end(); it != itEnd; ++it)
    {
        auto &&settings = it.value();
        if (!settings.modified)
            continue;

        m_settings->setValue(it.key() + "/Active", settings.active);
        m_settings->setValue(it.key() + "/Inactive", settings.inactive);
        m_settings->setValue(it.key() + "/Battery", settings.battery);
    }
}

inline QListWidgetItem *MainWindow::getSelectedItem() const
{
    return m_appsList->selectedItems().value(0);
}

void MainWindow::updateAppsList()
{
    QString currentAppName;

    if (auto item = getSelectedItem())
        currentAppName = item->data(Qt::UserRole).toString();

    auto selectionModel = m_appsList->selectionModel();
    selectionModel->blockSignals(true);
    m_appsList->clear();
    selectionModel->blockSignals(false);

    for (auto &&app : m_externalControl->applications())
    {
        auto item = new QListWidgetItem(QString("%1 (%2)").arg(app.name).arg(app.pid));
        item->setData(Qt::UserRole, app.name);

        m_appsList->addItem(item);

        if (m_appsList->currentItem() == nullptr && app.name == currentAppName)
            m_appsList->setCurrentItem(item);
    }

    if (!m_appsList->currentItem() && !currentAppName.isEmpty())
        appsListSelectionChanged();

    updateAppsFpsLater();
}

void MainWindow::updateAppsFpsLater()
{
    m_updateAppsFpsTimer->start();
}
void MainWindow::updateAppsFps()
{
    m_updateAppsFpsTimer->stop();

    const double activeFps = m_activeFpsChecked->isChecked()
        ? m_activeFps->value()
        : 0.0
    ;
    const double inactiveFps = m_inactiveFpsChecked->isChecked()
        ? m_inactiveFps->value()
        : activeFps
    ;
    const double batteryFps = m_batteryFpsChecked->isChecked()
        ? m_batteryFps->value()
        : activeFps
    ;

    const bool battery = (m_powerSupply->isOk() && m_powerSupply->isBattery());

    for (auto &&app : m_externalControl->applications())
    {
        const bool active = (app.pid == m_activeWindowPid);

        auto &settings = m_appSettings[app.name];

        double fps = 0.0;
        if (settings.active)
            fps = activeFps;
        if (!active && settings.inactive)
            fps = inactiveFps;
        if (battery && settings.battery && (fps == 0.0 || batteryFps < fps))
            fps = batteryFps;
        m_externalControl->setFps(app, fps);
    }
}

void MainWindow::changeCurrAppSettings()
{
    auto item = getSelectedItem();
    if (!item)
        return;

    auto &settings = m_appSettings[item->data(Qt::UserRole).toString()];
    settings.modified = true;
    settings.active = m_appActiveEnabled->isChecked();
    settings.inactive = m_appInactiveEnabled->isChecked();
    settings.battery = m_appBatteryEnabled->isChecked();

    updateAppsFpsLater();
}

void MainWindow::appsListSelectionChanged()
{
    auto item = getSelectedItem();
    if (!item)
    {
        m_appSettingsWidget->hide();
        return;
    }

    QSignalBlocker blocker[] {
        QSignalBlocker(m_appActiveEnabled),
        QSignalBlocker(m_appInactiveEnabled),
        QSignalBlocker(m_appBatteryEnabled),
    };

    auto &settings = m_appSettings[item->data(Qt::UserRole).toString()];

    m_appActiveEnabled->setChecked(settings.active);
    m_appInactiveEnabled->setChecked(settings.inactive);
    m_appBatteryEnabled->setChecked(settings.battery);

    m_appSettingsWidget->show();
}

void MainWindow::quit()
{
    auto choice = QMessageBox::question(
        isVisible() ? this : nullptr,
        QString(),
        "Do you want to quit?",
        QMessageBox::Yes, QMessageBox::No
    );
    if (choice == QMessageBox::Yes)
    {
        QCoreApplication::quit();
    }
}

void MainWindow::showEvent(QShowEvent *e)
{
    restoreGeometry(m_geo);
    QMainWindow::showEvent(e);
}
void MainWindow::hideEvent(QHideEvent *e)
{
    m_geo = saveGeometry();
    QMainWindow::hideEvent(e);
}
void MainWindow::closeEvent(QCloseEvent *e)
{
    if (!m_tray->isSystemTrayAvailable())
    {
        quit();
        return;
    }
    QMainWindow::closeEvent(e);
}
