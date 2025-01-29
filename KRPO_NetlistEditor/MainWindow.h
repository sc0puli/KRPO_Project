#pragma once

#include <QMainWindow>
#include <QTreeWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QMenuBar>
#include <QStatusBar>
#include <QProcess>

#include "Highlighter.h"
#include "CodeEditor.h"

class MainWindow : public QMainWindow {
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveAsFile();
    void onRunNetlist();
    void onRunPostprocessor();
    void onAbout();
    void onSimulatorStarted();
    void onSimulatorFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onSimulatorOutput();
    void onSimulatorError();
    void onPostprocessorStarted();
    void onPostprocessorFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onPostprocessorOutput();
    void onPostprocessorError();

private:
    void createMenus();
    void createWidgets();
    void createConnections();
    void parseNetlistAndUpdateTree(const QString& content);
    void populateTreeWidget(const QString& content);
    void addDirectiveItem(QTreeWidgetItem* category, const QString& directive);
    void appendLog(const QString& message, const QColor& color = Qt::black);
    void runSimulator();
    void runPostprocessor();

    // Виджеты
    QSplitter* mainSplitter;
    QTreeWidget* treeWidget;
    CodeEditor* codeEditor;
    QTextEdit* logTextEdit;
    QTreeWidgetItem* createCategoryItem(const QString& categoryName);
    SpiceHighlighter* spiceHighlighter;

    // Процессы
    QProcess* simulatorProcess;
    QProcess* postprocessorProcess;

    // Меню
    QMenu* fileMenu;
    QMenu* runMenu;
    QMenu* helpMenu;

    // Действия
    QAction* actNew;
    QAction* actOpen;
    QAction* actSave;
    QAction* actSaveAs;
    QAction* actExit;
    QAction* actRunNetlist;
    QAction* actRunPostprocessor;
    QAction* actAbout;

    QString currentFilePath;
};
