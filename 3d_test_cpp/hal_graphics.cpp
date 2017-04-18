#include "stdafx.h"

#include <opencv2/opencv.hpp>
/*
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
*/

#include "hal_graphics.h"
#include "basic3dCalc.h"


float	disp_base::v;
jhl_rgb	disp_base::line_color;
int		disp_base::line_width;
jhl_xy_i disp_base::window_size;


cv::Mat	disp_ocv2::img;
cv::Mat	disp_ocv2::img_z;



void disp_ocv2::
disp_init(jhl_xy_i window_size_ )
{
	window_size = window_size_;
	img   = cv::Mat(cv::Size(window_size.x, window_size.y), CV_8UC3, cv::Scalar(0, 0, 0));
	img_z = cv::Mat(cv::Size(window_size.x, window_size.y), CV_16UC1, cv::Scalar(0));
}

void disp_ocv2::
disp_clear()
{
	img = cv::Mat::zeros(window_size.x, window_size.y, CV_8UC3);
	img_z = cv::Mat::zeros(window_size.x, window_size.y, CV_16UC1);

}


void disp_ocv2::
disp_show()
{
	cv::namedWindow("img", /*CV_WINDOW_AUTOSIZE |*/ cv::WINDOW_KEEPRATIO);
	cv::namedWindow("img_z", /*CV_WINDOW_AUTOSIZE |*/ cv::WINDOW_KEEPRATIO);
}


void disp_ocv2::
line( jhl_xy_i start, jhl_xy_i end, bool draw_zbuff )
{
	// todo
	cv::line(img, 
		cv::Point(start.x, start.y), 
		cv::Point(end.x, end.y),
		CV_RGB(line_color.r, line_color.g, line_color.b), 
		line_width);
}


void disp_ocv2::
circle( jhl_xy_i pos, int radius, jhl_rgb color, int thickness)
{
	cv::circle(img, cv::Point(pos.x, pos.y), radius, CV_RGB(color.r, color.g, color.b), thickness ); // 省略パラメータ有り
}

void disp_ocv2::
rectangle(jhl_xy_i start, jhl_xy_i end, jhl_rgb color, int thickness)
{
	cv::rectangle( img, cv::Point( start.x, start.y ), cv::Point( end.x, end.y ),
		CV_RGB(color.r, color.g, color.b), thickness);
}


void disp_ocv2::
putText( const char* s, jhl_xy_i pos, jhl_rgb color)
{
	cv::putText(img, s, cv::Point(pos.x, pos.y),
		cv::FONT_HERSHEY_SIMPLEX, /*font size*/5,
		CV_RGB(color.r, color.g, color.b), /*thickness*/3,
		CV_AA);
}

void disp_ocv2::
disp_swap(void)
{
	cv::imshow("img", img);
	cv::imshow("img_z", img_z);
}

void disp_ocv2::disp_destroy()
{
	cv::destroyAllWindows();
}
