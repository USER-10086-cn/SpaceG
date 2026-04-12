#include "pid.h"
#include "tracking.h"
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "motor.h"
PID_Controller vision_pid(0.0f,0.0f,0.0f,0.0f,0.0f);//外环位置环控制pd
PID_Controller motor_pid_l(0.0f,0.0f,0.0f,0.0f,1.0f);//内环速度环控制pid
PID_Controller motor_pid_r(0.0f,0.0f,0.0f,0.0f,1.0f);
float based_speed=60.0;//60转每分钟
float MAX_SPEED=100.0f;
int stop_flag=0;
//最小二乘法拟合中线函数
bool fitLineLS(const std::vector<cv::Point>& points, float& k,float& b)
{
    int n = points.size();//计算点数
    if (n < 5) return false;//至少两个点才能拟合直线

    long long sum_x =0,sum_y=0;
    long long sum_xy=0,sum_yy=0;
    for (int i=0;i<n;i++)
    {
        sum_x+=points[i].x;
        sum_y+=points[i].y;
        sum_xy+=points[i].x*points[i].y;
        sum_yy+=points[i].y*points[i].y;
    }
    //计算分母
    long long denominator=(n*sum_yy-sum_y*sum_y);
    if (denominator == 0)//若分母为0，说明y坐标相同
        return false;
    k=(float)(n*sum_xy-sum_x*sum_y)/denominator;
    b=(float)(sum_x-k*sum_y)/n;
    return true;
}

//中线处理函数，用于预测y=120时x的值
float img_process(const std::vector<cv::Point>& points)
{
    float target_y=20.0f;
    float k=0.0f;
    float b=0.0f;
    if (fitLineLS(points,k,b)==true)
    {
        float predicted_x=k * target_y + b ;
        return predicted_x;
    }
    else
    {
      int stop_flag=1;
    }
    return 40.0f;
}

//位置环速度环闭环控制逻辑代码
void Car_contorl_loop(const std::vector<cv::Point>& points)
{
    float predicted_x=img_process(points);
    //简单丢线保护
    if (points.size() < 5)//点太少，可能丢线
    {
        motor_pid_l.Reset();
        motor_pid_r.Reset();
        set_motor_pwm(0,0);//紧急停车
        return;
    }
    //计算外环位置环输入并输出
    vision_pid.setTarget(40.0f);//目标x坐标
    float turn_output=vision_pid.PID_Calculate(predicted_x);//预测x坐标，并输出pd控制下的坐标差
    float target_speed_l=based_speed + turn_output;
    float target_speed_r=based_speed - turn_output;
    //输出限幅
    if (target_speed_l>MAX_SPEED) target_speed_l=MAX_SPEED;
    if (target_speed_r>MAX_SPEED) target_speed_r=MAX_SPEED;
    //将目标速度设置给内环pid
    motor_pid_l.setTarget(target_speed_l);
    motor_pid_r.setTarget(target_speed_r);
    //通过编码器获得实际速度
    float motor_speed_l=get_l_motor_speed();
    float motor_speed_r=get_r_motor_speed();
    //获取目标pwm
    float motor_pwm_l=motor_pid_l.PID_Calculate(motor_speed_l);
    float motor_pwm_r=motor_pid_r.PID_Calculate(motor_speed_r);
    set_motor_pwm(motor_pwm_l,motor_pwm_r);
}