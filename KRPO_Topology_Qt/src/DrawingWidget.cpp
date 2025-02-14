#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QMouseEvent>

#include "DrawingWidget.h"

DrawingWidget::DrawingWidget(QWidget *parent) : QWidget(parent) {

    QPalette pal = QPalette();

    pal.setColor(QPalette::Window, Qt::black);

    setAutoFillBackground(true);
    setPalette(pal);
    baseWidth = width() > 0 ? width() : 1;
    baseHeight = height() > 0 ? height() : 1;
}

void DrawingWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(Qt::NoPen);

    painter.setCompositionMode(QPainter::CompositionMode_Plus);

    float scaleX = static_cast<float>(width()) / baseWidth;
    float scaleY = static_cast<float>(height()) / baseHeight;

    for (const Shape &shape : shapes) {
        QColor color = (shape.type == Metal) ? METAL_COLOR : POLY_COLOR;
        painter.setBrush(color);
        QRectF scaledRect(
            shape.rect.x() * scaleX,
            shape.rect.y() * scaleY,
            shape.rect.width() * scaleX,
            shape.rect.height() * scaleY
        );
        painter.drawRect(scaledRect);
    }

    // Временный прямоугольник
    if (drawing) {
        QColor color = (currentMaterial == Metal) ? METAL_COLOR : POLY_COLOR;
        painter.setBrush(color);
        painter.drawRect(tempRect);
    }
}

void DrawingWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    update();
}

void DrawingWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        startPos = event->pos();
        tempRect = QRect(startPos, startPos);
        drawing = true;
        update();
    }
}

void DrawingWidget::mouseMoveEvent(QMouseEvent *event) {
    if (drawing) {
        tempRect = QRectF(startPos, event->pos()).normalized();
        update();
    }
}

void DrawingWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && drawing) {
        drawing = false;
        QRectF currentRect = QRectF(startPos, event->pos()).normalized();

        // Масштабирование в базовые координаты
        float scaleX = static_cast<float>(baseWidth) / width();
        float scaleY = static_cast<float>(baseHeight) / height();

        QRectF baseRect(
            currentRect.x() * scaleX,
            currentRect.y() * scaleY,
            currentRect.width() * scaleX,
            currentRect.height() * scaleY
        );

        if (!baseRect.isEmpty()) {
            shapes.append({baseRect, currentMaterial});
            update();
        }
    }
}

bool DrawingWidget::loadFromFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    shapes.clear();
    QTextStream in(&file);
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.startsWith("REC")) continue;

        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() != 6) continue;

        bool ok;
        // Парсим целочисленные значения
        int x = parts[1].toInt(&ok);
        if (!ok) continue;
        int y = parts[2].toInt(&ok);
        if (!ok) continue;
        int width = parts[3].toInt(&ok);
        if (!ok || width <= 0) continue;
        int height = parts[4].toInt(&ok);
        if (!ok || height <= 0) continue;

        // Определяем тип материала
        MaterialType type;
        if (parts[5] == "METAL") type = Metal;
        else if (parts[5] == "POLY") type = Poly;
        else continue;

        shapes.append({QRectF(x, y, width, height), type});
    }

    update();
    return true;
}

bool DrawingWidget::saveToFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    for (const Shape &shape : shapes) {
        QString typeStr = (shape.type == Metal) ? "METAL" : "POLY";
        out << "REC "
            << qRound(shape.rect.x()) << " "
            << qRound(shape.rect.y()) << " "
            << qRound(shape.rect.width()) << " "
            << qRound(shape.rect.height()) << " "
            << typeStr << "\n";
    }

    return true;
}