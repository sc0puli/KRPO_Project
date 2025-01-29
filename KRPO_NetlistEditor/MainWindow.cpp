#include "mainwindow.h"
#include "codeeditor.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QTextStream>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // Инициализация виджетов
    createWidgets();
    createMenus();
    createConnections();
    onNewFile();

    simulatorProcess = new QProcess(this);
    postprocessorProcess = new QProcess(this);
    spiceHighlighter = new SpiceHighlighter(codeEditor->document());

    connect(simulatorProcess, &QProcess::started, this, &MainWindow::onSimulatorStarted);
    connect(simulatorProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::onSimulatorFinished);
    connect(simulatorProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::onSimulatorOutput);
    connect(simulatorProcess, &QProcess::readyReadStandardError, this, &MainWindow::onSimulatorError);

    connect(postprocessorProcess, &QProcess::started, this, &MainWindow::onPostprocessorStarted);
    connect(postprocessorProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::onPostprocessorFinished);
    connect(postprocessorProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::onPostprocessorOutput);
    connect(postprocessorProcess, &QProcess::readyReadStandardError, this, &MainWindow::onPostprocessorError);

    setWindowTitle("KRPO SPICE Editor");
    resize(800, 600);
}

void MainWindow::createWidgets() {
    // Главный контейнер
    mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Левая панель (древовидный список)
    treeWidget = new QTreeWidget();
    treeWidget->setHeaderHidden(true);

    // Правая панель (редактор + лог)
    QSplitter* rightSplitter = new QSplitter(Qt::Vertical);

    codeEditor = new CodeEditor();
    logTextEdit = new QTextEdit();
    logTextEdit->setReadOnly(true);

    rightSplitter->addWidget(codeEditor);
    rightSplitter->addWidget(logTextEdit);

    mainSplitter->addWidget(treeWidget);
    mainSplitter->addWidget(rightSplitter);

    // Настройка пропорций
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 3);

    setCentralWidget(mainSplitter);
}

void MainWindow::createMenus() {
    // Создание меню
    QMenuBar* menuBar = new QMenuBar(this);

    // Меню File
    fileMenu = menuBar->addMenu("&File");
    actNew = fileMenu->addAction("&New");
    actOpen = fileMenu->addAction("&Open");
    actSave = fileMenu->addAction("&Save");
    actSaveAs = fileMenu->addAction("Save &As");
    fileMenu->addSeparator();
    actExit = fileMenu->addAction("E&xit");

    // Меню Run
    runMenu = menuBar->addMenu("&Run");
    actRunNetlist = runMenu->addAction("Run &Netlist");
    actRunPostprocessor = runMenu->addAction("Run &Postprocessor");

    // Меню Help
    helpMenu = menuBar->addMenu("&Help");
    actAbout = helpMenu->addAction("&About");

    setMenuBar(menuBar);
}

void MainWindow::createConnections() {
    // Соединение сигналов и слотов
    connect(actNew, &QAction::triggered, this, &MainWindow::onNewFile);
    connect(actOpen, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(actSave, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(actSaveAs, &QAction::triggered, this, &MainWindow::onSaveAsFile);
    connect(actExit, &QAction::triggered, qApp, &QApplication::quit);
    connect(actRunNetlist, &QAction::triggered, this, &MainWindow::onRunNetlist);
    connect(actRunPostprocessor, &QAction::triggered, this, &MainWindow::onRunPostprocessor);
    connect(actAbout, &QAction::triggered, this, &MainWindow::onAbout);

  
}

void MainWindow::onNewFile() {
    codeEditor->clear();
    currentFilePath.clear();
    treeWidget->clear();
    logTextEdit->clear();
    setWindowTitle("SPICE Editor - New File");
}

void MainWindow::onOpenFile() {
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open SPICE Netlist",
        "",
        "SPICE Files (*.sp);;All Files (*)"
    );

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to open file: " + file.errorString());
        return;
    }

    QTextStream in(&file);
    codeEditor->setPlainText(in.readAll());
    currentFilePath = filePath;
    setWindowTitle("SPICE Editor - " + QFileInfo(filePath).fileName());

    // Обновляем древовидный список
    parseNetlistAndUpdateTree(codeEditor->toPlainText());
    populateTreeWidget(codeEditor->toPlainText());
}

void MainWindow::onSaveFile() {
    if (currentFilePath.isEmpty()) {
        onSaveAsFile();
        return;
    }

    QFile file(currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to save file: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    out << codeEditor->toPlainText();
    logTextEdit->append("File saved: " + currentFilePath);
}

void MainWindow::onSaveAsFile() {
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save SPICE Netlist",
        "",
        "SPICE Files (*.sp);;All Files (*)"
    );

    if (filePath.isEmpty()) return;

    // Добавляем расширение, если его нет
    if (!filePath.endsWith(".sp")) {
        filePath += ".sp";
    }

    currentFilePath = filePath;
    onSaveFile();
    setWindowTitle("SPICE Editor - " + QFileInfo(filePath).fileName());
}

// Вспомогательный метод для парсинга нетлиста
void MainWindow::parseNetlistAndUpdateTree(const QString& content) {
    treeWidget->clear();
    QTreeWidgetItem* root = new QTreeWidgetItem(treeWidget);
    root->setText(0, QFileInfo(currentFilePath).fileName());

    // Группируем элементы по типам
    QMap<QString, QTreeWidgetItem*> categories;
    QStringList lines = content.split('\n');

    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith('*')) continue;

        QStringList parts = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (parts.size() < 2) continue;

        QString type = parts[0].left(1).toUpper();
        QString name = parts[0];

        // Создаем категории
        if (!categories.contains(type)) {
            QString categoryName;
            if (type == "R") categoryName = "Resistors";
            else if (type == "C") categoryName = "Capacitors";
            else if (type == "L") categoryName = "Inductors";
            else if (type == "V") categoryName = "Voltage Sources";
            else categoryName = "Other Components";

            categories[type] = new QTreeWidgetItem(root);
            categories[type]->setText(0, categoryName);
        }

        // Добавляем элемент в категорию
        QTreeWidgetItem* item = new QTreeWidgetItem(categories[type]);
        item->setText(0, name);
    }

    treeWidget->expandAll();
}

void MainWindow::populateTreeWidget(const QString& content) {
    treeWidget->clear();
    QTreeWidgetItem* rootItem = new QTreeWidgetItem(treeWidget);
    rootItem->setText(0, QFileInfo(currentFilePath).fileName());

    // Основные категории
    QMap<QString, QTreeWidgetItem*> categories = {
        {"R", createCategoryItem("Resistors")},
        {"C", createCategoryItem("Capacitors")},
        {"L", createCategoryItem("Inductors")},
        {"V", createCategoryItem("Voltage Sources")},
        {"I", createCategoryItem("Current Sources")},
        {".", createCategoryItem("Directives")},
    };

    // Парсинг содержимого
    QStringList lines = content.split('\n');
    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty()) continue;

        // Обработка директив
        if (trimmedLine.startsWith('.')) {
            addDirectiveItem(categories["."], trimmedLine);
            continue;
        }

        // Обработка элементов
        QStringList tokens = trimmedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (tokens.size() < 2) continue;

        QString prefix = tokens[0].left(1).toUpper();
        QString elementName = tokens[0];

        if (categories.contains(prefix)) {
            QTreeWidgetItem* item = new QTreeWidgetItem(categories[prefix]);
            item->setText(0, elementName);
            item->setToolTip(0, trimmedLine);
        }
        else {
            QTreeWidgetItem* otherItem = new QTreeWidgetItem(categories["X"]);
            otherItem->setText(0, elementName);
            otherItem->setToolTip(0, "Unknown component type");
        }
    }

    treeWidget->expandAll();
}

QTreeWidgetItem* MainWindow::createCategoryItem(const QString& categoryName) {
    QTreeWidgetItem* category = new QTreeWidgetItem(treeWidget->topLevelItem(0));
    category->setText(0, categoryName);
    category->setFlags(Qt::ItemIsEnabled);
    return category;
}

void MainWindow::addDirectiveItem(QTreeWidgetItem* category, const QString& directive) {
    QString cleanDirective = directive.simplified();
    QTreeWidgetItem* item = new QTreeWidgetItem(category);
    item->setText(0, cleanDirective.split(' ').first());
    item->setToolTip(0, cleanDirective);
    item->setForeground(0, QColor(Qt::darkGreen));
}


void MainWindow::onRunNetlist()
{
    if (currentFilePath.isEmpty()) {
        appendLog("Error: No netlist loaded!", Qt::red);

        return;
    }
    runSimulator();
}

void MainWindow::runSimulator() {
    if (currentFilePath.isEmpty()) return;

    simulatorProcess->setProgram("../../KRPO_Simulator/bin/KRPO_Simulator.exe");
    simulatorProcess->setArguments({ "--input", currentFilePath });
    simulatorProcess->start();

    appendLog("Simulator started...", Qt::blue);

}

void MainWindow::onRunPostprocessor()
{
    if (currentFilePath.isEmpty()) {
        appendLog("Error: No netlist loaded!", Qt::red);

        return;
    }
    runPostprocessor();
}

void MainWindow::runPostprocessor() {
    if (currentFilePath.isEmpty()) return;

    QString fixedFileExtension;

    if (currentFilePath.endsWith(".sp")) {
        fixedFileExtension = currentFilePath.left(currentFilePath.size() - 3) + ".psf";
    }
    
    postprocessorProcess->setProgram("../../KRPO_Waveform/bin/KRPO_Waveform.exe");
    postprocessorProcess->setArguments({ fixedFileExtension });
    postprocessorProcess->start();

    appendLog("Postprocessor started...", Qt::blue);
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About", "SPICE Editor v0.1");
}

void MainWindow::onSimulatorStarted() {
    appendLog("[Simulator] Process started", Qt::darkBlue);
    actRunNetlist->setEnabled(false);
}

void MainWindow::onSimulatorFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    const QString status = exitStatus == 0 ? "normally" : "crash";
    appendLog(QString("[Simulator] Exited %1 (code: %2)").arg(status).arg(exitCode),
        exitCode == 0 ? Qt::darkGreen : Qt::red);

    actRunNetlist->setEnabled(true);

    if (exitCode == 0) {
        runPostprocessor();
    }
}

void MainWindow::onSimulatorOutput() {
    QString output = QString::fromLocal8Bit(simulatorProcess->readAllStandardOutput());
    appendLog("[Simulator] " + output.trimmed(), Qt::black);
}

void MainWindow::onSimulatorError() {
    QString error = QString::fromLocal8Bit(simulatorProcess->readAllStandardError());
    appendLog("[Simulator Error] " + error.trimmed(), Qt::red);
}

// Реализация обработчиков для постпроцессора
void MainWindow::onPostprocessorStarted() {
    appendLog("[Postprocessor] Process started", Qt::darkBlue);
    actRunPostprocessor->setEnabled(false);
}

void MainWindow::onPostprocessorFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    const QString status = exitStatus == 0 ? "normally" : "crash";
    appendLog(QString("[Postprocessor] Exited %1 (code: %2)").arg(status).arg(exitCode),
        exitCode == 0 ? Qt::darkGreen : Qt::red);

    actRunPostprocessor->setEnabled(true);
}

void MainWindow::onPostprocessorOutput() {
    QString output = QString::fromLocal8Bit(postprocessorProcess->readAllStandardOutput());
    appendLog("[Postprocessor] " + output.trimmed(), Qt::black);
}

void MainWindow::onPostprocessorError() {
    QString error = QString::fromLocal8Bit(postprocessorProcess->readAllStandardError());
    appendLog("[Postprocessor Error] " + error.trimmed(), Qt::red);
}

// Вспомогательный метод для вывода в лог
void MainWindow::appendLog(const QString& message, const QColor& color) {
    QString html = QString("<span style='color:%1;'>%2</span>")
        .arg(color.name())
        .arg(message.toHtmlEscaped());

    logTextEdit->moveCursor(QTextCursor::End);
    logTextEdit->insertHtml(html + "<br>");
    logTextEdit->moveCursor(QTextCursor::End);
}

MainWindow::~MainWindow() {
}