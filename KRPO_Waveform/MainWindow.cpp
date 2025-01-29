#include <QDockWidget>
#include <QMenuBar>
#include <QFileDialog>

#include <iostream>
#include <fstream>
#include <sstream>

#include "MainWindow.h"

MainWindow::MainWindow() : QMainWindow(nullptr), viewpointWidget(this), plotWidget(this) {
    resize(900, 700);
    setWindowTitle("KRPO Waveform");

    initMenuBar();
    initLogWidget();
    initViewpointWidget();

    setCentralWidget(&plotWidget);
}

void MainWindow::showGraph(const Plot* graph, const int color) {
    plotWidget.showGraph(graph, color);
}

void MainWindow::initMenuBar() {
    QMenu* fileMenu = menuBar()->addMenu("&File");
    QMenu* viewMenu = menuBar()->addMenu("&Zoom");

    QAction* file_openAct  = fileMenu->addAction("&Open");
    QAction* file_closeAct = fileMenu->addAction("&Exit");
    QAction* view_fitAct   = viewMenu->addAction("&Reset");

    connect(file_openAct, &QAction::triggered, this, &MainWindow::file_openFunc);
    connect(file_closeAct, &QAction::triggered, this, &MainWindow::file_closeFunc);
    connect(view_fitAct, &QAction::triggered, this, &MainWindow::view_fitFunc);
}

void MainWindow::view_fitFunc() {
    plotWidget.fit();
}

void MainWindow::file_openFunc() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open File",
        "../KRPO_Simulator/bin/netlists/",
        "PSF Files (*.psf; *.tran);;All Files (*.*)");

    if (!fileName.isEmpty()) {
        logWidget.insertHtml(QString("<b>[INFO]</b> Loading plots from %1.<br>").arg(fileName));
        loadPlots(fileName);
    }
}

void MainWindow::loadPlots(const QString& fileName) {
    plots = readPSF(fileName.toStdString());

    viewpointWidget.clear();

    if (plots.empty()) {
        logWidget.insertHtml(QString("<b>[ERROR]</b> Failed to load file %1.<br>\
                                    File is corrupted.<br>").arg(fileName));
    }
    else {
        logWidget.insertHtml(QString("<b>[INFO]</b> Successfully loaded file %1<br>").arg(fileName));
        logWidget.insertHtml(QString("<b>[INFO]</b> Found %1 viewpoints<br>").arg(plots.size()));
        for (const auto& graph : plots) {
            viewpointWidget.addItem(QString::fromStdString(graph.name));
        }
        if (!plots.empty()) {
            auto firstGraph = plots.begin();
            showGraph(&(*firstGraph), Qt::red);
        }
    }
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

std::vector<Plot> MainWindow::readPSF(const std::string& fileName) {
    std::vector<Plot> out;

    std::ifstream file(fileName);
    if (!file.is_open()) return out;

    // Проверка шапки файла
    const std::vector<std::string> expectedHeader = {
        "HEADER",
        "\"PSFversion\" \"1.00\"",
        "\"simulator\" \"HSPICE\"",
        "\"runtype\" \"Transient Analysis\"",
        "TYPE",
        "\"node\" FLOAT DOUBLE PROP (",
        "\"key\" \"node\"",
        ")",
        "\"branch\" FLOAT DOUBLE PROP (",
        "\"key\" \"branch\"",
        ")",
        "\"sweep\" \"FLOAT DOUBLE\"",
        "SWEEP",
        "\"time\" \"sweep\"",
        "TRACE"
    };

    std::string line;
    for (const auto& expected : expectedHeader) {
        if (!getline(file, line) || trim(line) != expected) {
            file.close();
            return {}; // Возвращаем пустой вектор при несоответствии
        }
    }

    // Чтение данных после заголовка
    double time = 0;
    bool readingValues = false;

    // Чтение секции TRACE
    if (getline(file, line)) {
        std::istringstream tokStream(line);
        std::string token;
        int num = 0;
        tokStream >> token >> token >> num;

        // Чтение описаний трасс
        for (int i = 0; i < num; i++) {
            getline(file, line);
            std::istringstream traceStream(line);
            traceStream >> token;
            out.push_back({ token, std::string(token.begin(), token.begin() + 1), "t", {}, {} });
        }
        readingValues = true;
    }

    // Обработка значений
    while (getline(file, line)) {
        if (line.rfind("\"time\"", 0) == 0) {
            std::istringstream tokStream(line);
            std::string token;
            tokStream >> token >> time;
            continue;
        }

        if (line.rfind("\"group\"", 0) == 0) {
            for (auto& plot : out) {
                getline(file, line);
                std::istringstream tokStream(line);
                double val;
                tokStream >> val;
                plot.x.push_back(time);
                plot.y.push_back(val);
            }
        }
    }

    file.close();
    return out;
}


void MainWindow::file_closeFunc() {
    logWidget.insertHtml(QString("<br><b>[INFO]</b> Closing application.<br>"));
    close();
}

void MainWindow::initLogWidget() {
    auto* dock = new QDockWidget("Log", this);

    logWidget.setReadOnly(true);

    dock->setWidget(&logWidget);
    dock->setMaximumHeight(150);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
}

void MainWindow::initViewpointWidget() {
    auto* dock = new QDockWidget("Viewpoints", this);

    dock->setWidget(&viewpointWidget);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    connect(&viewpointWidget, &QListWidget::currentTextChanged, this, &MainWindow::onViewpointSelected);
}

void MainWindow::onViewpointSelected(const QString& graphName) {
    auto targetName = graphName.toStdString();
    
    int color = Qt::red;
    for (auto& plot : plots) {
        if (plot.name == targetName) {
            showGraph(&plot, color);
            return; // Выход после первого найденного совпадения
        }
        if (color < 17)
            color++;
        else
            color = Qt::red;
    }
}
