
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include "pid.h"
#include "lq_udp_client.hpp"
#include <atomic>
#include "motor.h"
#include <thread>
#include <future>
using namespace std;
lq_udp_client VOFA_Send("192.168.104.200", 8080); // 修改为自己电脑的ip，vofa监听端口为8080
const uint8_t vofa_tail[4] = {0x00, 0x00, 0x80, 0x7f};//justfloat帧尾
// 发送程序
/*
@brief:下位机向上位机发送数据
@param：target是目标位置，actual是实际位置
*/
void vofa_send_data(float target, float actual)
{
    float data[2] = {target, actual};
    VOFA_Send.udp_send(data, sizeof(data));
    VOFA_Send.udp_send(vofa_tail, 4);
    static int i =0 ;
    i++;
    if(i>=200)
    {
    printf("发送数据到上位机: 目标=%.3f, 实际=%.3f\n", target, actual);
    i=0;
    }
}
// 接收函数
/*
@brief:实现实时调参的具体函数
@param：pid为要修改的环
@example：vofa_turning_thread(vision_pid)--修改位置环pid
*/
bool vofa_turning_thread(PID_Controller &pid, float &base_speed,bool &flag) // 修改pid修改此处的参数即可
{
    int sockfd;
    struct sockaddr_in server_addr;
    // 创建UDP套接字
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket 创建失败");
        return false;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // 监听所有网卡
    server_addr.sin_port = htons(8081);       // 监听端口8081
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("原生Bind 失败！请检查8081是否被占用");
        close(sockfd);
        return false;
    }
    char rx_buf[1024];//接收缓存数组
    char type_str[10];//
    float val;
    bool motor_flag=false;
    printf("原生UDP接口已经启动，监听端口8081\n");
    while (true)
    { // 阻塞接收来自电脑的数据
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        ssize_t len = recvfrom(sockfd, rx_buf, sizeof(rx_buf) - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (len > 0)
        {
            rx_buf[len] = '\0';
            // 只要收到包，不管格式对不对，先打印出来
            printf("收到原始数据: [%s], 长度: %ld\n", rx_buf, len);
            float p, i, d;
                // 解析格式：CMD:START! 或 CMD:STOP! 或 VP:1.5 或 VD:0.5 或 BS:20
                if(strstr(rx_buf,"CMD:START!")!=NULL)
                {
                    printf("收到启动指令，电机已启动\n");
                    flag=true;
                }
                else if(strstr(rx_buf,"CMD:STOP!")!=NULL)
                {
                    flag=false;
                    set_motor_pwm(0,0);
                    printf("收到停止指令，电机已停止\n");
                }
                else if (sscanf(rx_buf, "%[^:]:%f", type_str, &val) == 2)
            {   
                if (strcmp(type_str, "VP") == 0)
                {
                    pid.getPID(p, i, d);
                    pid.setPID(val, i, d);
                    printf("VP的值已经更新为%.3f\n", val);
                }
                else if (strcmp(type_str, "VD") == 0)
                {
                    pid.getPID(p, i, d);
                    pid.setPID(p, i, val);
                    printf("VD的值已经更新为%.3f\n", val);
                }
                else if (strcmp(type_str, "BS") == 0)
                {
                    base_speed = val;
                    printf("base_speed已经更新为%.3f\n",val);
                }
            }
            else
            {
                printf("格式解析错误,收到的原始字符串是%s\n", rx_buf);
            }
            
        }
 }
            close(sockfd);
            return motor_flag;
}

// 构造函数初始化
PID_Controller::PID_Controller(float p, float i, float d, float out_limit, float integral_limit)
{
    kp = p;
    ki = i;
    kd = d;
    max_out = out_limit; // 最大输出限幅
    max_intergal = integral_limit;
    target = 0.0f;
}
void PID_Controller::setTarget(float t) // 设定目标值
{
    target = t;
}
// 计算输出量函数
float PID_Controller::PID_Calculate(float measured)
{
    float error = target - measured;
    integral += error;

    if (integral > max_intergal)
        integral = max_intergal;
    else if (integral < -max_intergal)
        integral = -max_intergal;

    float output = kp * error + ki * integral + kd * (error - error_prev);

    error_prev = error;

    if (output > max_out)
        output = max_out;
    else if (output < -max_out)
        output = -max_out;
    return output;
}
// 重置积分以防异常情况
void PID_Controller::Reset()
{
    integral = 0.0f;
    error_prev = 0.0f;
}
// 上位机修改pid参数
void PID_Controller::setPID(float p, float i, float d)
{
    kp = p;
    ki = i;
    kd = d;
}
// 板卡向上位机发送函数
void PID_Controller::getPID(float &p, float &i, float &d)
{
    p = kp;
    i = ki;
    d = kd;
}



void VOFA_Init(PID_Controller& pid,float& speed,bool &flag)
{

    std::thread tuning_t(vofa_turning_thread,std::ref(pid),std::ref(speed),std::ref(flag));
    tuning_t.detach();
    printf("VOFA_OK");
    
}