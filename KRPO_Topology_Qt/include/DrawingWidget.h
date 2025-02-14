#ifndef DRAWINGWIDGET_H
#define DRAWINGWIDGET_H

#include <QWidget>
#include <QList>
#include <QRect>

class DrawingWidget : public QWidget { 
    Q_OBJECT
public:
    enum MaterialType { 
        Metal, 
        Poly 
    };

    explicit DrawingWidget(QWidget *parent = nullptr);
    void setCurrentMaterial(MaterialType type) { currentMaterial = type; }
    bool loadFromFile(const QString &fileName);
    bool saveToFile(const QString &fileName);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    struct Shape {
        QRectF rect;
        MaterialType type;
    };

    QList<Shape> shapes;
    MaterialType currentMaterial = Metal;
    QPoint startPos;
    QRectF tempRect;
    bool drawing = false;
    int baseWidth = 1;
    int baseHeight = 1;
    const QColor METAL_COLOR = QColor(0, 0, 255, 255);
    const QColor POLY_COLOR = QColor(255, 0, 0, 255);
};

#endif // DRAWINGWIDGET_H