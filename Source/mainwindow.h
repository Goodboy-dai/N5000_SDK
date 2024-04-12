#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "protocol.h"
#include <QTimer>
#include "configparam.h"
#include <QHBoxLayout>
#include <QTableWidget>
#include "devtable.h"
#include "waveplot.h"
#include "vftflow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum STATUS_TYPE{
    READY,
    SAMPLING,
    LOG
};

enum TASK_TYPE{
    DIRECT_SAMPLE,
    VFT_TASK
};

struct dev_list_t{
    std::string Sn;
    std::string clientIP;
    uint32_t battery;
    std::vector<std::vector<float>> mpu_data;
    client_config_t param;
    //和画图相关
    WavePlot* wave_ploter;
    STATUS_TYPE status;
    //和测试任务相关
    vftFlow* vftFlowWindow;
    TASK_TYPE taskType; //表格菜单根据task_type显示
    QStringList testWords;
    std::string testName;
};

struct cmdqueue_t{
    std::string Sn;
    std::string cmd_data;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr, TASK_TYPE appType = DIRECT_SAMPLE);
    ~MainWindow();
    void timer_poll();
    void plot_poll();

protected:
    virtual void closeEvent(QCloseEvent *event) override;
private slots:

    void on_subWindowOpen();
    void on_subWindowClose();
    void on_cfgParamConfirmedCb();

    void on_plotRequested();
    void on_openTaskWindowRequested();
    void on_cfgParamRequested();
    void on_startSampleRequested();
    void on_stopSampleRequested();
    void on_saveFileRequested();
    void on_openFileRequested();
    void on_addMarkerRequested();
    void on_closeDeviceRequested();

    void on_vftTaskStartRequested(vftFlow::sampleSig_t param);
    void on_vftTaskStopRequested(vftFlow::sampleSig_t param);
    void on_vftTaskSetMarkerRequested(vftFlow::sampleSig_t param);
private:
    TASK_TYPE runType;

    Ui::MainWindow *ui;
    brain_server server;
    QTimer *p_timer_poll;
    QTimer *p_plot_poll;
    bool run_flag = true;
    CircularQueue<cmdqueue_t> cmdqueue{50};
    std::map<std::string, dev_list_t> dev_list;

    ConfigParam *cfg_param;
    //devtable
    DevTable *m_dev_table;

    QStringList questionList;
    void loadQuestion();
    void write2Mark(QString fileNamePrefix, QString markFileName, dev_list_t dev2Save);
    void write2RawData(QString fileNamePrefix, QString rawFileName, dev_list_t dev2Save);
};
#endif // MAINWINDOW_H
