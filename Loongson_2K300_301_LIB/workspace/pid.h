#ifndef PID_H
#define PID_H
class PID_Controller
{
private:
    float kp;//kp系数
    float ki;//ki系数
    float kd;//kd系数

    float target;
    float error_prev=0.0f;//前一次误差，用来计算微分量
    float integral=0.0f;//积分量
    float max_out;
    float max_intergal;
public:
    PID_Controller(float p, float i, float d, float out_limit,float integral_limit);
    //处理函数
    float PID_Calculate(float measured);
    void setTarget(float target);
    void Reset();
};
#endif
