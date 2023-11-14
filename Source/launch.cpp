#include "launch.h"
#include "ui_launch.h"
#include "mainwindow.h"

launch::launch(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::launch)
{
    ui->setupUi(this);
}

launch::~launch()
{
    delete ui;
}

/*
 * VFT任务 App
*/
void launch::on_btn_vftTask_clicked()
{
    entryApp = new MainWindow(nullptr, VFT_TASK);

    this->close();
    entryApp->resize(500,700);
    entryApp->setWindowTitle("VFT Device Manager");
    entryApp->show();

}

/*
 * 直接采集 App
*/
void launch::on_btn_directSample_clicked()
{
    entryApp = new MainWindow(nullptr, DIRECT_SAMPLE);

    this->close();
    entryApp->resize(500,700);
    entryApp->setWindowTitle("SAMPLE Device Manager");
    entryApp->show();
}

