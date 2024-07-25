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

#include "commandpalette.h"

#include <QVBoxLayout>
#include <QKeyEvent>

#include "Private/esettings.h"

CommandPalette::CommandPalette(QWidget *parent)
    : QDialog(parent)
    , mUserInput(nullptr)
    , mSuggestions(nullptr)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setWindowTitle(tr("Command Palette"));

    setMinimumWidth(350);

    const auto lay = new QVBoxLayout(this);

    mSuggestions = new QListWidget(this);
    mUserInput = new QLineEdit(this);

    lay->addWidget(mUserInput);
    lay->addWidget(mSuggestions);

    mSuggestions->setObjectName("CommandPaletteSuggestions");
    mSuggestions->setMinimumHeight(100);
    mSuggestions->hide();

    mUserInput->setObjectName("CommandPaletteInput");
    mUserInput->setPlaceholderText(tr("Search for action ..."));
    mUserInput->installEventFilter(this);
    mUserInput->setFocus();

    connect(mUserInput, &QLineEdit::textChanged, this, [this]() {
        mSuggestions->clear();
        mSuggestions->hide();
        if (mUserInput->text().isEmpty()) { return; }
        const auto actions = eSettings::instance().fCommandPalette;
        int items = 0;
        for (int i = 0; i < actions.count(); i++) {
            const auto act = actions.at(i);
            if (!act->text().startsWith(mUserInput->text(), Qt::CaseInsensitive)) { continue; }
            const auto item = new QListWidgetItem(act->text(), mSuggestions);
            item->setData(Qt::UserRole, i);
            mSuggestions->addItem(item);
            items++;
        }
        if (mSuggestions->isHidden() && items > 0) { mSuggestions->show(); }
    });

    connect(mUserInput, &QLineEdit::returnPressed, this, [this]() {
        if (mSuggestions->count() < 1) { return; }
        const auto index = mSuggestions->item(0)->data(Qt::UserRole).toInt();
        const auto act = eSettings::instance().fCommandPalette.at(index);
        if (!act) { return; }
        act->trigger();
        close();
    });

    connect(mSuggestions, &QListWidget::itemActivated, this, [this](const auto item) {
        const auto index = item->data(Qt::UserRole).toInt();
        const auto act = eSettings::instance().fCommandPalette.at(index);
        if (!act) { return; }
        act->trigger();
        close();
    });
}

bool CommandPalette::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == mUserInput) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
            if (keyEvent->key() == Qt::Key_Down) {
                mSuggestions->setFocus();
                return true;
            }
        }
        return false;
    }
    return QDialog::eventFilter(obj, e);
}
