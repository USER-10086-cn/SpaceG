#include "motor.h"
#include "lq_all_demo.hpp"
#include "tracking.h"
ls_encoder_pwm motor_l(ENC_PWM2_PIN66, PIN_74);
ls_encoder_pwm motor_r(ENC_PWM3_PIN67, PIN_75);
ls_pwm pwm_l(PWM0_PIN64, 100, 100);
ls_pwm pwm_r(PWM1_PIN65, 100, 100);

float get_l_motor_speed()
{

    float speed_l=motor_l.encoder_get_count();
    return speed_l;
};
//��ȡ�ҵ����ת��
float get_r_motor_speed()
{
    float speed_r=motor_r.encoder_get_count();
    return speed_r;
}
void set_motor_pwm(int left,int right)//�������ҵ����pwmֵ
{
    int max_pwm=300;//最大速度
    //����޷�
    if (left>max_pwm) left=max_pwm;
    if (right>max_pwm) right=max_pwm;
    if (stop_flag!=1)//ֹͣλ���ߴ���
    {
        pwm_l.pwm_set_duty(left);
        pwm_r.pwm_set_duty(right);
    }
    else
    {
        pwm_l.pwm_set_duty(0);
        pwm_r.pwm_set_duty(0);
    }
}