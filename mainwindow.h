#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include "transfer_func1d.h"
#include <vector_types.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_actionRead_Data_triggered();

    void on_Draw_PushButton_clicked();

    void on_actionDistRead_Data_triggered();

private:
    Ui::MainWindow *ui;
	QString DataDir;
	Transfer_Func1D *tr1D;
	GLWidget *glw;
};

#endif // MAINWINDOW_H
