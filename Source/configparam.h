#ifndef CONFIGPARAM_H
#define CONFIGPARAM_H

#include <QWidget>
#include <QShowEvent>
#include <QCloseEvent>
#include "protocol.h"
#include <vector>
#include <QCheckBox>

namespace Ui {
class ConfigParam;
}

class ConfigParam : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigParam(QWidget *parent = nullptr);
    void setParam(client_config_t *client_config);
    client_config_t getParam();
    ~ConfigParam();

signals:
    void subWindowOpened();
    void subWindowClosed();
    void cfgParamConfirmed();

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;

private slots:

    void on_pushButton_cancel_clicked();

    void on_pushButton_confirm_clicked();

private:
    Ui::ConfigParam *ui;
    std::vector<QCheckBox*> checkBoxVector;
};

#endif // CONFIGPARAM_H
