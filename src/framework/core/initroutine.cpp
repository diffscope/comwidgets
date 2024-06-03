#include "initroutine.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QToolTip>

#include <QMWidgets/qmdecoratorv2.h>
#include <QMWidgets/qmappextension.h>

namespace Core {

    void InitRoutine::initializeAppearance(QSettings *settings) {
        settings->beginGroup(QStringLiteral("Preferences"));
        auto value = settings->value(QStringLiteral("Translation"));
        if (!value.isNull()) {
            qIDec->setLocale(value.toString());
        }

        value = settings->value(QStringLiteral("Theme"));
        if (!value.isNull()) {
            qIDec->setTheme(value.toString());
        }

        value = settings->value(QStringLiteral("Zoom"));
        if (!value.isNull()) {
            qIDec->setZoomRatio(value.toString().toDouble() / 100);
        }

        value = settings->value(QStringLiteral("Font"));
        {
            // Default font
            QFont font(QMAppExtension::systemDefaultFont());
            font.setPixelSize(12);

            // Get font
            if (!value.isNull()) {
                font.fromString(value.toString());
            }

            // Normalize font
            if (font.pixelSize() <= 0) {
                font.setPixelSize(12);
            }
            qIDec->setFontRatio(font.pixelSize() / 12.0);

            // The application font is determined at startup and remains unchanged, using the
            // `userFont` property to record the changes made by the user during the current
            // application life.
            qApp->setProperty("userFont", font);
            qApp->setProperty("userFontInitial", font);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            font.setPointSize(font.pixelSize() * 0.75 * qIDec->zoomRatio());
#else
            font.setPixelSize(font.pixelSize() * qIDec->zoomRatio());
#endif
            // Make the default font update with system dpi
            QGuiApplication::setFont(font);
            QToolTip::setFont(font);
        }

        value = settings->value(QStringLiteral("UseSystemFont"));
        {
            // Default value
            qApp->setProperty("useSystemFont", true);
            // Get value
            if (value.type() == QVariant::String) {
                qApp->setProperty("useSystemFont",
                                  value.toString().toLower() == QStringLiteral("true"));
            }
        }
        settings->endGroup();
    }

    struct InitRoutinePrivate {
        QSplashScreen *splash = nullptr;
        int startMode = InitRoutine::Application;
        InitRoutine::StartEntry entry;
    };

    static InitRoutinePrivate ir;

    QSplashScreen *InitRoutine::splash() {
        return ir.splash;
    }

    void InitRoutine::setSplash(QSplashScreen *splash) {
        ir.splash = splash;
    }

    int InitRoutine::startMode() {
        return ir.startMode;
    }

    void InitRoutine::setStartMode(InitRoutine::StartMode startMode) {
        ir.startMode = startMode;
    }

    InitRoutine::StartEntry InitRoutine::startEntry() {
        return ir.entry;
    }

    void InitRoutine::setStartEntry(const InitRoutine::StartEntry &startEntry) {
        ir.entry = startEntry;
    }

}