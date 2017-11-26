/*
 *  Copyright (C) 2016 Damir Porobic <https://github.com/damirporobic>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#include "SnippingArea.h"

SnippingArea::SnippingArea(QWidget* parent) : QWidget(parent),
    mCursorFactory(new CursorFactory()),
    mConfig(KsnipConfig::instance()),
    mBackground(nullptr)
{
    // Make the frame span across the screen and show above any other widget
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);

    QWidget::setCursor(*mCursorFactory->createSnippingCursor());
}

SnippingArea::~SnippingArea()
{
    delete mCursorFactory;
    delete mBackground;
}

void SnippingArea::showWithoutBackground()
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    clearBackgroundImage();
    show();
}

void SnippingArea::showWithBackground(const QPixmap& background)
{
    setAttribute(Qt::WA_TranslucentBackground, false);
    setBackgroundImage(background);
    show();
}

void SnippingArea::show()
{
    init();
    setFixedSize(QDesktopWidget().size());
    QWidget::show();
    activateWindow();
}

void SnippingArea::setBackgroundImage(const QPixmap& background)
{
    clearBackgroundImage();
    mBackground = new QPixmap(background);
}

void SnippingArea::clearBackgroundImage()
{
    if(mBackground != nullptr) {
        delete mBackground;
    }
}

void SnippingArea::init()
{
    mCursorRulerEnabled = mConfig->cursorRulerEnabled();
    mCursorInfoEnabled = mConfig->cursorInfoEnabled();
    setMouseTracking(mCursorRulerEnabled || mCursorInfoEnabled);
    mMouseIsDown = false;
}

void SnippingArea::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    mMouseDownPosition = event->pos();
    updateCapturedArea(mMouseDownPosition, event->pos());
    mMouseIsDown = true;
}

void SnippingArea::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    mMouseIsDown = false;
    hide();
    emit areaSelected(mCaptureArea);
}

void SnippingArea::mouseMoveEvent(QMouseEvent* event)
{
    if (mMouseIsDown) {
        updateCapturedArea(mMouseDownPosition, event->pos());
    }
    update();
    QWidget::mouseMoveEvent(event);
}

void SnippingArea::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    if (mBackground != nullptr) {
        painter.drawPixmap(geometry(), *mBackground);
    }

    if (mMouseIsDown) {
        painter.setClipRegion(QRegion(geometry()).subtracted(QRegion(mCaptureArea)));
    }

    painter.setBrush(QColor(0, 0, 0, 150));
    painter.drawRect(geometry());

    if (mCursorRulerEnabled) {
        drawCursorRuler(painter);
    }

    if (mCursorInfoEnabled) {
        drawCursorInfo(painter);
    }

    if (mMouseIsDown) {
        painter.setPen(QPen(Qt::red, 4, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
        painter.drawRect(mCaptureArea);
    }

    QWidget::paintEvent(event);
}

void SnippingArea::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        emit cancel();
        close();
    }
    QWidget::keyPressEvent(event);
}

void SnippingArea::updateCapturedArea(const QPoint& pos1, const QPoint& pos2)
{
    mCaptureArea = MathHelper::getRectBetweenTwoPoints(pos1, pos2);
}

QString SnippingArea::createInfoText(int number1, int number2) const
{
    return QString::number(number1) + ", " + QString::number(number2);
}

QPoint SnippingArea::calculateInfoTextPosition(const QPoint& pos) const
{
    auto drawPos = pos + QPoint(6, 15);
    if (mMouseIsDown && mCaptureArea.contains(drawPos)) {
        drawPos = pos + QPoint(6, -5);
    }
    return drawPos;
}

void SnippingArea::drawCursorRuler(QPainter& painter) const
{
    auto pos = QCursor::pos() + QPoint(2, 2);
    painter.setPen(QPen(Qt::red, 1, Qt::DotLine, Qt::SquareCap, Qt::MiterJoin));
    painter.drawLine(QPointF(pos.x(), geometry().bottom()), QPointF(pos.x(), geometry().top()));
    painter.drawLine(QPointF(geometry().left(), pos.y()), QPointF(geometry().right(), pos.y()));
}

void SnippingArea::drawCursorInfo(QPainter& painter) const
{
    auto pos = QCursor::pos();
    painter.setPen(QPen(Qt::red, 1));
    auto drawPos = calculateInfoTextPosition(pos);
    if (mMouseIsDown) {
        painter.drawText(drawPos, createInfoText(mCaptureArea.width(), mCaptureArea.height()));
    } else {
        painter.drawText(drawPos, createInfoText(pos.x(), pos.y()));
    }
}

