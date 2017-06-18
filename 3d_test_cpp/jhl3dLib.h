#pragma once
#include <cmath>
#include <string>

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
	drawType_tex,
	/* 将来実装
		drawType_phong,
*/
drawType_max_,
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
	//	cv::Mat*	Tex;			// ocv 依存に..
	CvMat*		Tex;			// ocv 依存に..
	texUv*		uv;				// UV座標
	pol_def*	poldef;			// モデルのポリゴンと同じポリゴン番号、頂点順。uvの何番が対応するか

	std::string	texName2;
	cv::Mat*	Tex2;
	texUv*		uv2;
	pol_def*	poldef2;
};


struct attrib_flat
{
	int		n_member;
	int*    member;			// 属するポリゴン番号

	jhl_rgb	color;
};


#if 0
struct attrib_line_color : attrib_base
{
	int		n_member;
	int*    member;			// 属する頂点番号

	jhl_rgb	color;
};
#endif


class modelData {
public:
	static void dataDump(modelData&, bool detail = false);

public:
	std::string name;
	int			n_vert;		// 頂点数
	jhl_xyz*	verts;		//  vert は配列のつもりなのだが、これでいいらしい
	int			n_pol;		// ポリゴン数
	pol_def*	poldef;			
	int			n_attr_flat;		// 属性数
	int			n_attr_tex;		// 属性数
	attrib_tex*	attr_tex;
	attrib_flat*	attr_flat;
};



struct object {
	matHomo4		model_mat;
	modelData*		p_model;

	// todo もっといい実装
	bool		attrib_override;
	jhl_rgb		color;	// diffuse
	// jhl_rgb		color;	// 発光
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
	static jhl_xy_i			display;	// キャンバスサイズ

public:
	static void			setTgtObj( const object& tgt);

private:
	static modelData*	tgtMdl;
//	static int			setTgtMdl(modelData* t) { tgtMdl = t; };

	static disp_ocv2*	painter;
//	static disp_base*	painter;		// アクセスできない基底クラスうんたら

public:
//	static void			set_painter(disp_base& p) { painter = &p; };
	static void			set_painter(disp_ocv2& p) { painter = &p; };


// 変換行列関係
private:
	static matHomo4			view_mat;	//	ビュー変換(モデル変換は tgtMdl 持ち)
	static matHomo4_full	proj_mat;	//	投影変換
#ifdef DISP_MAT_KAKIKUDASHI
	static matHomo4			disp_mat;	//	ディスプレイ変換
#endif
	
	static matHomo4			modelViewMat;	// モデルビュー変換行列（光源計算に使う）
	static matHomo4_full	transMat;	// 最終的な変換行列

public:
	static void		set_view_mat(const jhl_xyz& eye_loc, const jhl_xyz& u_vec, const jhl_xyz& tgt_loc);
	static void		set_proj_mat(viewport_config& m_, bool ortho = false);
	static void		set_disp_trans(const jhl_xy_i& window);

//	static jhl_xyz	projback_disp_to_normal_box(float wari, jhl_xyz& p0, jhl_xyz& p1);
	static jhl_xyz	projback_disp_to_normal_box(float wari, float all, const jhl_xyz& p0, const jhl_xyz& p1);
	static void		projback_disp_to_normal_box_line(const jhl_xyz& p0, const jhl_xyz& p1, int y_force);

	static float	check_side(jhl_xyz* verts);

private:
	static void		setTransMat(const matHomo4& mdl_mat);


// 照明
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


// 描画
	static int draw(const object& mdl);

	static En_draw_type draw_type_next();
	static void		set_draw_type(En_draw_type type)
	{
		draw_type = type;
	}

private:
	static jhl_xyz	transToDisp(int vert_idx);
	static jhl_xyz* p_TTDcache;		// 頂点情報のディスプレ座標への変換結果をキャッシュする
	static int		TTDcacheSize;
	static	char*	mdl_name;

public:
	static int	transToDisp_cache_init(int cacheSize);
	static void	transToDisp_cache_clear();
	static void	transToDisp_cache_deinit();

};
    


