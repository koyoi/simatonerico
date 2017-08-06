#pragma once

#include "hal_graphics.h"
#include <opencv2/opencv.hpp>

typedef disp_base base;

class disp_ocv2 : public disp_base
{
public:

private:
	static int font_name;
	static int font_size;

	static cv::Scalar	line_color_cv;
	static cv::Scalar	fill_color_cv;

	static cv::Mat	img;
	static cv::Mat	img_z;


public:
	void	disp_init(jhl_xy_i window_size_);
	void	disp_show();
	void	disp_clear();
	void	disp_swap();
	void	disp_destroy();

	void	set_lineColor(jhl_rgb c) {
		line_color = c;
		line_color_cv = CV_RGB(c.r*255, c.g * 255, c.b * 255);
	}
	void	set_fillColor(jhl_rgb c) {
		fill_color = c;
		fill_color_cv = CV_RGB(c.r * 255, c.g * 255, c.b * 255);
	}

	void	point(jhl_xy_i& pos, int size = 1);
	void	point(jhl_xy_i& pos, jhl_rgb c);
	void	point(jhl_xy_i& pos, cv::Scalar color);
	void	line(jhl_xy_i& start, jhl_xy_i& end);
	void	line_f(jhl_xy_i& start, jhl_xy_i& end);
	void	line_h(unsigned int v, float h_start, float h_end);

	void	point_z(jhl_xy_i& pos, float depth);

	void	circle(jhl_xy_i& pos, int r, jhl_rgb& color, int thickness = 1);
	void	rectangle(jhl_xy_i& start, jhl_xy_i& end, jhl_rgb& color, int thickness = 1);

	void	fontInit()
	{
		font_name = cv::FONT_HERSHEY_PLAIN;
		font_size = 1;
	}
	void	putText(const char* s, jhl_xy_i& pos, jhl_rgb& color);

};


