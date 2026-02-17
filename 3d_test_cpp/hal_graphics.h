#pragma once

#include "basic3dCalc.h"


class disp_base
{
public:
	static jhl_xy_i	window_size;
	static float	v;				// �������C�����������Ƃ������̂�
	static int		line_width;

protected:
	static jhl_rgb	line_color;		// ���C���𑱂��ē����F���������Ƃ������̂�
	static jhl_rgb	fill_color;		// ���C���𑱂��ē����F���������Ƃ������̂�

public:
	virtual void	disp_init(jhl_xy_i window_size) = 0;
	virtual void	disp_show() = 0;
	virtual void	disp_clear() = 0;

	virtual void	set_lineColor(jhl_rgb) = 0;
	virtual void	set_fillColor(jhl_rgb) = 0;

	// �f�B�X�v���C���W��n��
	virtual void	point(jhl_xy_i& pos, int size = 3) = 0;
	virtual void	line(jhl_xy_i& start, jhl_xy_i& end) = 0;		
	virtual void	circle(jhl_xy_i& pos, int r, jhl_rgb&, int thickness = 1 ) = 0;

	virtual void	point_z(jhl_xy_i& pos, float depth) = 0;

	// ���ۃN���X�ł̎��� 
	virtual void	rectangle(jhl_xy_i& start, jhl_xy_i& end, jhl_rgb& color, int thickness = 1) = 0;	// ���f���`��ɂ͎g��Ȃ�
	void	triangle(jhl_xyz verts[3]);
	virtual void	line_h(unsigned int v, float h_start, float h_end) = 0;

	virtual void	putText(const char* s, jhl_xy_i& pos, jhl_rgb& color) = 0;

};


