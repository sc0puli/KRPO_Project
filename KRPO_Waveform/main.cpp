#include <QApplication>
#include <QMainWindow>

#include <iostream>

#include "MainWindow.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	MainWindow mainWindow;

	if (argc>1)
	{
		mainWindow.loadPlots(argv[1]);
	}

	mainWindow.show();

	return app.exec();
}