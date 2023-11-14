#include "configparam.h"
#include "ui_configparam.h"
#include <QShowEvent>
#include <QCloseEvent>
#include <iostream>

/*
 * 设置配置界面ui
 * 将通道按钮按照顺序放在vector中
*/
ConfigParam::ConfigParam(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigParam)
{
    ui->setupUi(this);
    checkBoxVector.push_back(ui->box_ch1);
    checkBoxVector.push_back(ui->box_ch2);
    checkBoxVector.push_back(ui->box_ch3);
    checkBoxVector.push_back(ui->box_ch4);
    checkBoxVector.push_back(ui->box_ch5);
    checkBoxVector.push_back(ui->box_ch6);
    checkBoxVector.push_back(ui->box_ch7);
    checkBoxVector.push_back(ui->box_ch8);
    checkBoxVector.push_back(ui->box_ch9);
    checkBoxVector.push_back(ui->box_ch10);
    checkBoxVector.push_back(ui->box_ch11);
    checkBoxVector.push_back(ui->box_ch12);
    checkBoxVector.push_back(ui->box_ch13);
    checkBoxVector.push_back(ui->box_ch14);
    checkBoxVector.push_back(ui->box_ch15);
    checkBoxVector.push_back(ui->box_ch16);
    checkBoxVector.push_back(ui->box_ch17);
    checkBoxVector.push_back(ui->box_ch18);
}

ConfigParam::~ConfigParam()
{
    delete ui;
}


/*
 * 配置页面关闭
*/
void ConfigParam::on_pushButton_cancel_clicked()
{
    this->close();
}

/*
 *  配置确认生效并关闭配置页面
*/
void ConfigParam::on_pushButton_confirm_clicked()
{
    emit cfgParamConfirmed();
    this->close();
}

/*
 * showEvent重写
 * 用于发送配置页面打开信号到mainwindow，便于在一开始设置配置页面的初始参数并禁用mainwindow的widget
*/
void ConfigParam::showEvent(QShowEvent *event){
    emit subWindowOpened();
    //std::cout << "show" << std::endl;
    QWidget::showEvent(event);
    //event->accept();
}

/*
 * 配置页面关闭信号，用于mainwindow恢复widget功能
*/
void ConfigParam::closeEvent(QCloseEvent *event){
    emit subWindowClosed();
    //std::cout << "close" << std::endl;
    event->accept();
}

/*
 * 设置配置页面初始显示
 * 注意：检查通道时只检查了0-17，默认其他通道对应开启
*/
void ConfigParam::setParam(client_config_t *client_config){
    if(client_config != nullptr){
        ui->lineEdit_sampleRate->setText(QString::number(client_config->sample_rate));
        ui->lineEdit_sampleDelay->setText(QString::number(client_config->sample_delay));
        ui->lineEdit_sampleDuration->setText(QString::number(client_config->sample_duration));
        ui->comboBox_adcGain->setCurrentIndex(client_config->sample_adc_gain[0]);
        ui->comboBox_mpuAres->setCurrentIndex(client_config->mpu_ares_gain);
        ui->comboBox_mpuGres->setCurrentIndex(client_config->mpu_gres_gain);

        //reset checkbox state
        for(QCheckBox* checkBoxPtr : checkBoxVector) checkBoxPtr->setChecked(false);
        //set checkbox state
        for(uint32_t channel : client_config->enable_channels)
            if(channel < 18) checkBoxVector[channel]->setChecked(true);

    }
}

/*
 * 获取配置页面的配置
 * 默认vector：adc gain，nburst，DAC strength以及对应的通道0-18-36，1-19-37 etc.在此配置
 * TODO:
 *      1. 确定sample_delay有效范围
*/
client_config_t ConfigParam::getParam(){
    client_config_t cfg_t;
    QString text;
    int sample_;
    bool ret;

    //sample_rate
    text = ui->lineEdit_sampleRate->text().trimmed();
    if(text.isEmpty()) sample_ = 20;  //默认为20Hz
    else {
        sample_ = text.toInt(&ret);
        if(ret){
            if(sample_ < 1) sample_ = 1;
            else if(sample_ > 50) sample_ = 50;
        } else sample_ = 20;
    }
    cfg_t.sample_rate = sample_;

    //sample_delay
    text = ui->lineEdit_sampleDelay->text().trimmed();
    if(text.isEmpty()) sample_ = 1000;  //默认为1000us
    else {
        sample_ = text.toInt(&ret);
        if(ret){
            if(sample_ < 1) sample_ = 1;
            else if(sample_ > 2000) sample_ = 2000;
        } else sample_ = 1000;
    }
    cfg_t.sample_delay = sample_;

    //nburst
    cfg_t.nburst = 20;

    //sample_duration
    text = ui->lineEdit_sampleDuration->text().trimmed();
    if(text.isEmpty()) sample_ = 60;  //默认为60s
    else {
        sample_ = text.toInt(&ret);
        if(ret){
            if(sample_ < 1) sample_ = 1;
            else if(sample_ > 10800) sample_ = 10800;
        } else sample_ = 60;
    }
    cfg_t.sample_duration = sample_;

    //enable_channels
    //顺序不可以颠倒，环境光补偿有用
    for(uint32_t channel = 0; channel < 18;++channel){
        if(checkBoxVector[channel]->isChecked()){
            cfg_t.enable_channels.push_back(channel+36);
            cfg_t.enable_channels.push_back(channel);
            cfg_t.enable_channels.push_back(channel+18);

        }
    }

    //led_strength
    std::array<uint32_t, LED_NUMBER> led_strength;
    for(int i = 0; i < LED_NUMBER;++i) led_strength[i] = 1023;
    cfg_t.led_strength = led_strength;

    //sample_adc_gain
    uint32_t adc_gain = ui->comboBox_adcGain->currentIndex();
    std::array<uint32_t, ADC_GAIN_NUMBER> sample_adc_gain;
    for(int i = 0;i < ADC_GAIN_NUMBER;++i) sample_adc_gain[i] = adc_gain;
    cfg_t.sample_adc_gain = sample_adc_gain;

    //mpu_ares_gain
    cfg_t.mpu_ares_gain = ui->comboBox_mpuAres->currentIndex();

    //mpu_gres_gain
    cfg_t.mpu_gres_gain = ui->comboBox_mpuGres->currentIndex();

    return cfg_t;
}



















