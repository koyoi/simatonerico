#pragma once
#include <cmath>

#include "basic3dCalc.h"
//#include "hal_graphics.h"
#include "hal_gfx_ocv.h"


struct viewport_config {
	int vp_far;
	int vp_near;
	int left, right;
	int top, btm;
};

enum En_draw_type {
	drawType_vertex,
	drawType_line,
	drawType_line_front_face,
	drawType_flat
};


struct grpAttrib
{
	int		n_member;
	int*    member;

	char*	texName;
	char*	pTex;
	texUv*	uv;

	jhl_rgb	color;
};


class modelData {
public:
	static void dataDump(modelData&, bool detail = false);

public:
	int			n_vert;
	jhl_xyz*	vert;	// vert は配列のつもりなのだが、これでいいらしい
	int			n_pol;
	pol_def*	poldef;
	int			n_group;
	grpAttrib*	attr;

};


struct object {
	matHomo4		model_mat;
	modelData*		p_model;

	// todo もっといい実装
	bool		attrib_override;
	jhl_rgb		color;
};

// ------------------------------------------------------------

class jhl3Dlib
{

public:
	static En_draw_type	draw_type;

	static viewport_config	vp;
	static matHomo4			view_mat;	//	ビュー変換(モデル変換は tgtMdl 持ち)

	static matHomo4_full	proj_mat;	//	投影変換

	static matHomo4			disp_mat;	//	ディスプレイ変換
	static jhl_xy_i			display;	

	static matHomo4_full	transMat;

	static jhl_rgb		light_ambient;
	static dir_light	light_directional[2];

private:
	static modelData*	tgtMdl;
	static disp_ocv2*	painter;
//	static disp_base*	painter;		// アクセスできない基底クラスうんたら

public:
//	static void		set_painter(disp_base& p) { painter = &p; };
	static void		set_painter(disp_ocv2& p) { painter = &p; };	
	
	static void		set_view_mat(const jhl_xyz& eye_loc, const jhl_xyz& u_vec, const jhl_xyz& tgt_loc);
	static void		set_proj_mat(viewport_config& m_, bool ortho = false);
	static void		set_disp_trans(const jhl_xy_i& window);
	/*
	↑のため、廃止予定
	static matHomo4 disp_trans(const jhl_xy_i&  window);
	static matHomo4 disp_trans_inv(matHomo4& mat_disp_trans);
	*/
	static void		setTransMat(matHomo4& mdl_mat);

	static jhl_xyz jhl3Dlib::proj_disp_to_normal_box(float wari, jhl_xyz& p0, jhl_xyz& p1);

	static float	check_side(int pol_idx);
	static jhl_rgb	calc_lighting(int pol_idx);

//	static int		setTgtMdl(modelData* t) { tgtMdl = t; };

	static jhl_rgb	set_light_ambient(jhl_rgb l) { light_ambient = l; };
	static dir_light	set_light_directional(int idx, dir_light ld) {
		if (idx < 2) {
			light_directional[idx] = ld;
		}
	}

	static int draw(object& mdl);

	static En_draw_type draw_type_next();
};
    

                      