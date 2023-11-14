#include "mywindow.h"
#include "qevent.h"

MyWindow::MyWindow(QWidget *parent)
    : QWidget{parent}
{

}

/*
 * 重写的打开窗口时间，产生信号
*/
void MyWindow::showEvent(QShowEvent *event){
    emit windowShowSig();
    //std::cout << "show" << std::endl;
    QWidget::showEvent(event);
    //event->accept();
}

/*
 * 重写的关闭窗口事件
*/
void MyWindow::closeEvent(QCloseEvent *event){
    emit windowCloseSig();
    //std::cout << "close" << std::endl;
    event->accept();
}
