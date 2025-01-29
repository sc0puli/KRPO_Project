#include <QPainter>
#include <QMouseEvent>

#include <iostream>

#include "PlotWidget.h"

PlotWidget::PlotWidget(QWidget* parent = nullptr) : QWidget(parent) {
    setMinimumSize(400, 300);
    setMouseTracking(true);
}

void PlotWidget::showGraph(const Plot* _graph, const int _color) {
    penColor = _color;
    graph = _graph;
    updateGraphData();
    updateViewData();
    fit();
}

// scaling graphics
void PlotWidget::fit() {
    scaleLocal = { 1, 1 };
    offsetCenterLocal = { (xMin + xMax) / 2.f, (yMin + yMax) / 2.f };
    update();
}

void PlotWidget::updateGraphData() {
    xMin = *std::min_element(graph->x.begin(), graph->x.end());
    xMax = *std::max_element(graph->x.begin(), graph->x.end());
    yMin = *std::min_element(graph->y.begin(), graph->y.end());
    yMax = *std::max_element(graph->y.begin(), graph->y.end());
}

void PlotWidget::updateViewData() {
    plotArea = rect().adjusted(leftMargin + margin, margin, -margin, -(margin + bottomMargin));

    xFitScale = ((xMax - xMin == 0) ? 1 : plotArea.width() / (xMax - xMin));
    yFitScale = ((yMax - yMin == 0) ? 1 : plotArea.height() / (yMax - yMin));

    legendVertical = rect().adjusted(0, 0, -plotArea.width() - margin, 0);
    legendHorizontal = rect().adjusted(plotArea.x(), plotArea.height() + margin, 0, 0);
}

// pixels to values
QPointF PlotWidget::toLocal(QPointF p) {
    auto x = (p.x() / (xFitScale * scaleLocal.x())) + offsetCenterLocal.x();
    auto y = (p.y() / (yFitScale * scaleLocal.y())) + offsetCenterLocal.y();
    return { x, y };
}

// values to pixels
QPointF PlotWidget::toGlobal(QPointF p) {
    double x = (p.x() - offsetCenterLocal.x()) * (scaleLocal.x() * xFitScale);
    double y = (p.y() - offsetCenterLocal.y()) * (scaleLocal.y() * yFitScale);
    return { x, y };
}

// pixels to pixels
QPointF PlotWidget::toPlotCoords(QPointF p) {
    p = { p.x() - plotArea.left(), p.y() - plotArea.bottom() };
    p.setY(-p.y());
    p = { p.x() - plotArea.width() / 2.f, p.y() - plotArea.height() / 2.f };
    return p;
}

// pixels to pixels
QPointF PlotWidget::toWidgetCoords(QPointF p) {
    p = { plotArea.width() / 2.f + p.x(), plotArea.height() / 2.f + p.y() };
    p.setY(-p.y());
    p = { plotArea.left() + p.x(), plotArea.bottom() + p.y() };
    return p;
}

void PlotWidget::drawGrid(float start, float end, bool vertical, bool labels, QPainter& painter, QPen& pen, QPen& penLabel) {
    auto powTmp = std::log10(end - start);
    auto power = int(powTmp > 0 ? powTmp : powTmp - 1);
    auto delta = std::pow(10, power);
    auto startStep = int(start * std::pow(10, -power)) * std::pow(10, power);

    for (int i = -10; i < 10; i++) {
        auto localPos = startStep + (delta * i);
        QString label = QString::number(localPos, 'e', 1);
        auto globalPos = vertical ? toWidgetCoords(toGlobal({ 0, localPos })).y() : toWidgetCoords(toGlobal({ localPos, 0 })).x();

        if (vertical) {
            if (globalPos <= plotArea.bottom() && globalPos >= plotArea.top()) {
                painter.setPen(pen);
                painter.drawLine(plotArea.left(), int(globalPos), plotArea.right(), int(globalPos));
                if (labels) {
                    painter.setPen(penLabel);
                    painter.drawText(plotArea.left() - leftMargin, int(globalPos), label);
                }
            }
        }
        else {
            if (globalPos >= plotArea.left() && globalPos <= plotArea.right()) {
                painter.setPen(pen);
                painter.drawLine(int(globalPos), plotArea.bottom(), int(globalPos), plotArea.top());
                if (labels) {
                    painter.setPen(penLabel);
                    painter.drawText(int(globalPos), plotArea.bottom() + (margin + bottomMargin) / 2.f, label);
                }
            }
        }
    }
}

void PlotWidget::paintEvent(QPaintEvent* event) {
    if (!graph || !(*graph)) {
        return;
    }

    updateViewData();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(plotArea, Qt::black);
    painter.setClipRect(plotArea.adjusted(-2, -2, 2, 2));

    // Draw the data points
    painter.setPen(QPen(Qt::GlobalColor(penColor), lineThickness));
    QPointF lastPoint;
    for (size_t i = 0; i < graph->x.size(); ++i) {
        QPointF currentPoint = toGlobal({ graph->x[i], graph->y[i] });
        if (i > 0) {
            painter.drawLine(toWidgetCoords(lastPoint), toWidgetCoords(currentPoint));
        }
        lastPoint = currentPoint;
    }

    // Draw axis labels
    painter.setPen(QPen(Qt::white, 2, Qt::SolidLine));
    auto center = toWidgetCoords(toGlobal({ 0, 0 })).toPoint();
    painter.drawLine(QPoint{ plotArea.left(), center.y() }, QPoint{ plotArea.right(), center.y() });
    painter.drawLine(QPoint{ center.x(), plotArea.top() }, QPoint{ center.x(), plotArea.bottom() });

    painter.setClipping(false);

    // Draw axis
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(plotArea.left(), plotArea.bottom(), plotArea.right(), plotArea.bottom());
    painter.drawLine(plotArea.left(), plotArea.top(), plotArea.left(), plotArea.bottom());

    painter.setPen(QPen(Qt::black));
    painter.setFont(QFont("Arial", fontSize));

    auto startLocal = toLocal({ 0, 0 });
    auto endLocal = toLocal({ (float)plotArea.width(), (float)plotArea.height() });

    QPen gridPen1(Qt::lightGray, 1, Qt::DashLine);
    QPen labelPen(Qt::black);

    drawGrid(startLocal.x(), endLocal.x(), false, true, painter, gridPen1, labelPen);
    drawGrid(startLocal.y(), endLocal.y(), true, true, painter, gridPen1, labelPen);
}

// plot coordinates
void PlotWidget::zoomToRect(QPointF _p1, QPointF _p2) {
    auto p1 = toLocal(_p1);
    auto p2 = toLocal(_p2);

    QPointF localMin = { std::min(p1.x(), p2.x()), std::min(p1.y(), p2.y()) };
    QPointF localMax = { std::max(p1.x(), p2.x()), std::max(p1.y(), p2.y()) };

    scaleLocal.setX(((xMax - xMin) / (localMax.x() - localMin.x())));
    scaleLocal.setY(((yMax - yMin) / (localMax.y() - localMin.y())));

    offsetCenterLocal.setX((localMax.x() + localMin.x()) / 2.f);
    offsetCenterLocal.setY((localMax.y() + localMin.y()) / 2.f);
}

void PlotWidget::mousePressEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::MiddleButton) {
        panning = true;
    }
    lastMousePos = event->pos();
}

void PlotWidget::mouseReleaseEvent(QMouseEvent* event) {
    panning = false;
    update();
}

void PlotWidget::mouseMoveEvent(QMouseEvent* event) {
    currentMousePos = event->pos();

    if (event->buttons() & Qt::MiddleButton) {
        QPointF delta = toLocal(toPlotCoords(currentMousePos)) - toLocal(toPlotCoords(lastMousePos));
        lastMousePos = currentMousePos;
        offsetCenterLocal -= delta;
    }

    update();
}

void PlotWidget::wheelEvent(QWheelEvent* event) {
    constexpr qreal zoomFactor = 1.1;

    bool vertical = legendVertical.contains(currentMousePos);
    bool horizontal = legendHorizontal.contains(currentMousePos);

    if (event->angleDelta().y() > 0) {
        if (horizontal || !vertical) scaleLocal.setX(scaleLocal.x() * zoomFactor);
        if (vertical || !horizontal) scaleLocal.setY(scaleLocal.y() * zoomFactor);
    }
    else if (event->angleDelta().y() < 0) {
        if (horizontal || !vertical) scaleLocal.setX(scaleLocal.x() / zoomFactor);
        if (vertical || !horizontal) scaleLocal.setY(scaleLocal.y() / zoomFactor);
    }
    update();
}
