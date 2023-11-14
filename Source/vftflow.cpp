#include "vftflow.h"
#include "ui_vftflow.h"
#include <QTimer>
#include <iostream>

vftFlow::vftFlow(QWidget *parent, std::string Sn, QStringList questList) :
    QWidget(parent),
    ui(new Ui::vftFlow)
{
    deviceSn = Sn;
    questionList = questList;   //所有的问题list
    ui->setupUi(this);
    this->resize(800,600);
    this->setWindowTitle(QString::fromStdString(Sn));
    //初始化定时器
    p_timer = new QTimer(this);
    p_timer->setInterval(1000);
    p_timer->setSingleShot(false);
    connect(p_timer, &QTimer::timeout, this, &vftFlow::secTimerCb);

    //初始化页面
    setPage(0);
    ui->lineEdit->clear();
}

vftFlow::~vftFlow()
{
    delete ui;
}

void vftFlow::setPage(int pageIndex){
    ui->stackedWidget->setCurrentIndex(pageIndex);
}

/*
 * 输入名字后的确认按钮回调函数
*/
void vftFlow::on_pushButton_clicked()
{
    //发送信号 param：Sn， 词语
    if(ui->lineEdit->text().trimmed().isEmpty()){
        ui->lineEdit->setToolTip("input invalid!");
        ui->lineEdit->setToolTipDuration(3000);
    }else{
        ui->lineEdit->setToolTip("");
        QStringList words;
        words.push_back(questionList.at(int(rand()%questionList.size())));
        words.push_back(questionList.at(int(rand()%questionList.size())));
        words.push_back(questionList.at(int(rand()%questionList.size())));
        testWords = words;
        emit startSampleSig({.Sn = deviceSn, .Name=ui->lineEdit->text().trimmed().toStdString(), .words = words});
        //调用状态机
        setState(1);
    }
}

/*
 * 1s 定时器回调函数
 * 用于根据目前状态更新界面
 * 当time <= 0,调用setState更新当前状态
*/
void vftFlow::secTimerCb(){
    this->time -= 1;

    switch(this->state){
        case 1: //更新倒计时界面
            updateCountDown();

            break;
        default:
            break;
    }

    // 根据time更新state
    if(this->time <= 0){
        setState(this->state+1);
    }
}

/*
 *设置当前的状态
 *用于初始化各个状态
*/
void vftFlow::setState(uint32_t switch2state){
    this->state = switch2state;

    //state == 1 or 8 do not emit signal to set marker
    if((1 != this->state) && (8!= this->state))
        emit setSampleMarker({.Sn=deviceSn});

    switch(this->state){
        case 1:
            this->ui->labelCountDown->setText("The task starts in 5 secs");
            setPage(1);
            this->time = 5;

            p_timer->start();
            break;
        case 2:
            setPage(2);
            this->time = 30;

            break;
        case 3:
            setPage(3);
            this->time = 20;
            //更新词语
            ui->label_8->setText(testWords[0]);
            break;
        case 4:
            this->time = 20;
            //更新词语
            ui->label_8->setText(testWords[1]);
            break;
        case 5:
            this->time = 20;
            //更新词语
            ui->label_8->setText(testWords[2]);
            break;
        case 6:
            setPage(2);
            this->time = 30;

            break;
        case 7:
            setPage(4);
            this->time = 7;
            emit stopSampleSig({.Sn = deviceSn});

            break;
        case 8:
            setPage(0);
            ui->lineEdit->clear();
            p_timer->stop();
            this->state = 0;    //trans to idle

            break;
        default:
            break;
    }
}

/*
 * 更新开始倒计时
*/
void vftFlow::updateCountDown(){
    this->ui->labelCountDown->setText("The task starts in " + QString::number(this->time) + " secs");
}

