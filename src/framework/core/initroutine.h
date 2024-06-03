#ifndef INITROUTINE_H
#define INITROUTINE_H

#include <functional>

#include <QtCore/QSettings>
#include <QtWidgets/QWidget>
#include <QtWidgets/QSplashScreen>

#include <idecoreFramework/idecoreframeworkglobal.h>

namespace Core {

    class IDECORE_FRAMEWORK_EXPORT InitRoutine {
    public:
        enum StartMode {
            Application,
            VST,
        };
        using StartEntry = std::function<QWidget *()>;

        static void initializeAppearance(QSettings *settings);

    public:
        static QSplashScreen *splash();
        static void setSplash(QSplashScreen *splash);

        static int startMode();
        static void setStartMode(StartMode startMode);

        static StartEntry startEntry();
        static void setStartEntry(const StartEntry &startEntry);
    };

}

#endif // INITROUTINE_H
