#ifndef MOTOR_H
#define MOTOR_H
float get_l_motor_speed();//获取左电机的转速
float get_r_motor_speed();//获取右电机的转速
void set_motor_pwm(float pwm_l,float pwm_r);//设置左右电机的pwm值
#endif