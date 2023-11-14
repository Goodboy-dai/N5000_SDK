#ifndef LAUNCH_H
#define LAUNCH_H

#include <QWidget>
#include "mainwindow.h"
namespace Ui {
class launch;
}

class launch : public QWidget
{
    Q_OBJECT

public:
    explicit launch(QWidget *parent = nullptr);
    ~launch();

private slots:
    void on_btn_vftTask_clicked();

    void on_btn_directSample_clicked();

private:
    Ui::launch *ui;
    MainWindow *entryApp;
};

#endif // LAUNCH_H
