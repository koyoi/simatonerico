#pragma once

#include "basic3dCalc.h"



class disp_base
{
public:
	static jhl_xy_i	window_size;

	static float	v;				// 水平ラインを引くことが多いので
	static jhl_rgb	line_color;		// ラインを続けて同じ色を引くことが多いので
	static int		line_width;


public:
	virtual void	disp_init(jhl_xy_i window_size) = 0;
	virtual void	line(jhl_xy_i start, jhl_xy_i end, bool draw_z = 1 ) = 0;	// このときは既にディスプレイ座標になっているはずなのだ
	virtual void	line_h(float v, float h_start, float h_end) = 0;
	virtual void	disp_show() = 0;
	virtual void	circle(jhl_xy_i pos, int r, jhl_rgb, int thickness = 1 ) = 0;
	virtual void	rectangle(jhl_xy_i start, jhl_xy_i end, jhl_rgb color, int thickness = 1) = 0;
	virtual void	putText(const char* s, jhl_xy_i pos, jhl_rgb color) = 0;
	virtual void	disp_clear() = 0;
};



class disp_ocv2 : disp_base
{
public:
	static cv::Mat	img;
	static cv::Mat	img_z;

	static int font_name;
	static int font_size;


public:
	void	disp_init(jhl_xy_i window_size_);
	void	disp_show();
	void	disp_clear();
	void	disp_swap();
	void	disp_destroy();

	void	line(jhl_xy_i start, jhl_xy_i end, bool draw_z = 1);
	void	line_h(float v, float h_start, float h_end) {};

	void	circle(jhl_xy_i pos, int r, jhl_rgb color, int thickness = 1);
	void	rectangle(jhl_xy_i start, jhl_xy_i end, jhl_rgb color, int thickness = 1);

	void	fontInit()
	{
		font_name = cv::FONT_HERSHEY_PLAIN;
		font_size = 1;
	}
	void	putText(const char* s, jhl_xy_i pos, jhl_rgb color);

};

