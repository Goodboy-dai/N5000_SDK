#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <iostream>
#include <thread>
#include <winsock2.h>
#include "json.hpp"
#include "circular_queue.h"
#include <map>

#define FRAME_HEAD_LENGTH 47
#define FRAME_HEAD_TAIL_LENGTH 52
#define FRAME_PREAMBLE 0xAB5A5ABA
#define FRAME_END_SYM 0x0A
#define PACKET_SEND_BUFSIZE 1024
#define PACKET_RECV_BUFSIZE 4096
#define LED_NUMBER 12
#define ADC_GAIN_NUMBER 24
#define QUEUE_SIZE 50

//0~17  各通道的Hb数据
//18~35 各通道的HbO数据
//36~53 各通道的环境光数据
const uint32_t CHANNEL_MAP[54] = {155,156,108,109, 61, 48,  0,  1,  2,  3,  4, 52, 53,101,104,152,153,154,
                                  179,180,132,133, 85, 72, 24, 25, 26, 27, 28, 76, 77,125,128,176,177,178,
                                  299,300,300,301,301,288,288,289,290,291,292,292,293,293,296,296,297,298};

/*
 * server_queue_t
 * 协议层向用户层发送的数据类型
 * 在客户端连接状态切换和数据上传时使用
 * 上传Sn为了标记map的键
 * 上传IP和battery用于显示设备状态
 * type:
 *      connected:      客户端连接，上传 Sn IP
 *      disconected:    客户端断开，上传 Sn IP
 *      wave:           有波形数据可用，上传 Sn，wave_data
 *      done:           客户端数据采样完成，上传 Sn
 *      dummy:          队列没有数据
 *      heartbeat:      客户端心跳包，用于显示状态如电池电压
*/
struct server_queue_t{
    std::string clientSn;
    std::string type;
    std::string clientIP;
    uint32_t battery;
    uint32_t index;
    std::vector<std::vector<int32_t>> wave_data;
    std::vector<std::vector<float>> mpu_data;
};

/*
 * packet_head_t
 * 客户端和上位机通信协议包头
*/
struct packet_head_t{
    uint32_t  preamble;
    uint8_t ver;
    uint16_t seq;
    uint32_t flags;
    uint8_t res[16];
    uint32_t len;
    uint32_t offset;
    uint32_t total_len;
    uint32_t jlen;
    uint32_t blen;
};

/*
 * client_config_t
 * 用户层向客户端发送start和stop指令时要带的配置参数
*/
struct client_config_t{
    uint32_t sample_rate;
    uint32_t sample_delay;
    uint32_t nburst;
    uint32_t sample_duration;
    std::vector<uint32_t> enable_channels;
    std::array<uint32_t, LED_NUMBER> led_strength;
    std::array<uint32_t, ADC_GAIN_NUMBER> sample_adc_gain;
    uint32_t mpu_ares_gain;
    uint32_t mpu_gres_gain;
};

/*
 * client_cmd_t
 * 用户层向客户端发送start和stop指令 client_queue
*/
struct client_cmd_t{
    std::string cmd;
    client_config_t config;
};

class packet_funs
{
private:
    packet_head_t send_packet_head = {FRAME_PREAMBLE, 0x01, 0, 0, {0}, 0, 0, 0, 0, 0};
    packet_head_t recv_packet_head;
    const uint32_t dummy_before_end = 0;
    const uint8_t frame_end_sym = 0x0A;
    char packet_send_buf[PACKET_SEND_BUFSIZE];
    char packet_recv_buf[PACKET_RECV_BUFSIZE];
    nlohmann::json packet_recv_jdata;
    std::vector<std::vector<int32_t>> wave_data;
    std::vector<std::vector<float>> mpu_data;
public:
    packet_funs();
    void send_cmd(SOCKET socket, std::string jstr, void *bin_buf, uint32_t bin_size);
    int recv_frame(SOCKET socket);
    nlohmann::json get_recv_jdata(){return packet_recv_jdata;}
    packet_head_t get_recv_packet_head(){return recv_packet_head;}
    std::vector<std::vector<int32_t>> get_wave_data(){return wave_data;}
    std::vector<std::vector<float>> get_mpu_data(){return mpu_data;}
};

class brain_client
{
private:
    bool running = true;
    std::string clientAddr;
    std::string clientSn;
    SOCKET clientSocket;
    CircularQueue<client_cmd_t> client_queue{QUEUE_SIZE};   //用户层向协议层发送指令，start，stop
    CircularQueue<server_queue_t> *p_server_queue;   //协议层向用户层返回指令/数据
    packet_funs* pfun;                     //socket 包有关，由类自行管理
    uint32_t battery = 0;
    void client_thread(std::map<std::string, brain_client*> *nirs_dev);
public:
    brain_client(std::string addr, std::string sn, SOCKET socket, CircularQueue<server_queue_t> *server_queue){
        pfun = new packet_funs(), clientAddr = addr, clientSn = sn, clientSocket = socket, p_server_queue = server_queue;}
    void client_set_running(bool run){running = run;}
    void client_set_clientAddr(std::string addr){clientAddr = addr;}
    void client_set_clientSn(std::string sn){clientSn = sn;}
    void client_set_clientSocket(SOCKET socket){clientSocket = socket;}
    void client_client_queue_push(client_cmd_t cmd){client_queue.push(cmd);}
    uint32_t client_get_battery(){return battery;}
    SOCKET client_get_clientSocket(){return clientSocket;}
    void client_startThread(std::map<std::string, brain_client*> *nirs_dev){running = true; std::thread th(&brain_client::client_thread, this, nirs_dev); th.detach();}

    ~brain_client(){
        running = false;    //线程回收
        //Sleep(1000);
        //delete pfun;
    };
};

class brain_server
{
private:
    int serverSocket;
    int port = 8765;
    bool running;
    packet_funs *pfun;   //server packet通信对象
    CircularQueue<server_queue_t> server_queue{QUEUE_SIZE};   //协议层向用户层返回指令/数据
    std::map<std::string, brain_client*> nirs_dev;
    void server_thread();
public:
    brain_server();
    void start();
    void stop();
    void put_cmd(std::string sn, std::string cmd);
    void put_cmd(std::string sn, std::string cmd, client_config_t config);
    server_queue_t get_msg();
};

#endif // PROTOCOL_H
