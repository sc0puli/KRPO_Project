#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class DrawingWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void openFile();
    void saveFileAs();
    void setMetal();
    void setPoly();

private:
    void createMenus();
    DrawingWidget *drawingWidget;
};

#endif // MAINWINDOW_H