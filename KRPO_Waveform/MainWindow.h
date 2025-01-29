#pragma once

#include <QApplication>
#include <QMainWindow>
#include <QListWidget>
#include <QTextEdit>

#include "Plot.h"
#include "PlotWidget.h"

class MainWindow : public QMainWindow {
public:
    MainWindow();
    void loadPlots(const QString& fileName);

private:
    QListWidget viewpointWidget;
    PlotWidget plotWidget;
    QTextEdit logWidget;

    std::vector<Plot> plots;

    void initMenuBar();
    void initLogWidget();
    void initViewpointWidget();

    void showGraph(const Plot* graph, const int color);
    std::vector<Plot> readPSF(const std::string& fileName);

private slots:
    void view_fitFunc();
    void file_openFunc();
    void file_closeFunc();
    void onViewpointSelected(const QString& graphName);
};