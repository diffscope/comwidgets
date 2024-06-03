#include "settingsdialog.h"

#include <QCollator>
#include <QLabel>

#include <QMWidgets/clineedit.h>
#include <QMWidgets/ctreewidget.h>
#include <QMWidgets/qmdecoratorv2.h>

#include <CoreApi/iloader.h>
#include <CoreApi/icorebase.h>

namespace Core {

    static int TitleRole = Qt::UserRole + 1000;
    static int EntityRole = TitleRole + 1;

    static const char settingCatalogC[] = "Core/SettingCatalog";

    static const char lastSettingPageIdC[] = "LastSettingPageId";

    static QString _getItemPathTitle(QTreeWidgetItem *item) {
        QStringList titles;
        while (item) {
            titles.prepend(item->text(0));
            item = item->parent();
        }
        return titles.join(" > ");
    }

    struct Order {
        QCollator qoc;
        Order() {
            qoc = QCollator(QLocale(QLocale::English));
        }

        bool operator()(QTreeWidgetItem *a, QTreeWidgetItem *b) const {
            return qoc.compare(a->data(0, TitleRole).toString(), b->data(0, TitleRole).toString()) <
                   0;
        }
    };

    SettingsDialog::SettingsDialog(QWidget *parent) : SVS::ConfigurableDialog(parent) {
        m_currentPage = nullptr;
        m_catalogWidget = nullptr;

        // Left
        m_tree = new CTreeWidget();
        m_tree->setHeaderHidden(true);

        m_searchBox = new CLineEdit();
        m_searchBox->setClearButtonEnabled(true);

        leftLayout = new QVBoxLayout();
        leftLayout->setContentsMargins({});
        // leftLayout->setSpacing(0);

        leftLayout->addWidget(m_searchBox);
        leftLayout->addWidget(m_tree);

        leftWidget = new QWidget();
        leftWidget->setLayout(leftLayout);

        // Right
        titleLabel = new QLabel();
        descriptionLabel = new QLabel();

        labelLayout = new QVBoxLayout();
        labelLayout->setContentsMargins({});
        labelLayout->setSpacing(0);

        labelLayout->addWidget(titleLabel);
        labelLayout->addWidget(descriptionLabel);

        m_page = new QStackedWidget();

        rightLayout = new QVBoxLayout();
        rightLayout->setContentsMargins({});
        // leftLayout->setSpacing(0);

        rightLayout->addLayout(labelLayout);
        rightLayout->addWidget(m_page);

        rightWidget = new QWidget();
        rightWidget->setLayout(rightLayout);

        topSplitter = new QSplitter();
        topSplitter->addWidget(leftWidget);
        topSplitter->addWidget(rightWidget);

        topSplitter->setStretchFactor(0, 0);
        topSplitter->setStretchFactor(1, 1);

        setWidget(topSplitter);

        auto sc = ICoreBase::instance()->settingCatalog();

        // Build tree widget
        {
            QList<QTreeWidgetItem *> items;
            for (auto page : sc->pages()) {
                items.append(buildTreeWidgetItem(page));
            }
            std::sort(items.begin(), items.end(), Order());
            m_tree->addTopLevelItems(items);
        }

        connect(m_tree, &QTreeWidget::currentItemChanged, this,
                &SettingsDialog::_q_currentItemChanged);
        connect(m_searchBox, &QLineEdit::textChanged, this,
                &SettingsDialog::_q_searchBoxTextChanged);

        connect(sc, &SettingCatalog::titleChanged, this, &SettingsDialog::_q_titleChanged);
        connect(sc, &SettingCatalog::descriptionChanged, this,
                &SettingsDialog::_q_descriptionChanged);

        qIDec->installLocale(this);

        // Init window sizes
        auto winMgr = ICoreBase::instance()->windowSystem();
        winMgr->loadGeometry(metaObject()->className(), this, {1280, 720});
        winMgr->loadSplitterSizes(metaObject()->className(), topSplitter, {250, width() - 250});

        // Init last page id
        m_settings = ILoader::instance()->settings()->value(settingCatalogC).toObject();
        selectPage(m_settings.value(lastSettingPageIdC).toString());
    }

    SettingsDialog::~SettingsDialog() {
        // Save last page id
        m_settings.insert(lastSettingPageIdC, m_currentPage ? m_currentPage->id() : QString());
        ILoader::instance()->settings()->insert(settingCatalogC, m_settings);

        // Save window sizes
        auto winMgr = ICoreBase::instance()->windowSystem();
        winMgr->saveSplitterSizes(metaObject()->className(), topSplitter);
        winMgr->saveGeometry(metaObject()->className(), this);
    }

    void SettingsDialog::reloadStrings() {
        m_searchBox->setPlaceholderText(tr("Search for settings"));

        setWindowTitle(tr("Settings"));
    }

    void SettingsDialog::selectPage(const QString &id) {
        auto sc = ICoreBase::instance()->settingCatalog();
        auto pages = sc->pages(id);
        if (!pages.isEmpty()) {
            auto item = m_treeIndexes.value(pages.front(), nullptr);
            if (item) {
                m_tree->setCurrentItem(item);
                return;
            }
        }
        if (!id.isEmpty()) {
            return;
        }
        if (m_tree->topLevelItemCount()) {
            m_tree->setCurrentItem(m_tree->topLevelItem(0));
            return;
        }
        clearPage();
    }

    void SettingsDialog::apply() {
        // accept all
        for (auto it = m_treeIndexes.begin(); it != m_treeIndexes.end(); ++it) {
            it.key()->accept();
        }
    }

    void SettingsDialog::finish() {
        // finish all
        for (auto it = m_treeIndexes.begin(); it != m_treeIndexes.end(); ++it) {
            it.key()->finish();
        }
    }

    QTreeWidgetItem *SettingsDialog::buildTreeWidgetItem(Core::ISettingPage *page) {
        auto item = new QTreeWidgetItem();
        item->setText(0, page->title());
        item->setToolTip(0, page->description());
        item->setData(0, TitleRole, page->sortKeyword());
        item->setData(0, EntityRole, QVariant::fromValue(page));

        QList<QTreeWidgetItem *> items;
        for (auto sub : page->pages()) {
            items.append(buildTreeWidgetItem(sub));
        }
        std::sort(items.begin(), items.end(), Order());
        item->addChildren(items);

        m_treeIndexes.insert(page, item);

        auto text = m_searchBox->text();
        if (!text.isEmpty()) {
            item->setHidden(!page->matches(text));

            std::list<QTreeWidgetItem *> items;
            for (const auto &sub : page->allPages()) {
                auto item = m_treeIndexes[sub];

                // All hidden
                item->setHidden(true);

                if (sub->matches(text)) {
                    items.push_back(item);
                }
            }

            for (auto p : qAsConst(items)) {
                while (p && p->isHidden()) {
                    p->setHidden(false);
                    p = p->parent();
                }
            }
        }

        return item;
    }

    void SettingsDialog::clearPage() {
        if (m_page->count()) {
            auto w = m_page->widget(0);
            m_page->removeWidget(w);

            if (w == m_catalogWidget) {
                delete w;
                m_catalogWidget = nullptr;
            }
        }

        titleLabel->setText({});
        descriptionLabel->setText({});
    }

    static QString unescapeAccelerateChar(const QString &s) {
        QString res;
        for (const auto &ch : s) {
            if (ch == '&') {
                res += "&&";
            } else {
                res += ch;
            }
        }
        return res;
    }

    void SettingsDialog::showCatalog() {
        auto w = new QFrame();

        auto layout = new QVBoxLayout();

        auto curItem = m_tree->currentItem();
        for (int i = 0; i < curItem->childCount(); ++i) {
            auto page = curItem->child(i)->data(0, EntityRole).value<ISettingPage *>();
            if (!page)
                continue;

            auto btn = new CTabButton();
            btn->setProperty("id", page->id());
            btn->setText(unescapeAccelerateChar(page->title()));

            // Update text
            connect(page, &ISettingPage::titleChanged, btn, [btn, page]() {
                btn->setText(unescapeAccelerateChar(page->title())); //
            });

            connect(btn, &QAbstractButton::clicked, this, [this]() {
                selectPage(sender()->property("id").toString()); //
            });
            layout->addWidget(btn);
        }
        layout->addStretch();

        w->setLayout(layout);

        clearPage();

        titleLabel->setText(_getItemPathTitle(curItem));
        descriptionLabel->setText(m_currentPage->description());
        m_page->addWidget(w);

        m_catalogWidget = w;
    }

    void SettingsDialog::_q_titleChanged(ISettingPage *page, const QString &title) {
        auto item = m_treeIndexes.value(page, nullptr);
        if (!item) {
            return;
        }
        item->setText(0, title);

        auto curItem = m_tree->currentItem();
        while (item && item != curItem) {
            item = item->parent();
        }

        if (item) {
            titleLabel->setText(_getItemPathTitle(curItem));
        }
    }

    void SettingsDialog::_q_descriptionChanged(Core::ISettingPage *page,
                                               const QString &description) {
        auto item = m_treeIndexes.value(page, nullptr);
        if (!item) {
            return;
        }
        item->setToolTip(0, description);

        if (page == m_currentPage) {
            descriptionLabel->setText(description);
        }
    }

    void SettingsDialog::_q_currentItemChanged(QTreeWidgetItem *cur, QTreeWidgetItem *prev) {
        Q_UNUSED(prev);

        clearPage();

        auto page = cur ? cur->data(0, EntityRole).value<ISettingPage *>() : nullptr;
        m_currentPage = page;

        if (!page) {
            return;
        }

        auto w = page->widget();
        if (w) {
            titleLabel->setText(_getItemPathTitle(cur));
            descriptionLabel->setText(page->description());
            m_page->addWidget(w);
        } else {
            // Add a catalog widget with description
            showCatalog();
        }
    }

    void SettingsDialog::_q_searchBoxTextChanged(const QString &text) {
        if (text.isEmpty()) {
            for (const auto &item : qAsConst(m_treeIndexes)) {
                item->setHidden(false);
            }
            return;
        }

        std::list<QTreeWidgetItem *> items;
        for (auto it = m_treeIndexes.begin(); it != m_treeIndexes.end(); ++it) {
            auto item = it.value();

            // All hidden
            item->setHidden(true);

            if (it.key()->matches(text)) {
                items.push_back(item);
            }
        }


        for (auto p : qAsConst(items)) {
            while (p && p->isHidden()) {
                p->setHidden(false);
                p = p->parent();
            }
        }
    }

}