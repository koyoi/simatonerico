#include "stdafx.h"

#include "hal_graphics.h"
#include "basic3dCalc.h"


float	disp_base::v;
jhl_rgb	disp_base::line_color;
int		disp_base::line_width;
jhl_rgb	disp_base::fill_color;
jhl_xy_i disp_base::window_size;


void disp_base::
triangle(jhl_xyz _p[3])
//	渡すアドレスはディスプレイ座標
{
	// polyLines, fillPoly() があるけど、今回はこれで
	jhl_xy_i verts[3] = { _p[0], _p[1], _p[2] };

	line( verts[0], verts[1]);
	line( verts[1], verts[2]);
	line( verts[0], verts[2]);
}

