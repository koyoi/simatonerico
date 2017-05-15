#pragma once
#include <cmath>

#include "basic3dCalc.h"
//#include "hal_graphics.h"
#include "hal_gfx_ocv.h"


//#define DISP_MAT_KAKIKUDASHI

#define NUM_DIR_LIGHT	2


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
	drawType_flat,
	drawType_flat_lighting,
	drawType_flat_z,
/* ��������
		drawType_flat_z_lighting,
		drawType_phong,
*/
drawType_max_,
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
	jhl_xyz*	verts;	// vert �͔z��̂���Ȃ̂����A����ł����炵��
	int			n_pol;
	pol_def*	poldef;
	int			n_group;
	grpAttrib*	attr;

};


struct object {
	matHomo4		model_mat;
	modelData*		p_model;

	// todo �����Ƃ�������
	bool		attrib_override;
	jhl_rgb		color;
};

// ------------------------------------------------------------

class jhl3Dlib
{

public:
	friend disp_base;


// settings
private:
	static En_draw_type		draw_type;

	static viewport_config	vp;
	static jhl_xy_i			display;	// �L�����o�X�T�C�Y


private:
	static modelData*	tgtMdl;
//	static int			setTgtMdl(modelData* t) { tgtMdl = t; };

	static disp_ocv2*	painter;
//	static disp_base*	painter;		// �A�N�Z�X�ł��Ȃ����N���X���񂽂�

public:
//	static void			set_painter(disp_base& p) { painter = &p; };
	static void			set_painter(disp_ocv2& p) { painter = &p; };


// �ϊ��s��֌W
private:
	static matHomo4			view_mat;	//	�r���[�ϊ�(���f���ϊ��� tgtMdl ����)
	static matHomo4_full	proj_mat;	//	���e�ϊ�
#ifdef DISP_MAT_KAKIKUDASHI
	static matHomo4			disp_mat;	//	�f�B�X�v���C�ϊ�
#endif
	
	static matHomo4			modelViewMat;	// ���f���r���[�ϊ��s��i�����v�Z�Ɏg���j
	static matHomo4_full	transMat;	// �ŏI�I�ȕϊ��s��

public:
	static void		set_view_mat(const jhl_xyz& eye_loc, const jhl_xyz& u_vec, const jhl_xyz& tgt_loc);
	static void		set_proj_mat(viewport_config& m_, bool ortho = false);
	static void		set_disp_trans(const jhl_xy_i& window);

	static void		setTransMat(const matHomo4& mdl_mat);

	static jhl_xyz	jhl3Dlib::proj_disp_to_normal_box(float wari, jhl_xyz& p0, jhl_xyz& p1);

	static float	check_side(jhl_xyz* verts);


// �Ɩ�
public:
	static jhl_rgb*		light_ambient;
	static dir_light*	light_directional;
	static int			num_light_dir;
private:
	static jhl_rgb		light_calced;

public:
//	static jhl_rgb		calc_lighting(pol_def* pol);
	static void			calc_lighting(pol_def* pol);

#if 0
	static jhl_rgb		set_light_ambient(jhl_rgb l) { light_ambient = l; };
	static dir_light	set_light_directional(int idx, dir_light ld) {
		if (idx < 2) {
			light_directional[idx] = ld;
		}
	}
#endif


// �`��
	static int draw(const object& mdl);

	static En_draw_type draw_type_next();
	static void		set_draw_type(::En_draw_type type)
	{
		draw_type = type;
	}

private:
	static jhl_xyz	transToDisp(int vert_idx);
	static jhl_xyz* p_TTDcache;		// ���_���̃f�B�X�v�����W�ւ̕ϊ����ʂ��L���b�V������
	static int		TTDcacheSize;
	static	char*	mdl_name;

public:
	static int	transToDisp_cache_init(int cacheSize);
	static void	transToDisp_cache_clear();
	static void	transToDisp_cache_deinit();

};
    


