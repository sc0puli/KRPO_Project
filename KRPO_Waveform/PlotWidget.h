#pragma once

#include <QWidget>

#include "Plot.h"

class PlotWidget : public QWidget {
public:
    PlotWidget(QWidget* parent);
    void showGraph(const Plot* _graph, const int _color);
    void fit();

protected:
    void updateGraphData();
    void updateViewData();
    QPointF toLocal(QPointF p);
    QPointF toGlobal(QPointF p);
    QPointF toPlotCoords(QPointF p);
    QPointF toWidgetCoords(QPointF p);
    void drawGrid(float start, float end, bool vertical, bool labels, QPainter& painter, QPen& pen, QPen& penLabel);
    void paintEvent(QPaintEvent* event) override;
    void zoomToRect(QPointF _p1, QPointF _p2);
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    const Plot* graph = nullptr;

    bool zooming = false;
    bool panning = false;

    QPoint lastMousePos;
    QPoint currentMousePos = { 0, 0 };

    QPointF offsetCenterLocal = { 0, 0 };
    QPointF scaleLocal = { 1.5, 1.5 };

    QRect plotArea;
    QRect legendVertical;
    QRect legendHorizontal;

    double xFitScale;
    double yFitScale;

    double xMin = 0;
    double xMax = 0;
    double yMin = 0;
    double yMax = 0;

    int leftMargin = 90;
    int bottomMargin = 40;
    int margin = 20;
    int fontSize = 10;
    double lineThickness = 1.5;
    int gridX = 10;
    int gridY = 20;

    int penColor = 7;
};
