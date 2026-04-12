#include "pid.h"
#include "tracking.h"
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "motor.h"
PID_Controller vision_pid(0.0f,0.0f,0.0f,0.0f,0.0f);//�⻷λ�û�����pd
PID_Controller motor_pid_l(0.0f,0.0f,0.0f,0.0f,1.0f);//�ڻ��ٶȻ�����pid
PID_Controller motor_pid_r(0.0f,0.0f,0.0f,0.0f,1.0f);
float based_speed=60.0;//60תÿ����
float MAX_SPEED=100.0f;
int stop_flag=0;
//��С���˷�������ߺ���
bool fitLineLS(const std::vector<cv::Point>& points, float& k,float& b)
{
    int n = points.size();//�������
    if (n < 5) return false;//����������������ֱ��

    long long sum_x =0,sum_y=0;
    long long sum_xy=0,sum_yy=0;
    for (int i=0;i<n;i++)
    {
        sum_x+=points[i].x;
        sum_y+=points[i].y;
        sum_xy+=points[i].x*points[i].y;
        sum_yy+=points[i].y*points[i].y;
    }
    //�����ĸ
    long long denominator=(n*sum_yy-sum_y*sum_y);
    if (denominator == 0)//����ĸΪ0��˵��y������ͬ
        return false;
    k=(float)(n*sum_xy-sum_x*sum_y)/denominator;
    b=(float)(sum_x-k*sum_y)/n;
    return true;
}

//���ߴ�������������Ԥ��y=120ʱx��ֵ
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

//λ�û��ٶȻ��ջ������߼�����
/void Car_contorl_loop(const std::vector<cv::Point>& points)
{
    float predicted_x=img_process(points);
    //�򵥶��߱���
    if (points.size() < 5)//��̫�٣����ܶ���
    {
        motor_pid_l.Reset();
        motor_pid_r.Reset();
        set_motor_pwm(0,0);//����ͣ��
        return;
    }
    //�����⻷λ�û����벢���
    vision_pid.setTarget(40.0f);//Ŀ��x����
    float turn_output=vision_pid.PID_Calculate(predicted_x);//Ԥ��x���꣬�����pd�����µ������
    float target_speed_l=based_speed + turn_output;
    float target_speed_r=based_speed - turn_output;
    //����޷�
    if (target_speed_l>MAX_SPEED) target_speed_l=MAX_SPEED;
    if (target_speed_r>MAX_SPEED) target_speed_r=MAX_SPEED;
    //��Ŀ���ٶ����ø��ڻ�pid
    motor_pid_l.setTarget(target_speed_l);
    motor_pid_r.setTarget(target_speed_r);
    //ͨ�����������ʵ���ٶ�
    float motor_speed_l=get_l_motor_speed();
    float motor_speed_r=get_r_motor_speed();
    //��ȡĿ��pwm
    float motor_pwm_l=motor_pid_l.PID_Calculate(motor_speed_l);
    float motor_pwm_r=motor_pid_r.PID_Calculate(motor_speed_r);
    set_motor_pwm(motor_pwm_l,motor_pwm_r);
}