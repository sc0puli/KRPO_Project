#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

#include "DrawingWidget.h"
#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), drawingWidget(new DrawingWidget(this)) {
    setCentralWidget(drawingWidget);
    createMenus();
    setWindowTitle("KRPO WinAPI, but Qt");
    resize(800, 600);
}

void MainWindow::createMenus() {
    QMenu *fileMenu = menuBar()->addMenu("&File");
    QAction *openAction = fileMenu->addAction("&Open...", this, &MainWindow::openFile);
    openAction->setShortcut(QKeySequence::Open);
    QAction *saveAsAction = fileMenu->addAction("&Save As...", this, &MainWindow::saveFileAs);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction("E&xit", this, &QWidget::close);
    exitAction->setShortcut(QKeySequence::Quit);

    QMenu *drawMenu = menuBar()->addMenu("&Draw");
    QAction *metalAction = drawMenu->addAction("&Metal", this, &MainWindow::setMetal);
    metalAction->setCheckable(true);
    QAction *polyAction = drawMenu->addAction("&Poly", this, &MainWindow::setPoly);
    polyAction->setCheckable(true);

    QActionGroup *drawGroup = new QActionGroup(this);
    drawGroup->addAction(metalAction);
    drawGroup->addAction(polyAction);
    metalAction->setChecked(true);
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Topology File", "",
                                                    "Topology Files (*.txt);;All Files (*)");
    if (!fileName.isEmpty()) {
        if (!drawingWidget->loadFromFile(fileName)) {
            QMessageBox::warning(this, "Error", "Failed to open file.");
        }
    }
}

void MainWindow::saveFileAs() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Topology File", "",
                                                    "Topology Files (*.txt);;All Files (*)");
    if (!fileName.isEmpty()) {
        if (!drawingWidget->saveToFile(fileName)) {
            QMessageBox::warning(this, "Error", "Failed to save file.");
        }
    }
}

void MainWindow::setMetal() {
    drawingWidget->setCurrentMaterial(DrawingWidget::Metal);
}

void MainWindow::setPoly() {
    drawingWidget->setCurrentMaterial(DrawingWidget::Poly);
}