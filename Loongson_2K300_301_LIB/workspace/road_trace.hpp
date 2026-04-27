#ifndef ROAD_TRACE_HPP
#define ROAD_TRACE_HPP

// 对外暴露的 3 个函数
void vision_init(void);
void vision_process_image(void);
void vision_follow_center_line(void);
void print_camera(void);
#endif