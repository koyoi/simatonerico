#pragma once
#include <cmath>
#include <string>

#include "basic3dCalc.h"
//#include "hal_graphics.h"
#ifdef USE_GDIPLUS
#include "hal_gfx_gdiplus.h"
#else
#include "hal_gfx_ocv.h"
#endif


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
	drawType_tex,
	/* ��������
		drawType_phong,
*/
drawType_max_,
};

static const char *draw_type_str[] {
	"drawType_vertex",
	"drawType_line",
	"drawType_line_front_face",
	"drawType_flat",
	"drawType_flat_lighting",
	"drawType_flat_z",
	"drawType_tex",
	/* ��������
	"drawType_phong,
	*/
	"drawType_max_",
};


enum attr_type {
	eATTR_FLAT,
	eATTR_TEX
};

#if 0
class attrib_base
{
public:
	int hoge;
};
#endif

struct attrib_tex
{
	//	char*	texName;
	std::string	texName;
	//	char*	Tex;
#ifdef USE_GDIPLUS
	Gdiplus::Bitmap*	Tex;	// GDI+ テクスチャ
#else
	cv::Mat		Tex;			// todo 複数のモデル/グループで使いまわしたいのでハンドル管理したほうがいいだろう
#endif
	texUv*		uv;				// UV座標
	pol_def*	poldef;			// モデルのポリゴンと同じポリゴン番号、頂点数。uvの何番が対応するか
								//  ポリゴン定義と合わせる
};


struct attrib_flat
{
	// ����̍\���̂��Ă���H�_�~�[������H
};


typedef enum group_attr_type_
{
	attr_flat = 0,
	attr_tex,
}group_attr_type;


typedef struct pol_group_
{
	group_attr_type	attr_type;
	int		n_member;
	int*	member;		// �����o�[�|���S��
	void*	attrib;		// attr_type�ɂ���ĈႤ
						// void* ���w���̂́A
						//	attrib_tex*		attr_tex;
						//	attrib_flat*	attr_flat;
						// ���Ȃ�
	jhl_rgb	color;		// �Ƃ肠�����A���ł��J���[
	//jhl_rgb	color;		// line color?
}pol_group;


class modelData {
public:
	static void dataDump(modelData&, bool detail = false);

public:
	std::string name;
	int			n_vert;		// ���_��
	jhl_xyz*	verts;		//  vert �͔z��̂���Ȃ̂����A����ł����炵��
	int			n_pol;		// �|���S����
	pol_def*	poldef;		// ���f���̌`��
	int			n_groups;
	pol_group*	group;
};



struct object {
	matHomo4		model_mat;
	modelData*		p_model;

	// todo �����Ƃ�������
	bool		attrib_override;
	jhl_rgb		force_color;	// diffuse,�f�o�b�O�p���H
	// jhl_rgb		xxxx_color;	// ����
	// �ȂǂȂ�..
};

// ------------------------------------------------------------

class jhl3Dlib
{

public:
	friend disp_base;
	static const char*		jhl3Dlib::get_draw_type_string();
	static En_draw_type		jhl3Dlib::get_draw_type();
#ifdef USE_GDIPLUS
	static Gdiplus::Bitmap*	tgtTex;
#else
	static cv::Mat*			tgtTex;
#endif


// settings
private:
	static En_draw_type		draw_type;

	static viewport_config	vp;
	static jhl_xy_i			display;	// �L�����o�X�T�C�Y

public:
	static void			setTgtObj( const object& tgt);

private:
	static modelData*	tgtMdl;
//	static int			setTgtMdl(modelData* t) { tgtMdl = t; };

#ifdef USE_GDIPLUS
	static disp_gdiplus*	painter;
#else
	static disp_ocv2*	painter;
#endif
//	static disp_base*	painter;		// アクセスできないとクラスが嫌んだら

public:
//	static void			set_painter(disp_base& p) { painter = &p; };
#ifdef USE_GDIPLUS
	static void			set_painter(disp_gdiplus& p) { painter = &p; };
#else
	static void			set_painter(disp_ocv2& p) { painter = &p; };
#endif


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

	static jhl_xyz	interpolate_line_to_non_persed(const int wari, const int all, const jhl_xyz& p0, const jhl_xyz& p1);
	static void		interpolate_line_to_non_persed_tex(const int point, const int line_len, const jhl_xyz & p0, const jhl_xyz & p1, const jhl_xyz & p0t, const jhl_xyz & p1t, jhl_xyz & dest_p, jhl_xyz & dest_t);
	static void		interpolate_line_to_non_persed_with_z_fill(const jhl_xyz& p0, const jhl_xyz& p1, int y_force);
	static void		interpolate_line_to_non_persed_with_z_fill_tex(const jhl_xyz& p0, const jhl_xyz& p1, int y_force, jhl_xyz& tex0, jhl_xyz& tex1);

	static float	check_side(jhl_xyz* verts);
#ifdef USE_GDIPLUS
	static bool		is_not_transparent(gdi_color3b *p);
#else
	static bool		is_not_transparent(cv::Vec3b *p);
#endif

private:
	static void		setTransMat(const matHomo4& mdl_mat);
	static int		draw_a_polygon_flat(jhl_xyz ** rds, texUv ** texuv_sorted);
	static int		draw_a_polygon_tex(jhl_xyz ** rds, texUv ** texuv_sorted);
	
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
	static void		set_draw_type(En_draw_type type)
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
    


