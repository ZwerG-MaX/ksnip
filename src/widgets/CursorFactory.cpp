/*
*  Copyright (C) 2017 Damir Porobic <https://github.com/damirporobic>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#include "CursorFactory.h"

CursorFactory::CursorFactory()
{
    mConfig = KsnipConfig::instance();
    mDefaultCursorSize = 8;
}

QCursor *CursorFactory::createPainterCursor(PaintMode mode, AbstractPainterItem *painterItem)
{
    auto cursorSize = getCursorSize(mode);

    switch(mode) {
        case PaintMode::Pen:
        case PaintMode::Marker:
        case PaintMode::Rect:
        case PaintMode::Ellipse:
        case PaintMode::Line:
        case PaintMode::Arrow:
            return new CustomCursor(CustomCursor::Circle, mConfig->toolColor(mode), cursorSize);
        case PaintMode::Text:
            return new QCursor(Qt::IBeamCursor);
        case PaintMode::Number:
            return new QCursor(Qt::PointingHandCursor);
        case PaintMode::Erase:
            return new CustomCursor(CustomCursor::Rect, QColor(Qt::white), cursorSize);
        case PaintMode::Select:
            if(painterItem == nullptr) {
                return new QCursor(Qt::OpenHandCursor);
            } else {
                return new QCursor(Qt::ClosedHandCursor);
            }
    }
}

QCursor* CursorFactory::createDefaultCursor()
{
    return new CustomCursor;
}

QCursor* CursorFactory::createSnippingCursor()
{
    return new CustomCursor(CustomCursor::Cross, mConfig->snippingCursorColor(), mConfig->snippingCursorSize());
}

int CursorFactory::getCursorSize(PaintMode mode) const
{
    if(!mConfig->dynamicCursorSizeEnabled()) {
        return mDefaultCursorSize;
    }

    return mConfig->toolSize(mode);
}
