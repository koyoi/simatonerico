#include "stdafx.h"

#include "hal_graphics.h"
#include "basic3dCalc.h"


float	disp_base::v;
jhl_rgb	disp_base::line_color;
int		disp_base::line_width;
jhl_rgb	disp_base::fill_color;
jhl_xy_i disp_base::window_size;




/*
fill:	true: �h��Ԃ�	todo: fill color
front:	true: ���ʕ`�悹��	todo
light:	true: �����̉e���L��	todo
*/
void disp_base::
triangle(jhl_xyz verts[3], bool fill, bool front, bool light)
//	�n���A�h���X�̓f�B�X�v���C���W
{
	line(verts[0], verts[1], false);
	line(verts[1], verts[2], false);
	line(verts[0], verts[2], false);
}

