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

#include "RasterEffects/customrastereffect.h"
#include "ReadWrite/basicreadwrite.h"

using namespace Friction::Core;

CustomRasterEffect::CustomRasterEffect(const QString &name,
                                       const HardwareSupport hwSupport,
                                       const bool hwInterchangeable) :
    RasterEffect(name, hwSupport, hwInterchangeable,
                 RasterEffectType::CUSTOM) {}

void CustomRasterEffect::writeIdentifier(eWriteStream& dst) const {
    RasterEffect::writeIdentifier(dst);
    getIdentifier().write(dst);
}

void CustomRasterEffect::writeIdentifierXEV(QDomElement& ele) const {
    RasterEffect::writeIdentifierXEV(ele);
    getIdentifier().writeXEV(ele);
}
