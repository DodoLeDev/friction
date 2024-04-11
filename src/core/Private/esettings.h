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

// Fork of enve - Copyright (C) 2016-2020 Maurycy Liebner

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtCore>
#include <QColor>

#include "skia/skiaincludes.h"
#include "efiltersettings.h"
#include "memorystructs.h"

enum class GpuVendor {
    intel,
    amd,
    nvidia,
    unrecognized
};

enum class AccPreference : int {
    cpuStrongPreference,
    cpuSoftPreference,
    defaultPreference,
    gpuSoftPreference,
    gpuStrongPreference
};

class CORE_EXPORT eSettings : public QObject
{
    Q_OBJECT

public:
    eSettings(const int cpuThreads,
              const intKB ramKB);

    // accessors
    static intMB sRamMBCap();
    static int sCpuThreadsCapped();
    static const QString& sSettingsDir();
    static const QString& sIconsDir();

    static const eSettings &instance();
    static eSettings* sInstance;

    void loadDefaults();
    void loadFromFile();
    void saveToFile();

    // general
    QString fUserSettingsDir;

    // performance settings
    const int fCpuThreads;
    int fCpuThreadsCap = 0; // <= 0 - use all available threads

    const intKB fRamKB;
    intMB fRamMBCap = intMB(0); // <= 0 - cap at 80 %

    AccPreference fAccPreference = AccPreference::defaultPreference;
    bool fPathGpuAcc = true;

    bool fHddCache = true;
    QString fHddCacheFolder = ""; // "" - use system default temporary files folder
    intMB fHddCacheMBCap = intMB(0); // <= 0 - no cap

    // history
    int fUndoCap = 25; // <= 0 - no cap

    enum class AutosaveTarget {
        dedicated_folder,
        same_folder
    };

    int fQuickSaveCap = 5; // <= 0 - no cap
    int fAutoQuickSaveMin = 0; // <= 0 - disabled
    AutosaveTarget fQuickSaveTarget = AutosaveTarget::same_folder;

    // ui settings
    qreal fInterfaceScaling;
    qreal fCurrentInterfaceDPI;
    bool fDefaultInterfaceScaling;

    // canvas settings
    bool fCanvasRtlSupport;

    qreal fPathNodeScaling;
    QColor fPathNodeColor;
    QColor fPathNodeSelectedColor;

    qreal fPathDissolvedNodeScaling;
    QColor fPathDissolvedNodeColor;
    QColor fPathDissolvedNodeSelectedColor;

    qreal fPathControlScaling;
    QColor fPathControlColor;
    QColor fPathControlSelectedColor;

    // timeline settings
    bool fTimelineAlternateRow;
    QColor fTimelineAlternateRowColor;
    bool fTimelineHighlightRow;
    QColor fTimelineHighlightRowColor;

    QColor fObjectKeyframeColor;
    QColor fPropertyGroupKeyframeColor;
    QColor fPropertyKeyframeColor;
    QColor fSelectedKeyframeColor;

    QColor fVisibilityRangeColor;
    QColor fSelectedVisibilityRangeColor;
    QColor fAnimationRangeColor;

signals:
    void settingsChanged();

private:
    QString mIconsDir;
};

#endif // SETTINGS_H
