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
    //        
    float PID_Calculate(float measured);
    void setTarget(float target);
    void Reset();
    void setPID(float p,float i,float d);
    void getPID(float& p,float& i,float& d);
};
bool vofa_turning_thread(PID_Controller &pid, float &base_speed,bool &flag);
void vofa_send_data(float target,float actual);
void VOFA_Init(PID_Controller& pid,float& speed,bool &flag);
#endif
