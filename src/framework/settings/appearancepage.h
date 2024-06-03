#ifndef APPEARANCEPAGE_H
#define APPEARANCEPAGE_H

#include <QFont>
#include <QPointer>

#include <CoreApi/isettingpage.h>

#include <idecoreFramework/idecoreframeworkglobal.h>

namespace Core {

    class AppearancePageWidget;

    class IDECORE_FRAMEWORK_EXPORT AppearancePage : public ISettingPage {
        Q_OBJECT
    public:
        explicit AppearancePage(QObject *parent = nullptr);
        ~AppearancePage();

    public:
        QString sortKeyword() const override;

        bool matches(const QString &word) const override;
        QWidget *widget() override;

        bool accept() override;
        void finish() override;

    private:
        QPointer<AppearancePageWidget> m_widget;
    };

}

#endif // APPEARANCEPAGE_H
