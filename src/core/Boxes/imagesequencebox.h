﻿// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef IMAGESEQUENCEBOX_H
#define IMAGESEQUENCEBOX_H
#include "animationbox.h"
#include "FileCacheHandlers/imagesequencecachehandler.h"

class CORE_EXPORT ImageSequenceBox : public AnimationBox {
    e_OBJECT
protected:
    ImageSequenceBox();
public:
    void setFolderPath(const QString &folderPath);

    void changeSourceFile();

    void writeBoundingBox(eWriteStream& dst) const;
    void readBoundingBox(eReadStream& src);

    void prp_readPropertyXEV(const QDomElement& ele, const XevImporter& imp);
    QDomElement prp_writePropertyXEV(const XevExporter& exp) const;
private:
    void fileHandlerConnector(ConnContext& conn,
                              ImageSequenceFileHandler* obj);
    void fileHandlerAfterAssigned(ImageSequenceFileHandler *obj);

    FileHandlerObjRef<ImageSequenceFileHandler> mFileHandler;
};

#endif // IMAGESEQUENCEBOX_H
