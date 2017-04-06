#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "hal_graphics.h"

void disp_init(unsigned int window_v, unsigned int window_h)
{
	cv::Mat img(cv::Size(window_h, window_v), CV_8UC3, cv::Scalar(0, 0, 0));
	cv::Mat img_z(cv::Size(window_h, window_v), CV_16UC1, cv::Scalar(0));
}

#if 0

im = cv2.line(im, t[d[0]], t[d[1]], line_color, line_width)

#endif
