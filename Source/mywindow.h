#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <QWidget>

class MyWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MyWindow(QWidget *parent = nullptr);

protected:

    virtual void showEvent(QShowEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;

signals:
    void windowShowSig();
    void windowCloseSig();
};

#endif // MYWINDOW_H
