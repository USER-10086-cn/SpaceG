#include "motor.h"
#include "lq_all_demo.hpp"
#include "tracking.h"
ls_encoder_pwm motor_l(ENC_PWM2_PIN66, PIN_74);
ls_encoder_pwm motor_r(ENC_PWM3_PIN67, PIN_75);
ls_pwm pwm_l(PWM0_PIN64, 25000, 1000);
ls_pwm pwm_r(PWM1_PIN65, 25000, 1000);
ls_gpio gpio1(PIN_72, GPIO_MODE_OUT);
ls_gpio gpio2(PIN_73, GPIO_MODE_OUT);
float get_l_motor_speed()
{

    float speed_l=motor_l.encoder_get_count();
    return speed_l*0.0325;
};
//��ȡ�ҵ����ת��
float get_r_motor_speed()
{
    float speed_r=motor_r.encoder_get_count();
    return speed_r*0.0325;
}
void set_motor_pwm(int left,int right)//�������ҵ����pwmֵ
{
    int max_pwm=4000;//最大速度
    //����޷�
    if(left<0)
    {
        left=-left;
        gpio1.gpio_level_set(GPIO_LOW);
    }
    else
    {
        gpio1.gpio_level_set(GPIO_HIGH);
    }

    if(right<0)
    {
        right=-right;
        gpio2.gpio_level_set(GPIO_HIGH);
    }
    else
    {

        gpio2.gpio_level_set(GPIO_LOW);
    }
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