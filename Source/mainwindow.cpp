#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "protocol.h"
#include <ctime> //获取系统时间
#include <QTimer>
#include "configparam.h"
#include "devtable.h"
#include <iostream>

#define DEBUG 0

MainWindow::MainWindow(QWidget *parent, TASK_TYPE appType)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    runType = appType;
    ui->setupUi(this);

    //加载问题
    if(runType == VFT_TASK)
        loadQuestion();

    //初始化配置页面
    cfg_param = new ConfigParam();
    //cfg_param->setParent(this,Qt::Tool);
    //绑定配置页面开启和关闭相关的 slots
    connect(cfg_param, &ConfigParam::subWindowOpened, this, &MainWindow::on_subWindowOpen);
    connect(cfg_param, &ConfigParam::subWindowClosed, this, &MainWindow::on_subWindowClose);
    connect(cfg_param, &ConfigParam::cfgParamConfirmed, this, &MainWindow::on_cfgParamConfirmedCb);

    //初始化设备列表，TODO: 参数是程序运行的模式，直接采集、VFT任务，相应的菜单的样式会变化
    m_dev_table = new DevTable(runType);
    ui->verticalLayout->addWidget(m_dev_table);

    //绑定m_dev_table菜单对应的信号回调函数
    connect(m_dev_table, &DevTable::plotRequested, this, &MainWindow::on_plotRequested);
    connect(m_dev_table, &DevTable::cfgParamRequested, this, &MainWindow::on_cfgParamRequested);
    connect(m_dev_table, &DevTable::startSampleRequested, this, &MainWindow::on_startSampleRequested);
    connect(m_dev_table, &DevTable::stopSampleRequested, this, &MainWindow::on_stopSampleRequested);
    connect(m_dev_table, &DevTable::saveFileRequested, this, &MainWindow::on_saveFileRequested);
    connect(m_dev_table, &DevTable::openFileRequested, this, &MainWindow::on_openFileRequested);
    connect(m_dev_table, &DevTable::openTaskWindowRequested, this, &MainWindow::on_openTaskWindowRequested);
    connect(m_dev_table, &DevTable::addMarkerRequested, this, &MainWindow::on_addMarkerRequested);
    connect(m_dev_table, &DevTable::closeDeviceRequested, this, &MainWindow::on_closeDeviceRequested);

    //协议处理定时器
    p_timer_poll = new QTimer(this);
    p_timer_poll->setInterval(10);
    p_timer_poll->setSingleShot(false);
    connect(p_timer_poll, &QTimer::timeout, this, &MainWindow::timer_poll);
    p_timer_poll->start();

    //画图定时器
    p_plot_poll = new QTimer(this);
    p_plot_poll->setInterval(100);
    p_plot_poll->setSingleShot(false);
    connect(p_plot_poll, &QTimer::timeout, this, &MainWindow::plot_poll);
    p_plot_poll->start();

    //服务端线程开始
    server.start();

/*
 * for debug
*/
#if DEBUG
    m_dev_table->resizeRowsCount(3);
    m_dev_table->setItemText(0,0,"00-00-00-00-00-00");
    m_dev_table->setItemText(1,0,"00-00-00-00-00-01");
    m_dev_table->setItemText(2,0,"00-00-00-00-00-02");

    dev_list["00-00-00-00-00-00"] = {
        .Sn="00-00-00-00-00-00",
                                  .clientIP = "192.168.1.2",
                                  .param={  .sample_rate = 20,
                                            .sample_delay = 1000,
                                            .nburst = 200,
                                            .sample_duration = 600,
                                            .enable_channels = {36,0,18},
                                            .led_strength = {1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023},
                                            .sample_adc_gain = {7,7,7,7,7,7,7,7,7,7,7,7, 7,7,7,7,7,7,7,7,7,7,7,7},
                                            .mpu_ares_gain = 0,
                                            .mpu_gres_gain = 0},
                                            .wave_ploter = new WavePlot(nullptr, "00-00-00-00-00-00", runType),
                                            .status = READY,
                                            .vftFlowWindow = new vftFlow(nullptr, "00-00-00-00-00-00", questionList),
                                            .taskType = runType};
    connect(dev_list["00-00-00-00-00-00"].vftFlowWindow, &vftFlow::startSampleSig, this, &MainWindow::on_vftTaskStartRequested);
    connect(dev_list["00-00-00-00-00-00"].vftFlowWindow, &vftFlow::stopSampleSig, this, &MainWindow::on_vftTaskStopRequested);
    connect(dev_list["00-00-00-00-00-00"].vftFlowWindow, &vftFlow::setSampleMarker, this, &MainWindow::on_vftTaskSetMarkerRequested);
    //图例、窗口名称初始化
    dev_list["00-00-00-00-00-00"].wave_ploter->resizeChannelLength(dev_list["00-00-00-00-00-00"].param.enable_channels);

    dev_list["00-00-00-00-00-01"] = {.Sn="00-00-00-00-00-01", .clientIP = "192.168.1.3",
                                     .param={  .sample_rate = 20,
                                               .sample_delay = 1000,
                                               .nburst = 200,
                                               .sample_duration = 600,
                                               .enable_channels = {36,0,18},
                                               .led_strength = {1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023},
                                               .sample_adc_gain = {7,7,7,7,7,7,7,7,7,7,7,7, 7,7,7,7,7,7,7,7,7,7,7,7},
                                               .mpu_ares_gain = 0,
                                               .mpu_gres_gain = 0},
                                       .wave_ploter = new WavePlot(nullptr, "00-00-00-00-00-01", runType),
                                       .status = READY,
                                       .vftFlowWindow = new vftFlow(nullptr, "00-00-00-00-00-01", questionList),
                                       .taskType = runType};
    connect(dev_list["00-00-00-00-00-01"].vftFlowWindow, &vftFlow::startSampleSig, this, &MainWindow::on_vftTaskStartRequested);
    connect(dev_list["00-00-00-00-00-01"].vftFlowWindow, &vftFlow::stopSampleSig, this, &MainWindow::on_vftTaskStopRequested);
    connect(dev_list["00-00-00-00-00-01"].vftFlowWindow, &vftFlow::setSampleMarker, this, &MainWindow::on_vftTaskSetMarkerRequested);
    //图例初始化
    dev_list["00-00-00-00-00-01"].wave_ploter->resizeChannelLength(dev_list["00-00-00-00-00-01"].param.enable_channels);

    dev_list["00-00-00-00-00-02"] = {.Sn="00-00-00-00-00-00", .clientIP = "192.168.1.4",
                                     .param={  .sample_rate = 20,
                                               .sample_delay = 1000,
                                               .nburst = 200,
                                               .sample_duration = 600,
                                               .enable_channels = {36,0,18},
                                               .led_strength = {1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023},
                                               .sample_adc_gain = {7,7,7,7,7,7,7,7,7,7,7,7, 7,7,7,7,7,7,7,7,7,7,7,7},
                                               .mpu_ares_gain = 0,
                                               .mpu_gres_gain = 0},
                                               .wave_ploter = new WavePlot(nullptr, "00-00-00-00-00-02", runType),
                                               .status = READY,
                                               .vftFlowWindow = new vftFlow(nullptr, "00-00-00-00-00-02", questionList),
                                               .taskType = runType};
    connect(dev_list["00-00-00-00-00-02"].vftFlowWindow, &vftFlow::startSampleSig, this, &MainWindow::on_vftTaskStartRequested);
    connect(dev_list["00-00-00-00-00-02"].vftFlowWindow, &vftFlow::stopSampleSig, this, &MainWindow::on_vftTaskStopRequested);
    connect(dev_list["00-00-00-00-00-02"].vftFlowWindow, &vftFlow::setSampleMarker, this, &MainWindow::on_vftTaskSetMarkerRequested);
    //图例初始化
    dev_list["00-00-00-00-00-02"].wave_ploter->resizeChannelLength(dev_list["00-00-00-00-00-02"].param.enable_channels);

/*
 * debug end
*/
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * mainwindow窗口关闭事件重写，用于关闭其他子窗口，如果有并且还未关闭
*/
void MainWindow::closeEvent(QCloseEvent *event){

    for(auto &it : dev_list){
        if(it.second.vftFlowWindow) it.second.vftFlowWindow->close();
        if(it.second.wave_ploter) it.second.wave_ploter->setWindowVisible(false);
    }
    cfg_param->close();

    event->accept();
}

/*
 * 更新数据,dev_Table显示
 * TODO
 *  设定按钮是否可以click
 *  在timer_poll里面直接追加
 *
 *  debug的时候用于追加虚拟数据
*/
void MainWindow::plot_poll(){
#if DEBUG
/*
 * for debug
*/
//    static int count = 0;

//    for(int i = 0; i < 1; ++i){
//        if(count < 1000){
//            for(int j = 0; j < wave_plot->graphCount(); ++j)
//                wave_plot->append(j,1.0*count/10, rand() % 10);
//        }
//        count++;
//    }



    static int count = 0;
    for(int i = 0; i < 10; ++i){
        if(count < 1000000){
            for (int i = 0; i < dev_list["00-00-00-00-00-00"].wave_ploter->graphCount();i++){
                dev_list["00-00-00-00-00-00"].wave_ploter->append(i ,1.0*count/10, rand() % 10);  // 更新显示（随机生成10以内的一个数）
            }
            for (int i = 0; i < dev_list["00-00-00-00-00-01"].wave_ploter->graphCount();i++){
                dev_list["00-00-00-00-00-01"].wave_ploter->append(i ,1.0*count/10, rand() % 10);  // 更新显示（随机生成10以内的一个数）
            }
            for (int i = 0; i < dev_list["00-00-00-00-00-02"].wave_ploter->graphCount();i++){
                dev_list["00-00-00-00-00-02"].wave_ploter->append(i ,1.0*count/10, rand() % 10);  // 更新显示（随机生成10以内的一个数）
            }
        }
        count++;
    }


/*
 * debug end
*/
#endif

    //dev_table 更新
    //TODO，在进入cfgParam界面时，不更新???
    //检查dev_list，如果和当前dev_table的行数不同，则更新行数
    if(dev_list.size() != m_dev_table->rowCount()){
        //std::cout << "rowCount: " << m_dev_table->rowCount() << " dev_size: " << dev_list.size() << std::endl;
        m_dev_table->resizeRowsCount(dev_list.size());
    }
    int rowIndex = 0;
    for( auto it = dev_list.begin(); it != dev_list.end(); ++it, ++rowIndex){
        //ID
        m_dev_table->setItemText(rowIndex, 0, QString::fromStdString(it->first));
        //IP
        m_dev_table->setItemText(rowIndex, 1, QString::fromStdString(it->second.clientIP));
        //Battery
        m_dev_table->setItemText(rowIndex, 2, QString::number(it->second.battery));
        //Status
        m_dev_table->setItemText(rowIndex, 3, it->second.status == READY ? "READY" : (it->second.status == SAMPLING ? "SAMPLING" : "LOG"));
        //update plot
        it->second.wave_ploter->update();
    }
}

/*
 * 协议层交互轮询器
*/
void MainWindow::timer_poll(){

    if(run_flag){
        //用户操作指令
        //if(!cmdqueue.empty()){
        while(!cmdqueue.empty()){

            cmdqueue_t cmd;
            cmdqueue.pop(cmd);

            if(dev_list[cmd.Sn].status == LOG) continue;

            if(cmd.cmd_data == "start"){
                //如果此时正处于工作状态或LOG则略过
                if(dev_list[cmd.Sn].status == SAMPLING){
                    ui->dialog->append("Device is sampleing now, please stop it first and restart!");
                    continue;
                }
                //清空原来的wave mpu
                for (int i = 0; i < dev_list[cmd.Sn].mpu_data.size();i++)
                    dev_list[cmd.Sn].mpu_data[i].clear();

                //图例初始化 在方法内部清空了原来的WaveView, 不关闭wavewindow
                dev_list[cmd.Sn].wave_ploter->resizeChannelLength(dev_list[cmd.Sn].param.enable_channels);

                //切换设备状态-->SAMPLING
                dev_list[cmd.Sn].status = SAMPLING;
                server.put_cmd(cmd.Sn, "start", dev_list[cmd.Sn].param);

                ui->dialog->append(QString::fromStdString(cmd.Sn) + " start sample.");

            } else if(cmd.cmd_data == "stop"){

                //切换设备状态-->SAMPLING
                dev_list[cmd.Sn].status = READY;
                server.put_cmd(cmd.Sn, "stop");

                ui->dialog->append(QString::fromStdString(cmd.Sn) + " stop sample.");
            }
        }
        //画图
client_proc:

        while(true){
            server_queue_t client_queue_frame = server.get_msg();
            if(client_queue_frame.type != "dummy"){
                //接收到有效type数据
                if(client_queue_frame.type == "connected"){ //设备连接事件 TODO UI text log, config params
                    std::cout << "device: " << client_queue_frame.clientSn << " connected." << std::endl;
                    ui->dialog->append("device: " + QString::fromStdString(client_queue_frame.clientSn) + ": " + QString::fromStdString(client_queue_frame.clientIP) + " connnected.");
                    dev_list[client_queue_frame.clientSn] = {
                                                             .Sn=client_queue_frame.clientSn,
                                                             .clientIP = client_queue_frame.clientIP,
                                                             .param={  .sample_rate = 20,
                                                                       .sample_delay = 1000,
                                                                       .nburst = 200,
                                                                       .sample_duration = 600,
                                                                       .enable_channels = {36,0,18},
                                                                       .led_strength = {1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023},
                                                                       .sample_adc_gain = {4,4,4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4,4,4,4,4,4,4},
                                                                       .mpu_ares_gain = 0,
                                                                       .mpu_gres_gain = 0},
                                                                       .wave_ploter = new WavePlot(nullptr, QString::fromStdString(client_queue_frame.clientSn), runType),
                                                                       .status = READY,
                                                                       .vftFlowWindow = new vftFlow(nullptr, client_queue_frame.clientSn, questionList),
                                                                       .taskType = runType};
                    connect(dev_list[client_queue_frame.clientSn].vftFlowWindow, &vftFlow::startSampleSig, this, &MainWindow::on_vftTaskStartRequested);
                    connect(dev_list[client_queue_frame.clientSn].vftFlowWindow, &vftFlow::stopSampleSig, this, &MainWindow::on_vftTaskStopRequested);
                    connect(dev_list[client_queue_frame.clientSn].vftFlowWindow, &vftFlow::setSampleMarker, this, &MainWindow::on_vftTaskSetMarkerRequested);
                    //图例、窗口名称初始化
                    dev_list[client_queue_frame.clientSn].wave_ploter->resizeChannelLength(dev_list[client_queue_frame.clientSn].param.enable_channels);
//                    for(int i = 0; i < dev_list[client_queue_frame.clientSn].param.enable_channels.size(); ++i){
//                        dev_list[client_queue_frame.clientSn].wave_ploter->setLegendName(i, QString::number(dev_list[client_queue_frame.clientSn].param.enable_channels[i]));
//                    }

                    //初始化wave mpu vector通道 TODO 当合并WaveView后，不需要在这里存储波形数据
                    dev_list[client_queue_frame.clientSn].mpu_data.resize(7);

                    //初始电池电压
                    dev_list[client_queue_frame.clientSn].battery = client_queue_frame.battery;

                } else if(client_queue_frame.type == "disconnected"){
                    std::cout << "device: " << client_queue_frame.clientSn << " disconnected." << std::endl;
                    ui->dialog->append("device: " + QString::fromStdString(client_queue_frame.clientSn) + ": " + QString::fromStdString(client_queue_frame.clientIP) + " disconnnected.");
                    if(dev_list[client_queue_frame.clientSn].vftFlowWindow != nullptr) dev_list[client_queue_frame.clientSn].vftFlowWindow->close();
                    dev_list.erase(client_queue_frame.clientSn);

                } else if(client_queue_frame.type == "done"){
                    std::cout << "device: " << client_queue_frame.clientSn << " sample done." << std::endl;
                    ui->dialog->append("device: " + QString::fromStdString(client_queue_frame.clientSn) + " sample complete.");

                } else if(client_queue_frame.type == "wave"){   //TODO start times
                    std::cout << "device: " << client_queue_frame.clientSn << " wave got." << std::endl;
                    for(int i = 0; i < client_queue_frame.wave_data.size(); i++)    //通道数
                        for(int j = 0; j < client_queue_frame.wave_data[i].size();j++){ //nitem数

                            //添加数据到波形 未进行环境光补偿
                            dev_list[client_queue_frame.clientSn].wave_ploter->append(i,1.0*(client_queue_frame.index+j)/dev_list[client_queue_frame.clientSn].param.sample_rate,client_queue_frame.wave_data[i][j]);

                            //环境光补偿
                            //if        红光数据
                            //else if   红外数据
                            //else      环境光数据
//                            if(dev_list[client_queue_frame.clientSn].param.enable_channels[i] < 18){
//                                dev_list[client_queue_frame.clientSn].wave_ploter->append(i,1.0*(client_queue_frame.index+j)/dev_list[client_queue_frame.clientSn].param.sample_rate,client_queue_frame.wave_data[i][j]-client_queue_frame.wave_data[i-1][j]);
//                            } else if(dev_list[client_queue_frame.clientSn].param.enable_channels[i] < 36){
//                                dev_list[client_queue_frame.clientSn].wave_ploter->append(i,1.0*(client_queue_frame.index+j)/dev_list[client_queue_frame.clientSn].param.sample_rate,client_queue_frame.wave_data[i][j]-client_queue_frame.wave_data[i-2][j]);
//                            } else{
//                                dev_list[client_queue_frame.clientSn].wave_ploter->append(i,1.0*(client_queue_frame.index+j)/dev_list[client_queue_frame.clientSn].param.sample_rate,client_queue_frame.wave_data[i][j]);
//                            }
                        }

//                    std::cout << client_queue_frame.clientSn << " data:" << std::endl;
//                    for(int i = 0; i < dev_list[client_queue_frame.clientSn].wave_data.size();i++){
//                        std::cout << "channel: " << i << " ";
//                        for(int j = 0; j < dev_list[client_queue_frame.clientSn].wave_data[i].size();j++)
//                            std::cout << dev_list[client_queue_frame.clientSn].wave_data[i][j] << " ";
//                        std::cout << std::endl;
//                    }
                } else if(client_queue_frame.type == "mpu"){
                    std::cout << "device: " << client_queue_frame.clientSn << " mpu got." << std::endl;
                    for(int i = 0; i < client_queue_frame.mpu_data.size(); i++)
                        for(int j = 0; j < client_queue_frame.mpu_data[i].size();j++)
                            dev_list[client_queue_frame.clientSn].mpu_data[i].push_back(client_queue_frame.mpu_data[i][j]);

//                    std::cout << client_queue_frame.clientSn << " data:" << std::endl;
//                    for(int i = 0; i < dev_list[client_queue_frame.clientSn].mpu_data.size();i++){
//                        std::cout << "channel: " << i << " ";
//                        for(int j = 0; j < dev_list[client_queue_frame.clientSn].mpu_data[i].size();j++)
//                            std::cout << dev_list[client_queue_frame.clientSn].mpu_data[i][j] << " ";
//                        std::cout << std::endl;
//                    }
                } else if(client_queue_frame.type == "heartbeat"){
                    std::cout << "device: " << client_queue_frame.clientSn << " heartbeat got." << std::endl;
                    dev_list[client_queue_frame.clientSn].battery = client_queue_frame.battery;

                }
            } else{
                break;
            }
        }

    }
}

/*
 *
 * 点选和连选默认选择第一行作为参数
*/
void MainWindow::on_subWindowOpen(){
    QList<QTableWidgetItem *> devTableSelectedItems = m_dev_table->selectedItems();
    int currentRow = m_dev_table->row(devTableSelectedItems[0]);

    //if(dev_list[m_dev_table->item(currentRow, 0)->text().toStdString()].status == LOG) return;

    cfg_param->setParam(&dev_list[m_dev_table->item(currentRow, 0)->text().toStdString()].param);
    //主窗口不可以点选
    this->setDisabled(true);
}

/*
 * 设置mainwindow widget有功能
*/
void MainWindow::on_subWindowClose(){
    this->setEnabled(true);
}

/*
 * 设置配置参数
 * 根据已经选中的设备列表中的设备
*/
void MainWindow::on_cfgParamConfirmedCb(){
    client_config_t cfgParam = cfg_param->getParam();
    QList<QTableWidgetItem *> devTableSelectedItems = m_dev_table->selectedItems();
    for(int dev_index = 0;dev_index < devTableSelectedItems.size()/DevTable::DEV_TABLE_COLUMN_NUMBER;++dev_index){
        int currentRow = m_dev_table->row(devTableSelectedItems[dev_index*DevTable::DEV_TABLE_COLUMN_NUMBER]);
        if(dev_list[m_dev_table->item(currentRow, 0)->text().toStdString()].status == READY)
            dev_list[m_dev_table->item(currentRow, 0)->text().toStdString()].param = cfgParam;
        else{ //TODO ui->dialog显示log
            std::cout << m_dev_table->item(currentRow, 0)->text().toStdString() << " is busy/log now!" << std::endl;
            ui->dialog->append("device: " + m_dev_table->item(currentRow, 0)->text() + " is busy/log now!");
        }
    }
}

/*
 * 波形图回调函数
*/
void MainWindow::on_plotRequested(){
    QList<QTableWidgetItem *> devTableSelectedItems = m_dev_table->selectedItems();
    for(int dev_index = 0;dev_index < devTableSelectedItems.size()/DevTable::DEV_TABLE_COLUMN_NUMBER;++dev_index){
        int currentRow = m_dev_table->row(devTableSelectedItems[dev_index*DevTable::DEV_TABLE_COLUMN_NUMBER]);
        dev_list[m_dev_table->item(currentRow, 0)->text().toStdString()].wave_ploter->setWindowVisible(true);
    }
}

/*
 * vft 任务窗口回调函数
*/
void MainWindow::on_openTaskWindowRequested()
{
    QList<QTableWidgetItem *> devTableSelectedItems = m_dev_table->selectedItems();
    for(int dev_index = 0;dev_index < devTableSelectedItems.size()/DevTable::DEV_TABLE_COLUMN_NUMBER;++dev_index){
        int currentRow = m_dev_table->row(devTableSelectedItems[dev_index*DevTable::DEV_TABLE_COLUMN_NUMBER]);

        if(dev_list[m_dev_table->item(currentRow, 0)->text().toStdString()].status == LOG) continue;

        dev_list[m_dev_table->item(currentRow, 0)->text().toStdString()].vftFlowWindow->setWindowVisible(true);
    }
}

/*
 * 配置参数请求回调函数
 * 根据已选中的设备进行参数配置，若第一个为LOG类型则不显示
*/
void MainWindow::on_cfgParamRequested(){
    QList<QTableWidgetItem *> devTableSelectedItems = m_dev_table->selectedItems();
    int currentRow = m_dev_table->row(devTableSelectedItems[0]);
    if(dev_list[m_dev_table->item(currentRow, 0)->text().toStdString()].status == LOG) return;

    cfg_param->show();
}

/*
 * 启动设备采样回调函数
*/
void MainWindow::on_startSampleRequested(){
    QList<QTableWidgetItem *> devTableSelectedItems = m_dev_table->selectedItems();
    for(int dev_index = 0;dev_index < devTableSelectedItems.size()/DevTable::DEV_TABLE_COLUMN_NUMBER;++dev_index){
        int currentRow = m_dev_table->row(devTableSelectedItems[dev_index*DevTable::DEV_TABLE_COLUMN_NUMBER]);
        std::string Sn = m_dev_table->item(currentRow, 0)->text().toStdString();

        if(dev_list[Sn].status == LOG) continue;

        cmdqueue.push({Sn, "start"});
    }

}

/*
 * 停止设备采样回调函数
*/
void MainWindow::on_stopSampleRequested(){
    QList<QTableWidgetItem *> devTableSelectedItems = m_dev_table->selectedItems();
    for(int dev_index = 0;dev_index < devTableSelectedItems.size()/DevTable::DEV_TABLE_COLUMN_NUMBER;++dev_index){
        int currentRow = m_dev_table->row(devTableSelectedItems[dev_index*DevTable::DEV_TABLE_COLUMN_NUMBER]);
        std::string Sn = m_dev_table->item(currentRow, 0)->text().toStdString();

        if(dev_list[Sn].status == LOG) continue;

        cmdqueue.push({Sn, "stop"});
    }
}

/*
 * 直接采样中保存文件回调函数
*/
void MainWindow::on_saveFileRequested(){
    //std::cout << "SaveFile Triggered" << std::endl;
    // 获取当前系统时间的时间戳
    std::time_t b_currentTime = std::time(nullptr);
    std::tm *currentTime = std::localtime(&b_currentTime);

    //获取选中的Sn
    QList<QTableWidgetItem *> devTableSelectedItems = m_dev_table->selectedItems();
    for(int dev_index = 0;dev_index < devTableSelectedItems.size()/DevTable::DEV_TABLE_COLUMN_NUMBER;++dev_index){
        int currentRow = m_dev_table->row(devTableSelectedItems[dev_index*DevTable::DEV_TABLE_COLUMN_NUMBER]);
        dev_list_t dev_to_save = dev_list[m_dev_table->item(currentRow, 0)->text().toStdString()];

        if(dev_to_save.status == LOG) continue;

        if(dev_to_save.wave_ploter->graphCount() > 0){
            //获取保存文件名的目录
            QString currentPath = QDir::currentPath();
            QString logFolderPath = currentPath + "\\log\\DirectSample";

            // 创建目录，如果目录不存在
            if (!QDir().mkpath(logFolderPath)) {
                // 创建目录失败，处理错误
                std::cerr << "Mkpath failed：" << logFolderPath.toStdString() << std::endl;
                ui->dialog->append("make path: " + logFolderPath + " failed!");
                return;
            }

            //获取保存文件名前缀
            QString file_name_prefix =  QString::fromStdString(m_dev_table->item(currentRow, 0)->text().toStdString()) +
                                        "+" +
                                        QString::number(currentTime->tm_year+1900) +
                                        QString::number(currentTime->tm_mon+1) +
                                        QString::number(currentTime->tm_mday) +
                                        QString::number(currentTime->tm_hour) +
                                        QString::number(currentTime->tm_min) +
                                        QString::number(currentTime->tm_sec);

            QString png_file_name = logFolderPath + "\\" + file_name_prefix + ".png";
            dev_to_save.wave_ploter->savePng(png_file_name,1920,1080);

            QString markFileName =  logFolderPath + "\\" + file_name_prefix + "_mark.csv";
            QString rawFileName =  logFolderPath + "\\" + file_name_prefix + "_rawdata.csv";
            write2Mark(file_name_prefix, markFileName, dev_to_save);
            write2RawData(file_name_prefix, rawFileName, dev_to_save);
        }
    }
}

/*
 * 写Mark文件
*/
void MainWindow::write2Mark(QString fileNamePrefix, QString markFileName, dev_list_t dev2Save){
    QFile csvFile(markFileName);
    if (csvFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream filestream(&csvFile);
        //写头部
        filestream << "Header,\n";
        filestream << "Participant Infomation,\n";
        filestream << "ID,\n";
        if(dev2Save.taskType == VFT_TASK)
            filestream << "Name," << QString::fromStdString(dev2Save.testName) << ",\n";
        else if(dev2Save.taskType == DIRECT_SAMPLE)
            filestream << "Name,\n";
        filestream << "Comment,\n";
        filestream << "Birth Date,\n";
        filestream << "Age,\n";
        filestream << "Sex,\n";
        filestream << "Measure Infomation,\n";
        filestream << "Date,\n";
        filestream << "Probe Type,\n";
        filestream << "sample_rate," << QString::number(dev2Save.param.sample_rate) << ",\n";
        filestream << "sample_delay," << QString::number(dev2Save.param.sample_delay) << ",\n";
        filestream << "sample_time," << QString::number(dev2Save.param.sample_duration) << ",\n";
        filestream << "luminous_intensity,\n";
        filestream << fileNamePrefix + ",\n";
        filestream << "Data,\n";
        filestream << "Mark,Time,\n";

        //写marker
        for(int i = 0, total_count = dev2Save.wave_ploter->getMarker().size(); i < total_count; ++i){
            filestream << i << "," << dev2Save.wave_ploter->getMarker().at(i)->point1->key() << ",";
            if(dev2Save.taskType == VFT_TASK){
                switch(i){
                    case 1:
                        filestream << dev2Save.testWords.at(0) << ",\n";
                        break;
                    case 2:
                        filestream << dev2Save.testWords.at(1) << ",\n";
                        break;
                    case 3:
                        filestream << dev2Save.testWords.at(2) << ",\n";
                        break;
                    default:
                        filestream << "\n";
                        break;
                }
            }
            else if(dev2Save.taskType == DIRECT_SAMPLE) filestream << "\n";
        }

        std::cout << "write to mark file: "<< markFileName.toStdString() << " done." << std::endl;
        ui->dialog->append("write to mark file: " + markFileName + " done.");

        csvFile.close();
    } else {
        std::cerr << "write to mark file: "<< markFileName.toStdString() << " failed!" << std::endl;
        ui->dialog->append("write to mark file: " + markFileName + " failed!");
    }
}

/*
 * 写RawData文件
*/
void MainWindow::write2RawData(QString fileNamePrefix, QString rawFileName, dev_list_t dev2Save){
    QFile csvFile(rawFileName);
    if (csvFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream filestream(&csvFile);
        //写头部
        filestream << "Header,\n";
        filestream << "Participant Infomation,\n";
        filestream << "ID,\n";
        if(dev2Save.taskType == VFT_TASK)
            filestream << "Name," << QString::fromStdString(dev2Save.testName) << ",\n";
        else if(dev2Save.taskType == DIRECT_SAMPLE)
            filestream << "Name,\n";
        filestream << "Comment,\n";
        filestream << "Birth Date,\n";
        filestream << "Age,\n";
        filestream << "Sex,\n";
        filestream << "Measure Infomation,\n";
        filestream << "Date,\n";
        filestream << "Probe Type,\n";
        filestream << "sample_rate," << QString::number(dev2Save.param.sample_rate) << ",\n";
        filestream << "sample_delay," << QString::number(dev2Save.param.sample_delay) << ",\n";
        filestream << "sample_time," << QString::number(dev2Save.param.sample_duration) << ",\n";
        filestream << "luminous_intensity,\n";
        filestream << fileNamePrefix + ",\n";
        filestream << "Data,\n";
        if(dev2Save.taskType == VFT_TASK)
            filestream << "time(s),H1,G1,H2,G2,A2,B2,A3,B3,C3,D3,C4,D4,E4,F4,E5,F5,E6,F6,E7,F7,E8,F8,C8,D8,C9,D9,A9,B9,A10,B10,H10,G10,H11,G11,H12,G12,\n";
        else if(dev2Save.taskType == DIRECT_SAMPLE){
            filestream << "time(s),";
            for(int i = 0, total_count = dev2Save.wave_ploter->graphCount(); i < total_count; ++i){
                    filestream << dev2Save.wave_ploter->graph(i)->name() << ",";
                    if(i == total_count - 1) filestream << "\n";
            }
        }

//        std::vector<int> N50002N4000Map={7,25,6,24,23,5,22,4,21,3,20,2,19,1,18,0,35,17,34,16,33,15,32,14,31,13,30,12,29,11,10,28,9,27,8,26};

        //写raw data
        for(int i = 0, total_data_count = dev2Save.wave_ploter->graph(0)->dataCount(), total_graph_count = dev2Save.wave_ploter->graphCount(); i < total_data_count; ++i){
            filestream << dev2Save.wave_ploter->graph(0)->data().data()->at(i)->key << ",";  //time(s)
            for(int j = 0; j < total_graph_count; ++j)
                    filestream << dev2Save.wave_ploter->graph(j)->data().data()->at(i)->value << ",";
            filestream << "\n";
        }

        std::cout << "write to rawdata file: "<< rawFileName.toStdString() << " done." << std::endl;
        ui->dialog->append("write to rawdata file: " + rawFileName + " done.");

        csvFile.close();
    } else {
        std::cerr << "write2RawData failed!" << std::endl;
        ui->dialog->append("write to rawdata file: " + rawFileName + " failed!");
    }
}

/*
 * 打开文件以显示
*/
void MainWindow::on_openFileRequested(){
    //std::cout << "OpenFile Triggered" << std::endl;
    // 打开文件对话框以获取文件路径
    QStringList filePathList = QFileDialog::getOpenFileNames(nullptr, "选择文件", "", "文本文件 (*.csv);;所有文件 (*.*)");

    for(const QString &filePath : filePathList){

        if (!filePath.isEmpty()) {
            // 使用获取的文件路径打开文件
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);

                // 读取文件内容并输出到控制台
                int line_count = 0;
                QString lineContent;
                QStringList lineStringList;
                std::string devName;
                std::string dataType;
                while (!in.atEnd()) {
                    lineContent = in.readLine();
                    lineStringList = lineContent.split(',');
                    ++line_count;
                    if(line_count == 16){
                        devName = lineStringList[0].toStdString();
                            //新的log设备
                        if(dev_list.find(devName) == dev_list.end()){
                        dev_list[devName] = {
                                             .Sn=devName,
                                             .wave_ploter = new WavePlot(nullptr, QString::fromStdString(devName), runType),
                                             .status = LOG,
                                             .taskType = runType};
                        }
                    }
                    if(line_count == 18) dataType = (lineStringList[0] == "Mark" ? "mark" : "rawData");
                    //设置曲线名字
                    if(line_count == 18 && dataType == "rawData"){
                        dev_list[devName].wave_ploter->resizeChannelLength(lineStringList.size()-1);
                        for(int i = 1; i < lineStringList.size();++i) dev_list[devName].wave_ploter->setChannelName(i-1, lineStringList[i]);
                    } else if(line_count == 18 && dataType == "mark") dev_list[devName].wave_ploter->clearMarker();

                    if(line_count > 18){
                        if(dataType == "mark") dev_list[devName].wave_ploter->addMarker(lineStringList[1].toDouble());
                        if(dataType == "rawData"){
                            for(int i = 1; i < lineStringList.size();++i)
                                dev_list[devName].wave_ploter->append(i-1,lineStringList[0].toDouble(),lineStringList[i].toDouble());
                        }
                    }

                }

                ui->dialog->append("file: " + filePath + " loaded.");

                file.close();
            } else {
                std::cerr << "can not open file：" << filePath.toStdString() << std::endl;
                ui->dialog->append("can not load file: " + filePath + "!");
            }
        } else {
            //qDebug() << "用户取消了文件选择操作";
        }
    }
}

/*
 * 添加marker回调
*/
void MainWindow::on_addMarkerRequested(){
    QList<QTableWidgetItem *> devTableSelectedItems = m_dev_table->selectedItems();
    for(int dev_index = 0;dev_index < devTableSelectedItems.size()/DevTable::DEV_TABLE_COLUMN_NUMBER;++dev_index){
        int currentRow = m_dev_table->row(devTableSelectedItems[dev_index*DevTable::DEV_TABLE_COLUMN_NUMBER]);
        std::string Sn = m_dev_table->item(currentRow, 0)->text().toStdString();

        if(dev_list[Sn].status == LOG) continue;

        dev_list[Sn].wave_ploter->addMarker();
    }
}

/*
 * 关闭设备（LOG）
*/
void MainWindow::on_closeDeviceRequested(){
    QList<QTableWidgetItem *> devTableSelectedItems = m_dev_table->selectedItems();
    for(int dev_index = 0;dev_index < devTableSelectedItems.size()/DevTable::DEV_TABLE_COLUMN_NUMBER;++dev_index){
        int currentRow = m_dev_table->row(devTableSelectedItems[dev_index*DevTable::DEV_TABLE_COLUMN_NUMBER]);
        std::string Sn = m_dev_table->item(currentRow, 0)->text().toStdString();

        if(dev_list[Sn].status == LOG){
            dev_list[Sn].wave_ploter->setWindowVisible(false);
            dev_list.erase(Sn);
        } else ui->dialog->append("Only LOG type device can be closed!");

    }
}

/*
 * vft任务开始信号
 * 配置参数，开始采样，传递words参数
*/
void MainWindow::on_vftTaskStartRequested(vftFlow::sampleSig_t param){
    client_config_t cfgParam ={ .sample_rate = 20,
                                .sample_delay = 1000,
                                .nburst = 200,
                                .sample_duration = 600,
                                .enable_channels = {7,25,6,24,23,5,22,4,21,3,20,2,19,1,18,0,35,17,34,16,33,15,32,14,31,13,30,12,29,11,10,28,9,27,8,26},
                                .led_strength = {1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023},
                                .sample_adc_gain = {4,4,4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4,4,4,4,4,4,4},
                                .mpu_ares_gain = 0,
                                .mpu_gres_gain = 0};
    dev_list[param.Sn].param = cfgParam;
    dev_list[param.Sn].testWords = param.words;
    dev_list[param.Sn].testName = param.Name;
    //test
    //dev_list[param.Sn].wave_ploter->resizeChannelLength(dev_list[param.Sn].param.enable_channels);
    cmdqueue.push({param.Sn, "start"});
}

/*
 * vft任务结束信号
 * 结束采样，保存文件
*/
void MainWindow::on_vftTaskStopRequested(vftFlow::sampleSig_t param){

    cmdqueue.push({param.Sn, "stop"});
    //Save
    // 获取当前系统时间的时间戳
    std::time_t b_currentTime = std::time(nullptr);
    std::tm *currentTime = std::localtime(&b_currentTime);

    dev_list_t dev_to_save = dev_list[param.Sn];
    if(dev_to_save.wave_ploter->graphCount() > 0){
        //获取保存文件名的目录
        QString currentPath = QDir::currentPath();
        QString logFolderPath = currentPath + "\\log\\VFT";

        // 创建目录，如果目录不存在
        if (!QDir().mkpath(logFolderPath)) {
            // 创建目录失败，处理错误
            std::cerr << "Mkpath failed：" << logFolderPath.toStdString() << std::endl;
            ui->dialog->append("make path: " + logFolderPath + " failed!");
            return;
        }

        //获取保存文件名前缀
        QString file_name_prefix = QString::fromStdString(dev_to_save.testName) +
                                   "+" +
                                   QString::number(currentTime->tm_year+1900) +
                                   QString::number(currentTime->tm_mon+1) +
                                   QString::number(currentTime->tm_mday) +
                                   QString::number(currentTime->tm_hour) +
                                   QString::number(currentTime->tm_min) +
                                   QString::number(currentTime->tm_sec);

        QString png_file_name = logFolderPath + "\\" + file_name_prefix + ".png";
        dev_to_save.wave_ploter->rescaleAxes(true);
        dev_to_save.wave_ploter->replot();
        dev_to_save.wave_ploter->savePng(png_file_name,1920,1080);

        QString markFileName = logFolderPath + "\\" + file_name_prefix + "_mark.csv";
        QString rawFileName = logFolderPath + "\\" + file_name_prefix + "_rawdata.csv";
        write2Mark(file_name_prefix, markFileName, dev_to_save);
        write2RawData(file_name_prefix, rawFileName, dev_to_save);
    }
}

/*
 * vft设置marker回调
*/
void MainWindow::on_vftTaskSetMarkerRequested(vftFlow::sampleSig_t param){
    dev_list[param.Sn].wave_ploter->addMarker();
}

/*
 * 加载词库
*/
void MainWindow::loadQuestion(){
    // 打开文件对话框以获取文件路径

    QString currentPath = QDir::currentPath();
    QString filePath = currentPath + "\\config\\structure.csv";
    //QString filePath = QFileDialog::getOpenFileName(nullptr, "选择文件", "", "文本文件 (*.csv);;所有文件 (*.*)");

    if (!filePath.isEmpty()) {
        // 使用获取的文件路径打开文件
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);

            // 读取问题
            QString lineContent;
            if(!in.atEnd()) {
                lineContent = in.readLine();
                questionList = lineContent.split(',');
            }

            file.close();
        } else {
            std::cerr << "can not open file：" << filePath.toStdString() << std::endl;
            ui->dialog->append("can not load file: " + filePath + "!");
        }
    } else {
        //qDebug() << "用户取消了文件选择操作";
    }
}
