#ifndef ROAD_TRACE_HPP
#define ROAD_TRACE_HPP
#include "pid.h"
// 对外暴露的 3 个函数
void vision_init(void);
void vision_process_image(void);
void vision_follow_center_line(void);
void print_camera(void);
uint16_t element_judge(void);
void  pid_motor_control(void);
extern PID_Controller vision_pid;
extern PID_Controller motor_pid_l;
extern PID_Controller motor_pid_r;
extern float based_speed;
void tuchuan(void);
#endif