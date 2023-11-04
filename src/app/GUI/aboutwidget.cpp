/*
#
# Friction - https://friction.graphics
#
# Copyright (c) Friction contributors
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
# See 'README.md' for more information.
#
*/

#include "aboutwidget.h"
#include "appsupport.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QTextBrowser>
#include <QFile>

AboutWidget::AboutWidget(QWidget *parent)
    : QWidget{parent}
{
    const auto mLayout = new QVBoxLayout(this);

    const auto mTopWidget = new QWidget(this);
    const auto mTopLayout = new QHBoxLayout(mTopWidget);

    const auto mTopLabel = new QLabel(this);

    mTopLabel->setText(QString::fromUtf8("<div style=\"margin: 0; padding: 0; text-align: center; font-weight: normal;\">"
                                         "<img src=\":/icons/hicolor/%2x%2/apps/%4.png\" width=\"%2\" height=\"%2\">"
                                         "<h1 style=\"font-weight: normal; margin-top: 0; padding-top: 0;\">%3<br><span style=\"font-size: large;\">%1</span></h1>"
                                         "</div>")
                                         .arg(AppSupport::getAppVersion(true),
                                              QString::number(96),
                                              AppSupport::getAppDisplayName(),
                                              AppSupport::getAppName()));

    mTopLayout->addStretch();
    mTopLayout->addWidget(mTopLabel);
    mTopLayout->addStretch();

    mLayout->addWidget(mTopWidget);

    QString css = "<style>"
                  "h1, h2, h3 { font-weight: normal; }"
                  "pre { font-size: small; white-space: pre-wrap; }"
                  "th { background-color: rgb(23, 23, 28); padding: .5em; }"
                  "td {"
                  "border: 1px solid rgb(23, 23, 28);"
                  "background-color: rgb(46, 46, 53);"
                  "padding: .5em;"
                  "}"
                  "</style>";

    const auto mTab = new QTabWidget(this);
    mTab->setSizePolicy(QSizePolicy::Expanding,
                        QSizePolicy::Expanding);
    mLayout->addWidget(mTab);

    AboutWidgetTab tab1;
    tab1.title = tr("About");
    tab1.path = ":/docs/about.html";

    AboutWidgetTab tab2;
    tab2.title = tr("Shortcuts");
    tab2.path = ":/docs/documentation/shortcuts.html";

    AboutWidgetTab tab3;
    tab3.title = tr("Privacy");
    tab3.path = ":/docs/privacy.html";

    QList<AboutWidgetTab> tabs;
    tabs << tab1 << tab2 << tab3;
    for (const auto &tab: tabs) {
        QFile file(tab.path);
        if (!file.open(QIODevice::Text | QIODevice::ReadOnly)) { continue; }
        const auto browser = new QTextBrowser(this);
        browser->setAcceptRichText(tab.html);
        browser->setOpenExternalLinks(tab.html);
        browser->setOpenLinks(tab.html);
        QString content = tab.html ? css : QString();
        content.append(file.readAll());
        if (tab.html) { browser->setHtml(content.replace("<table",
                                                         "<table width=\"100%\">")); }
        else { browser->setPlainText(content); }
        file.close();
        mTab->addTab(browser, tab.title);
    }

    const auto mThirdParty = new QTabWidget(this);
    mThirdParty->setTabPosition(QTabWidget::South);
    mThirdParty->setTabBarAutoHide(true);
    mThirdParty->setObjectName(QString::fromUtf8("ThirdPartyBrowser"));

    QStringList parties;
    parties << "friction";
#if defined LINUX_DEPLOY
    parties << "skia_bundle" << "qt"  << "qscintilla" << "ffmpeg" << "unwind" << "xkbcommon";
#elif defined WIN_DEPLOY
    parties << "skia_bundle" << "qt"  << "qscintilla" << "ffmpeg_win";
#else
    #ifdef FRICTION_BUNDLE_SKIA_BUNDLE
        parties << "skia_bundle";
    #else
        parties << "skia";
    #endif
    #ifdef Q_OS_LINUX
        parties << "gperftools";
    #endif
#endif

    for (const auto &doc: parties) {
        QFile file(doc == "friction" ? QString(":/docs/LICENSE") : QString(":/docs/3rdparty/%1.html").arg(doc));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { continue; }
        const auto browser = new QTextBrowser(this);
        browser->setOpenExternalLinks(false);
        browser->setOpenLinks(false);
        if (doc == "friction") { browser->setPlainText(file.readAll()); }
        else {
            QString html = css;
            html.append(file.readAll());
            browser->setHtml(html);
        }
        file.close();
        QString title = doc.split("_").takeFirst();
        mThirdParty->addTab(browser, title);
    }
    mTab->addTab(mThirdParty, tr("Licenses"));
}
