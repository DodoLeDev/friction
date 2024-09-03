/*
# enve2d - https://github.com/enve2d
#
# Copyright (c) enve2d developers
# Copyright (c) 2016-2020 Maurycy Liebner
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
*/

#include "GUI/mainwindow.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QDesktopWidget>

#include "hardwareinfo.h"
#include "Private/esettings.h"
#include "GUI/ewidgetsimpl.h"
#include "importhandler.h"
#include "effectsloader.h"
#include "memoryhandler.h"
#include "ShaderEffects/shadereffectprogram.h"
#include "videoencoder.h"
#include "appsupport.h"
#include "themesupport.h"

#ifdef Q_OS_WIN
#include "windowsincludes.h"
#include <QtPlatformHeaders/QWindowsWindowFunctions>
#endif

#include <QJSEngine>

#define GPU_NOT_COMPATIBLE gPrintException("Your GPU drivers do not seem to be compatible.")

void setDefaultFormat()
{
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(0);
    //format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    //format.setSwapInterval(0); // Disable vertical refresh syncing
    QSurfaceFormat::setDefaultFormat(format);
}

void generateAlphaMesh(QPixmap& alphaMesh,
                       const int dim)
{
    alphaMesh = QPixmap(2*dim, 2*dim);
    const QColor light = QColor::fromRgbF(0.2, 0.2, 0.2);
    const QColor dark = QColor::fromRgbF(0.4, 0.4, 0.4);
    QPainter p(&alphaMesh);
    p.fillRect(0, 0, dim, dim, light);
    p.fillRect(dim, 0, dim, dim, dark);
    p.fillRect(0, dim, dim, dim, dark);
    p.fillRect(dim, dim, dim, dim, light);
    p.end();
}

int main(int argc, char *argv[])
{
    // check if renderer
    const bool isRenderer = AppSupport::hasArg(argc, argv, "--renderer");

    // init env variables
    AppSupport::initEnv(isRenderer);

    // print version info
    AppSupport::printVersion();

    // print help
    if (AppSupport::hasArg(argc, argv, "--help")) {
        AppSupport::printHelp(isRenderer);
        return 0;
    }

    // init app
    QApplication::setApplicationDisplayName(AppSupport::getAppDisplayName());
    QApplication::setApplicationName(AppSupport::getAppName());
    QApplication::setOrganizationName(AppSupport::getAppCompany());
    QApplication::setOrganizationDomain(AppSupport::getAppDomain());
    QApplication::setApplicationVersion(AppSupport::getAppVersion());

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication::setAttribute(isRenderer ? Qt::AA_UseSoftwareOpenGL : Qt::AA_UseDesktopOpenGL);

    setDefaultFormat();
    QApplication app(argc, argv);
    setlocale(LC_NUMERIC, "C");

    // handle XDG args
#ifdef Q_OS_LINUX
    const auto handleXDGActs = AppSupport::handleXDGArgs(isRenderer,
                                                         QApplication::arguments());
    if (handleXDGActs.first) { return handleXDGActs.second; }
#endif

    // init hardware
#ifdef Q_OS_WIN
    if (!isRenderer) {
        QWindowsWindowFunctions::setHasBorderInFullScreenDefault(true);
    }
#endif

#ifndef Q_OS_DARWIN
    const bool threadedOpenGL = QOpenGLContext::supportsThreadedOpenGL();
    if (!threadedOpenGL) {
        gPrintException("Your GPU drivers do not support OpenGL "
                        "rendering outside the main thread");
    }
#endif

    try { HardwareInfo::sUpdateInfo(); }
    catch (const std::exception& e) {
        GPU_NOT_COMPATIBLE;
        gPrintExceptionCritical(e);
    }

    // init settings
    eSettings settings(HardwareInfo::sCpuThreads(),
                       HardwareInfo::sRamKB());

    // init ui
    OS_FONT = QApplication::font();
    eSizesUI::font.setEvaluator([]() {
        const auto fm = QFontMetrics(OS_FONT);
        return fm.height();
    });
    eSizesUI::widget.setEvaluator([]() {
        return eSizesUI::font.size()*4/3;
    });
    QObject::connect(&eSizesUI::font, &SizeSetter::sizeChanged,
                     &eSizesUI::widget, &SizeSetter::updateSize);
    eSizesUI::font.add(&app, [&app](const int size) {
        const auto fm = QFontMetrics(OS_FONT);
        const qreal mult = size/qreal(fm.height());
        QFont font = OS_FONT;
        if(OS_FONT.pixelSize() == -1) {
            font.setPointSizeF(mult*OS_FONT.pointSizeF());
        } else {
            font.setPixelSize(qRound(mult*OS_FONT.pixelSize()));
        }
        app.setFont(font);
    });

    eSizesUI::widget.add(&eSizesUI::button, [](const int size) {
        eSizesUI::button.set(qRound(size*1.1));
    });

    eSizesUI::widget.add([](const int size) {
        KEY_RECT_SIZE = size*3/5;
    });

    QPixmap alphaMesh;
    eSizesUI::widget.add([&alphaMesh](const int size) {
        generateAlphaMesh(alphaMesh, size/2);
    });
    ALPHA_MESH_PIX = &alphaMesh;

    // load theme
    ThemeSupport::setupTheme(eSizesUI::widget);

    // check permissions
    AppSupport::checkPerms(isRenderer);

    AppSupport::handlePortableFirstRun();
    // check XDG integration
#ifdef Q_OS_LINUX
    AppSupport::initXDGDesktop(isRenderer);
#endif

    // load settings
    try { settings.loadFromFile(); }
    catch (const std::exception& e) { gPrintExceptionCritical(e); }

    // init handlers
    eFilterSettings filterSettings;
    eWidgetsImpl widImpl;
    ImportHandler importHandler;

    MemoryHandler memoryHandler;
    TaskScheduler taskScheduler;
    QObject::connect(&memoryHandler, &MemoryHandler::enteredCriticalState,
                     &taskScheduler, &TaskScheduler::enterCriticalMemoryState);
    QObject::connect(&memoryHandler, &MemoryHandler::finishedCriticalState,
                     &taskScheduler, &TaskScheduler::finishCriticalMemoryState);

    // init document
    Document document(taskScheduler);
    Actions actions(document);

    // init effects
    EffectsLoader effectsLoader;
    try {
        effectsLoader.initializeGpu();
        taskScheduler.initializeGpu();
    } catch (const std::exception& e) {
        GPU_NOT_COMPATIBLE;
        gPrintExceptionFatal(e);
    }

    // disabled for now
    //effectsLoader.iniCustomPathEffects();
    //std::cout << "Custom path effects initialized" << std::endl;

    // disabled for now
    //effectsLoader.iniCustomRasterEffects();
    //std::cout << "Custom raster effects initialized" << std::endl;

    // init shaders
    try { effectsLoader.iniShaderEffects(); }
    catch (const std::exception& e) {
        GPU_NOT_COMPATIBLE;
        gPrintExceptionCritical(e);
    }
    QObject::connect(&effectsLoader, &EffectsLoader::programChanged,
                     [&document](ShaderEffectProgram * program) {
        for (const auto& scene : document.fScenes) {
            scene->updateIfUsesProgram(program);
        }
        document.actionFinished();
    });

    // disabled for now
    //effectsLoader.iniCustomBoxes();
    //std::cout << "Custom objects initialized" << std::endl;

    // init audio
    eSoundSettings soundSettings;
    AudioHandler audioHandler;
    if (!isRenderer) {
        try {
            audioHandler.initializeAudio(soundSettings.sData(),
                                         AppSupport::getSettings(QString::fromUtf8("audio"),
                                                                 QString::fromUtf8("output")).toString());
        } catch (const std::exception& e) { gPrintExceptionCritical(e); }
    }

    // init renderer
    const auto videoEncoder = enve::make_shared<VideoEncoder>();
    RenderHandler renderHandler(document,
                                audioHandler,
                                *videoEncoder,
                                memoryHandler,
                                isRenderer);

    // check for ffmpeg version
    AppSupport::checkFFmpeg(isRenderer);

    // load friction
    MainWindow w(document,
                 actions,
                 audioHandler,
                 renderHandler,
                 argc > 1 ? argv[1] : QString(),
                 nullptr,
                 isRenderer);
    if (!isRenderer) { w.show(); }
    try { return app.exec(); }
    catch (const std::exception& e) {
        gPrintExceptionFatal(e);
        return -1;
    }
}
