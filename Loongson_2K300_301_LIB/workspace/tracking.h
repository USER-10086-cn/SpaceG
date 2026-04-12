#ifndef TRACKING_H
#define TRACKING_H
#include <opencv2/opencv.hpp>
#include <vector>
extern int stop_flag;
bool fitLineLS(const std::vector<cv::Point>& points, float& k,float& b);
float img_process(const std::vector<cv::Point>& points);
void Car_contorl_loop(const std::vector<cv::Point>& points);
#endif
