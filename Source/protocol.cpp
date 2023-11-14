#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "protocol.h"
#include <iostream>
#include <winsock2.h>
#include <chrono>   //延时有关
#include <thread>
#include <ctime> //获取系统时间
#include <pthread.h>
#include <unistd.h> //套接字关闭
#include "circular_queue.h"
#include <map>

#include "json.hpp"


#pragma comment(lib,"ws2_32.lib")


packet_funs::packet_funs(){

}

/*
 * 命令数据包打包发送
*/
void packet_funs::send_cmd(SOCKET socket, std::string jstr, void *bin_buf, uint32_t bin_size){
    //包头数据赋值
    send_packet_head.preamble = FRAME_PREAMBLE;
    send_packet_head.ver = 0x01;
    send_packet_head.seq += 1;
    send_packet_head.flags = 0;
    memset(send_packet_head.res, 0, sizeof(send_packet_head.res));
    send_packet_head.len = FRAME_HEAD_TAIL_LENGTH + jstr.size() + bin_size;
    send_packet_head.offset = 0;
    send_packet_head.total_len = jstr.size() + bin_size;
    send_packet_head.jlen = jstr.size();
    send_packet_head.blen = bin_size;

    // 将包头写入发送缓冲区
    memcpy(packet_send_buf, &send_packet_head.preamble, 4);
    memcpy(packet_send_buf + 4, &send_packet_head.ver, 1);
    memcpy(packet_send_buf + 5, &send_packet_head.seq, 2);
    memcpy(packet_send_buf + 7, &send_packet_head.flags, 4);
    memcpy(packet_send_buf + 11, send_packet_head.res, 16);
    memcpy(packet_send_buf + 27, &send_packet_head.len, 4);
    memcpy(packet_send_buf + 31, &send_packet_head.offset, 4);
    memcpy(packet_send_buf + 35, &send_packet_head.total_len, 4);
    memcpy(packet_send_buf + 39, &send_packet_head.jlen, 4);
    memcpy(packet_send_buf + 43, &send_packet_head.blen, 4);

    //写入json数据
    memcpy(packet_send_buf + FRAME_HEAD_LENGTH, jstr.c_str(), jstr.size());
    //写入bin数据
    memcpy(packet_send_buf + FRAME_HEAD_LENGTH + jstr.size(), bin_buf, bin_size);
    //写入包尾dummy
    memcpy(packet_send_buf + FRAME_HEAD_LENGTH + jstr.size() + bin_size, &dummy_before_end, 4);
    //写入包尾sym
    memcpy(packet_send_buf + FRAME_HEAD_TAIL_LENGTH - 1 + jstr.size() + bin_size, &frame_end_sym, 1);    //写入包尾

    //套接字发送数据包
    send(socket, packet_send_buf, send_packet_head.len, 0);
}

//  超时异常怎么做？
/*
 *数据包接收，如果成功接收，自动将包头保存在私有类packet_head中
 *返回值：
 *      0   ：数据包有效
 *      -1  : 未获取到有效包头
 *      -2  : 包头长度过短，无效包头
 *      -3  : 包尾无效
 *      -4  : recv异常,超时等
*/
int packet_funs::recv_frame(SOCKET socket){
    uint32_t recv_size = 0;
    //先获取包头
    memset(packet_recv_buf, 0, PACKET_RECV_BUFSIZE);
    while(recv_size < 47){
        int ret = recv(socket, packet_recv_buf + recv_size,(FRAME_HEAD_LENGTH - recv_size),0);
        if(ret <= 0){
            std::cerr << "error recv: " << ret << std::endl;
            return -4;
        }
        else recv_size += ret;
    }

    //获取包头的前导标识
    memcpy(&recv_packet_head.preamble, packet_recv_buf, 4);
    if(recv_packet_head.preamble != FRAME_PREAMBLE){
        std::cerr << "error preamble: " << std::hex << recv_packet_head.preamble << std::endl;
        return -1;
    }

    //获取包长度
    memcpy(&recv_packet_head.len, packet_recv_buf + 27, 4);
    if(recv_packet_head.len < 47){
        std::cerr << "invalid head length: " << recv_packet_head.len << std::endl;
        return -2;
    }

    //std::cout << "len:" << recv_packet_head.len <<" total_len:" << recv_packet_head.total_len <<" jlen:" << recv_packet_head.jlen << " blen:" << recv_packet_head.blen << std::endl;

    //接受json、bin、dummy、end_sym数据
    while(recv_size < recv_packet_head.len){
        int ret = recv(socket, packet_recv_buf + recv_size, recv_packet_head.len - recv_size, 0);
        if(ret <= 0){
            std::cerr << "error recv: " << ret << std::endl;
            return -4;
        }
        else recv_size += ret;
    }

    //校验包尾SYM
    if(packet_recv_buf[recv_packet_head.len - 1] != FRAME_END_SYM){
        std::cerr << "error end sym: " << std::hex << packet_recv_buf[recv_packet_head.len - 1] << std::endl;
        return -3;
    }

    //解析包头数据
    memcpy(&recv_packet_head.ver, packet_recv_buf + 4, 1);
    memcpy(&recv_packet_head.seq, packet_recv_buf + 5, 2);
    memcpy(&recv_packet_head.flags, packet_recv_buf + 7, 4);
    memcpy(recv_packet_head.res, packet_recv_buf + 11, 16);
    memcpy(&recv_packet_head.offset, packet_recv_buf + 31, 4);
    memcpy(&recv_packet_head.total_len, packet_recv_buf + 35, 4);
    memcpy(&recv_packet_head.jlen, packet_recv_buf + 39, 4);
    memcpy(&recv_packet_head.blen, packet_recv_buf + 43, 4);


    //获取jdata数据，如果有
    if( recv_packet_head.jlen){
        std::string jdata_string(packet_recv_buf + FRAME_HEAD_LENGTH, recv_packet_head.jlen);
        packet_recv_jdata = nlohmann::json::parse(jdata_string);

        //获取bin数据，如果有
        if(recv_packet_head.blen){
            //波形数据
            if(5 == packet_recv_jdata["cmd"]){
                //初始化vector大小
                wave_data.clear();
                int chn = packet_recv_jdata["data"]["chn"];
                int nitem = packet_recv_jdata["data"]["nitem"];
                int jlen = recv_packet_head.jlen;
                wave_data.resize(chn);
                for(int i = 0; i < chn;i++)
                    wave_data[i].resize(nitem);
                //填充vector
                for(int i = 0; i < chn;i++){
                    for(int j = 0; j < nitem;j++)
                        memcpy(&wave_data[i][j], packet_recv_buf + FRAME_HEAD_LENGTH + jlen + 4*(i + j*chn), 4);
                }
            } else if(6 == packet_recv_jdata["cmd"]){
                //初始化vector大小
                mpu_data.clear();
                int chn = packet_recv_jdata["data"]["chn"];
                int nitem = packet_recv_jdata["data"]["nitem"];
                int jlen = recv_packet_head.jlen;
                mpu_data.resize(chn);
                for(int i = 0; i < chn;i++)
                    mpu_data[i].resize(nitem);
                //填充vector
                for(int i = 0; i < chn;i++){
                    for(int j = 0; j < nitem;j++)
                        memcpy(&mpu_data[i][j], packet_recv_buf + FRAME_HEAD_LENGTH + jlen + 4*(i + j*chn), 4);
                }
            }
        }
    }



    return 0;
}

/*
 * 设置套接字超时时间
*/
bool setSocketTimeout(int sockfd, int timeoutSec, int option) {
    struct timeval timeout;
    timeout.tv_sec = timeoutSec;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, option, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
        perror("Setting socket timeout failed");
        return false;
    }

    return true;
}

/*
 * 客户端线程
 * 线程可以调用客户端对象的公有变量成员，不同线程的共有变量成员不同
 * TODO heart beat 1z for remain connect and acquire battery status etc.
*/
void brain_client::client_thread(std::map<std::string, brain_client*> *nirs_dev){
    int ret;
    nlohmann::json jcmd;
    nlohmann::json jdata;
    std::string jstr;
    bool acq = false;
    bool mpu = false;
    bool sdataFinish = false;
    bool mpudataFinish = false;
    uint32_t hb_acq_count = 0;
    bool hb_acq = false;
    server_queue_t sq={.clientSn = clientSn};
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));   //sleep for 1s
    while(running){

        auto start_time = std::chrono::high_resolution_clock::now();

        if(acq && !mpu && !sdataFinish && !hb_acq){ //wave acq
            jcmd = {
                {"cmd",  5},
                {"scmd", 0}
            };

            if(mpudataFinish) mpu = false; else mpu = true;
        }
        else if(acq && mpu && !mpudataFinish && !hb_acq){ // mpu acq
            jcmd = {
                {"cmd",  6},
                {"scmd", 0}
            };

            if(sdataFinish) mpu = true; else mpu = false;
        }
        else{   //hb
            jcmd = {
                {"cmd",  3},
                {"scmd", 2}
            };

            hb_acq = false;
        }
        //std::cout << "acq:" << acq << " mpu:" << mpu << " sF:" << sdataFinish << " mF:" << mpudataFinish << std::endl;
        jstr = jcmd.dump();
        pfun->send_cmd(clientSocket, jstr, nullptr, 0);

        ret = pfun->recv_frame(clientSocket);
        if(ret){
            std::cerr << "recv_frame error code: " << ret << std::endl;
            if(ret == -4) break;
            else continue;
        }

        //成功接收到了数据包，获取jdata
        jdata = pfun->get_recv_jdata();
        //std::cout << jdata << std::endl;

        //心跳包
        if((jdata["cmd"] == 3) && (jdata["scmd"] == 2)){
            if(jdata.find("vtg") != jdata.end()){
                battery = jdata["vtg"]; //获取电压
                p_server_queue->push({.clientSn = clientSn, .type = "heartbeat", .battery = battery});
            }
        }
        //如果blen有波形/mpu数据，需要用户层先发送4，0
        //std::cout << "blen length: " << pfun->get_recv_packet_head().blen << std::endl;
        if(pfun->get_recv_packet_head().blen){
            //std::cout << "enter blen process." << std::endl;
            /*
             * 将波形数据发送打用户层
             *      1. 如果jdata["data"]中有"ms" 给startms和mpustarttime赋值为ms，否则startms赋值为0
             *      2. 将bin data数据从缓冲区拿出来
             *      3. 判断接收的jdata的"cmd"，如果是5，将wave数据从bdata中解析出来，并且通过sdataqueue push wave、sn、chn和startms到用户层
             *         如果是6，解析响应的mpu数据，保存在list中，ValCnt？
            */
//            if(jdata["data"].find("ms") != jdata["data"].end()){
//                sq.startms = jdata["data"]["ms"];
//            } else{
//                sq.startms = 0;
//            }

            //接收到wave数据 TODO 接收到mpu数据
            if(jdata["cmd"] == 5){
                //std::vector<std::vector<int32_t>> wave_data = pfun->get_wave_data();
                //如果有ms，说明采样刚开始，发送startms到用户层
//                if(jdata["data"].find("ms") != jdata["data"].end())
//                    p_server_queue->push({.clientSn = clientSn, .type = "wave", .startms = jdata["data"]["ms"], .wave_data = pfun->get_wave_data()});
//                else
//                    p_server_queue->push({.clientSn = clientSn, .type = "wave", .startms = 0, .wave_data = pfun->get_wave_data()});
                p_server_queue->push({.clientSn = clientSn, .type = "wave", .index = jdata["data"]["index"], .wave_data = pfun->get_wave_data()});
            } else if(jdata["cmd"] == 6){
                    p_server_queue->push({.clientSn = clientSn, .type = "mpu", .index = jdata["data"]["index"], .mpu_data = pfun->get_mpu_data()});
            }

        }

        //LED和MPU数据结束接收标志
        if(acq && (jdata["cmd"] > 3)){
            if((jdata["cmd"] == 5) && (jdata["data"]["finish"] == 1)) sdataFinish = true;
            else if( (jdata["cmd"] == 6) && (jdata["data"]["finish"] == 1)) mpudataFinish = true;
        }
        //测量结束
        if(acq && sdataFinish && mpudataFinish){
            //TODO: push ‘done’ through sdataqueue to user layer
            acq = false;
            sdataFinish = false;
            mpudataFinish = false;
            p_server_queue->push({.clientSn = clientSn, .type = "done"});
        }

        //处理用户层的cmd start stop
        //检查队列是否为空
        if(!client_queue.empty()){
            //不为空，处理第一个队列指令
            client_cmd_t cmd;
            client_queue.pop(cmd);
            //std::cout << "client got cmd: " << cmd.cmd << std::endl;
            //开始采集任务
            if(cmd.cmd == "start"){
                acq = true;
                hb_acq_count = 0;
                //convert channel map to device map
                std::vector<uint32_t> enable_channels_mapped;
                for(auto it : cmd.config.enable_channels) enable_channels_mapped.push_back(CHANNEL_MAP[it]);
                //start pd sample
                jcmd = {
                    {"cmd", 4},
                    {"scmd", 1},
                    {"data",{
                              {"type", 0},
                              {"sample_rate", cmd.config.sample_rate},
                              {"sample_delay", cmd.config.sample_delay},
                              {"nburst", cmd.config.nburst},
                              {"sample_np", cmd.config.sample_duration*cmd.config.sample_rate},
                              {"channels", enable_channels_mapped},
                              //{"channels", cmd.config.enable_channels},
                              {"dac", cmd.config.led_strength},
                              {"gain", cmd.config.sample_adc_gain},
                              {"ms", std::time(nullptr)*1000}
                        }
                    }
                };
                jstr = jcmd.dump();
                //std::cout << "cmd start: " << jstr << std::endl;
                pfun->send_cmd(clientSocket, jstr, nullptr, 0);

                ret = pfun->recv_frame(clientSocket);
                if(ret){
                    std::cerr << "recv_frame error code: " << ret << std::endl;
                    if(ret == -4) break;
                    else continue;
                }

                //成功接收到了数据包，获取jdata
                jdata = pfun->get_recv_jdata();
                //std::cout << jdata << std::endl;

                //start mpu sample
                jcmd = {
                    {"cmd", 4},
                    {"scmd", 2},
                    {"data",{
                              {"type", 1},
                              {"sample_rate", cmd.config.sample_rate},
                              {"sample_np", cmd.config.sample_duration*cmd.config.sample_rate},
                              {"aresGain", cmd.config.mpu_ares_gain},
                              {"gresGain", cmd.config.mpu_gres_gain},
                              {"ms", std::time(nullptr)*1000}
                            }
                    }
                };
                jstr = jcmd.dump();
                //std::cout << "cmd start: " << jstr << std::endl;
                pfun->send_cmd(clientSocket, jstr, nullptr, 0);

                int ret = pfun->recv_frame(clientSocket);
                if(ret){
                    std::cerr << "recv_frame error code: " << ret << std::endl;
                    if(ret == -4) break;
                    else continue;
                }

                //成功接收到了数据包，获取jdata
                //jdata = pfun->get_recv_jdata();
                //std::cout << jdata << std::endl;
            } else if(cmd.cmd == "stop"){
                //stop acq
                acq = false;
                jcmd = {
                    {"cmd", 4},
                    {"scmd",0}
                };
                jstr = jcmd.dump();
                //std::cout << "cmd start: " << jstr << std::endl;
                pfun->send_cmd(clientSocket, jstr, nullptr, 0);

                int ret = pfun->recv_frame(clientSocket);
                if(ret){
                    std::cerr << "recv_frame error code: " << ret << std::endl;
                    if(ret == -4) break;
                    else continue;
                }

                //成功接收到了数据包，获取jdata
                jdata = pfun->get_recv_jdata();
                //std::cout << jdata << std::endl;
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        //获取上面代码运行时长
        auto duration_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        //补偿延时
        //采样状态延时100ms,5s一次hb
        //非采样状态1000ms
        if(acq){
            if(duration_time.count() < 100) std::this_thread::sleep_for(std::chrono::milliseconds(100-duration_time.count()));
            hb_acq_count++;
            if(hb_acq_count >= 50) hb_acq_count = 0, hb_acq = true;
        } else {
            if(duration_time.count() < 1000) std::this_thread::sleep_for(std::chrono::milliseconds(1000-duration_time.count()));
        }

    }
    //客户端线程退出
    std::cout << clientSn << " closesocket" << std::endl;
    delete pfun;
    closesocket(clientSocket);
    p_server_queue->push({.clientSn = clientSn, .type = "disconnected", .clientIP = clientAddr});
    nirs_dev->erase(clientSn);
}

brain_server::brain_server(){

}
/*
 * 服务端线程
 * 处理新的客户端连接请求，实现客户端并发
 * 服务端不需要关闭客户端socket，在客户端进程退出时自动关闭
*/
void brain_server::server_thread(){
    pfun = new packet_funs();

    while(running){
        std::cout << "waiting for connection..." << std::endl;

        // 获取当前系统时间的时间戳
        std::time_t currentTime = std::time(nullptr);
        // 将时间戳转换为字符串格式并输出
        std::cout << std::ctime(&currentTime);

        //accept客户端连接
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
// new client accept
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error accepting connection: " << WSAGetLastError() << std::endl;
            continue;
        }
        //打印当前连接的客户端地址
        std::cout << "connected from: " << inet_ntoa(clientAddr.sin_addr) << ":" << clientAddr.sin_port<< std::endl;

        // 设置客户端为阻塞模式
        u_long client_mode = 0; // 0 表示阻塞模式
        if (ioctlsocket(clientSocket, FIONBIO, &client_mode) != 0) {
            std::cerr << "ioctlsocket failed" << std::endl;
            closesocket(clientSocket);
            continue;
        }

        //套接字接收超时10s，在客户端线程用于检测recv超时
        int timeout = 10000;
        if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
            std::cerr << "setsockopt failed" << std::endl;
            closesocket(clientSocket);
            continue;
        }

        //发送dev info指令
        nlohmann::json jcmd = {
            {"cmd" , 1},
            {"scmd", 0}
        };
        std::string jstr = jcmd.dump();

        //new packet_funs获取设备信息sn等
        pfun->send_cmd(clientSocket, jstr, nullptr, 0);

        //TODO: 超时异常检测
        int ret = pfun->recv_frame(clientSocket);
        if(ret){
            std::cerr << "recv_frame error code: " << ret << std::endl;
            closesocket(clientSocket);
            continue;
        }

        //成功接收到了数据包，获取jdata
        nlohmann::json jdata = pfun->get_recv_jdata();
        //std::cout << jdata << std::endl;

        //获取sn码（MAC地址）作为nirs设备唯一标识
        std::string sn = jdata["data"]["sn"];

        //如果重复连接了同一个nirs设备
        if(nirs_dev.find(sn) != nirs_dev.end()){
            delete nirs_dev[sn]; //调用~brain_client(), 内部free, (erase方法不会delete)
            nirs_dev.erase(sn);
            std::cerr << "reconnect Sn: " << sn << std::endl;
            continue;
        }

        //创建client_dev成员
        brain_client *client_dev = new brain_client(inet_ntoa(clientAddr.sin_addr), sn, clientSocket, &server_queue);
        //开启客户端线程
        client_dev->client_startThread(&nirs_dev);
        //将设备添加到设备map中
        nirs_dev[sn] = client_dev;

        //向用户层发送设备连接消息
        server_queue.push({.clientSn = sn, .type = "connected", .clientIP =  inet_ntoa(clientAddr.sin_addr), .battery = jdata["data"]["Battery"]});
    }
}

/*
 *  加载win套接字库 WSA
 *  初始化serversocket
 *  绑定套接字并开启监听
 *  开启一个服务端线程
*/
void brain_server::start(){

    //初始化WSA
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return;
    }

    //初始化serversocket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket." << std::endl;
        return;
    }

    //指定服务器监听端口和地址
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    //绑定套接字
    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding socket." << std::endl;
        return;
    }

    //开启服务器监听
    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error listening on socket." << std::endl;
        return;
    }

    //开启服务端线程
    running = true;
    std::thread th(&brain_server::server_thread, this);
    th.detach();
}

/*
 * 所有客户端停止采样
*/
void brain_server::stop(){
    for(auto it = nirs_dev.begin(); it != nirs_dev.end(); ++it){
        nirs_dev[it->first]->client_set_running(false);
    }

}

/*
 * 用户层向协议层传递控制nirs设备参数，命令和参数待确定！！！
*/
void brain_server::put_cmd(std::string sn, std::string cmd){
    client_config_t config;
    if(nirs_dev.find(sn) != nirs_dev.end()){
        nirs_dev[sn]->client_client_queue_push({.cmd = cmd, .config = config});
    } else{
        std::cerr << "nirs dev not in list!!!" << std::endl;
    }
}
void brain_server::put_cmd(std::string sn, std::string cmd, client_config_t config){
    if(nirs_dev.find(sn) != nirs_dev.end()){
        nirs_dev[sn]->client_client_queue_push({.cmd = cmd, .config = config});
    } else{
        std::cerr << "nirs dev not in list!!!" << std::endl;
    }
}

/*
 * 用户层获取数据队列
 * 使用时注意应先判断返回值的clientSn是否在nirs_dev中
*/
server_queue_t brain_server::get_msg(){
    server_queue_t ret;
    if(!server_queue.empty()){
        server_queue.pop(ret);
        return ret;
    } else{
        ret = {.type = "dummy"}; //invalid Type
        return ret;
    }
}


