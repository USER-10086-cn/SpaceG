#include <opencv2/opencv.hpp>
#include <iostream>
#include "pid.h"
using namespace cv;
using namespace std;
//���캯����ʼ��
PID_Controller::PID_Controller(float p, float i, float d, float out_limit,float integral_limit)//pid输出，积分
{
    kp=p;
    ki=i;
    kd=d;
    max_out=out_limit;//�������޷�
    max_intergal=integral_limit;
    target=0.0f;
}
void PID_Controller::setTarget(float t)//�趨Ŀ��ֵ
{
    target=t;
}
//�������������
float PID_Controller::PID_Calculate(float measured)
{
    float error=(target-measured);
    integral+=error;

    if (integral>max_intergal)  integral=max_intergal;
    else if (integral<-max_intergal)  integral=-max_intergal;

    float output = kp*error + ki*integral + kd*(error-error_prev);

    error_prev=error;

    if (output>max_out) output = max_out;
    else if (output<-max_out) output = -max_out;
    return output;
}
//���û����Է��쳣���
void PID_Controller::Reset()
{
    integral=0.0f;
    error_prev=0.0f;
}
//
// Created by 24695 on 2026/4/8.
//