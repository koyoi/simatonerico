#include "stdafx.h"

#include "jhl3dLib.h"
#include "readData.h"

/*
メモ
https://www.ogis-ri.co.jp/otc/hiroba/technical/CppDesignNote/


*/

//-----------------------------------------------------------

En_draw_type	jhl3Dlib::draw_type = drawType_flat;
modelData*		jhl3Dlib::tgtMdl;

jhl_rgb*		jhl3Dlib::light_ambient;
dir_light*		jhl3Dlib::light_directional;
int				jhl3Dlib::num_light_dir;
jhl_rgb			jhl3Dlib::light_calced;		// 有効な全光源と、面の傾きを考えたもの。

jhl_xy_i		jhl3Dlib::display;
viewport_config	jhl3Dlib::vp;

matHomo4		jhl3Dlib::view_mat;		//	ビュー変換(モデル変換は tgtMdl 持ち)
matHomo4_full	jhl3Dlib::proj_mat;		//	投影変換
#ifdef DISP_MAT_KAKIKUDASHI
matHomo4		jhl3Dlib::disp_mat;		//	ディスプレイ変換
#endif

matHomo4 		jhl3Dlib::modelViewMat;	// モデルビュー変換
matHomo4_full	jhl3Dlib::transMat;		// モデルビュー変換からディスプレイ変換まで含む投影行列（一般名？）

disp_ocv2*		jhl3Dlib::painter;


// ディスプレイ座標への変換に使用(キャッシュなど)
jhl_xyz*		jhl3Dlib::p_TTDcache;
int				jhl3Dlib::TTDcacheSize;
char*			jhl3Dlib::mdl_name;

int	cache_miss;
int	cache_total;


// toria zチェック
static void min_max_update(float& tgt);
static void min_max_clear();
float z_min;
float z_max;


//-----------------------------------------------------------
inline static void swap(int& a, int& b)
{
	int t = a;
	a = b;
	b = t;
}

static float grad(jhl_xyz& p0, jhl_xyz& p1)
{
	if ((p1.y - p0.y) == 0)
	{
		// 分母ゼロ
		std::cout << "div by zero!" << std::endl;
		return((p1.x - p0.x) / (0.00001));	// todo toria
	}
	else
	{
		return((p1.x - p0.x) / (p1.y - p0.y));
	}
}

static void sort_y( jhl_xyz* points, int* y_sort )
{
	if (points[y_sort[1]].y > points[y_sort[2]].y)
	{
		swap(y_sort[1], y_sort[2]);		// y_sort[] の中身は都度初期化しないでいいのだ
	}
	if (points[y_sort[0]].y > points[y_sort[1]].y)
	{
		swap(y_sort[0], y_sort[1]);
	}
	if (points[y_sort[1]].y > points[y_sort[2]].y)
	{
		swap(y_sort[1], y_sort[2]);
	}
//	std::cout << points[0] << " , " << points[1] << " , " << points[2] << std::endl;
//	std::cout << y_sort[0] << " , " << y_sort[1] << " , " << y_sort[2] << std::endl;

}


//-----------------------------------------------------------

// 表裏判定
//	渡すのははディスプレイ座標
// カメラの方を向いていると正。
float jhl3Dlib::check_side(jhl_xyz* verts)
{
	// ポリゴンの頂点 verts[3] の作る面ベクトル v1=p1-p0, v2=p2-p1 の法線ベクトルが、
	// 視線(＝カメラ変換後なので、[0,0,-1]) と向き合うか？
	// 最適化済み結果

	// jhl_xyz v1 = verts[1] - verts[0];
	// jhl_xyz v2 = verts[2] - verts[1];
	float v1_x = verts[1].x - verts[0].x;
	float v1_y = verts[1].y - verts[0].y;
	float v2_x = verts[2].x - verts[1].x;
	float v2_y = verts[2].y - verts[1].y;

	return(-(v1_y*v2_x - v1_x*v2_y));
}



// あたる光の色を計算
// 	set_tgt_pObj(tgtMdl->verts);	// 後々の計算をモデル.ポリゴン番号 だけで引けるように
// で対象のモデルがセット済みであること。todo 確認出来るように
// todo 座標系　透視変換前に計算しなくてはならないのでは？
// 環境光＋平行光(面との内積を取る)。返値と、面の色を掛けてつかう
//jhl_rgb jhl3Dlib::calc_lighting(pol_def* pol)
void jhl3Dlib::calc_lighting(pol_def* pol)
{
	jhl_xyz p[3];

//	jhl_rgb rv = *light_ambient;
	light_calced = *light_ambient;

	// 注目のポリゴンの法線
	jhl_xyz v1 = tgtMdl->verts[ pol->b ] - tgtMdl->verts[ pol->a ];
	jhl_xyz v2 = tgtMdl->verts[ pol->c ] - tgtMdl->verts[ pol->b ];
	
	jhl_xyz n = modelViewMat * (v1 * v2);	// ベクトル積
	n = n.normalize();

	for (int i = 0; i < num_light_dir; i++)
	{
		float coeff = -n.dot( light_directional[i].dir);	// 当然、裏からの投影は無視
		if (coeff > 0)
		{
			light_calced += light_directional[i].col * coeff;
		}
	}
	
	// todo toriaezu １でクリップ
	if( light_calced.r > 1.0f ){  light_calced.r = 1.0f; }
	if( light_calced.g > 1.0f ){  light_calced.g = 1.0f; }
	if( light_calced.b > 1.0f ){  light_calced.b = 1.0f; }
//	return (light_calced);
}


void jhl3Dlib::setTgtObj(const object & tgt)
{
	tgtMdl = tgt.p_model;		// setTgt() みたいにした方が明示的に分かる？　別名つけただけでしかないけど...
	setTransMat(tgt.model_mat);	// モデルビュー変換～ディスプレイ座標変換まで一気にセット

	transToDisp_cache_clear();
}


// ビューマトリクス生成
// t : 視線方向の逆ベクトル
// r : 視線をｚ方向としたときの、x方向に当たるベクトル
// s : 同、y方向
void jhl3Dlib::set_view_mat(const jhl_xyz& eye_loc, const jhl_xyz& u_vec, const jhl_xyz& tgt_loc)
{
	jhl_xyz t, r, s;

	t = eye_loc - tgt_loc;
	r = u_vec * t;
	s = t * r;

	view_mat = matHomo4(r / r.norm(), s / s.norm(), t / t.norm(), -eye_loc);
}


// ディスプレイ座標へ変換
void jhl3Dlib::set_disp_trans(const jhl_xy_i& window_)
{
/*[ x/2   0   0  x/2 ]
  [  0   y/2  0  y/2 ]
  [  0    0   1   0  ]
  [  0    0   0   1  ]
*/
#ifdef DISP_MAT_KAKIKUDASHI
	disp_mat = matHomo4();
	disp_mat.m[0 * 4 + 0] = (float)window_.x / 2;
	disp_mat.m[1 * 4 + 1] = (float)window_.y / 2;
	disp_mat.m[0 * 4 + 3] = (float)window_.x / 2;
	disp_mat.m[1 * 4 + 3] = (float)window_.y / 2;
	disp_mat.m[2 * 4 + 2] = 1.0f;
	disp_mat.m[3 * 4 + 3] = 1.0f;
#endif
	display = window_ /2;
}


#if 1	// 上下左右対称バージョン
void jhl3Dlib::set_proj_mat(viewport_config & m_, bool ortho)
{
	vp = m_;
	matHomo4_full mp;

	int	rl, tb, fn;

	rl = vp.right - vp.left;
	tb = vp.top - vp.btm;
	fn = vp.vp_far - vp.vp_near;

	if (ortho) {
		// 等角
		mp.m[0 * 4 + 0] = (float)2 / rl;
		mp.m[1 * 4 + 1] = (float)2 / tb;
		mp.m[2 * 4 + 2] = (float)-2 / fn;
		mp.m[0 * 4 + 3] = (float)0;	// -(vp.right + vp.left) / rl;
		mp.m[1 * 4 + 3] = (float)0;	// -(vp.top + vp.btm) / tb;
		mp.m[2 * 4 + 3] = (float)-(vp.vp_far + vp.vp_near) / fn;
		mp.m[3 * 4 + 3] = 1;
	}
	else
	{
		// パースあり
		mp.m[0 * 4 + 0] = (float)2 * vp.vp_near / rl;
		mp.m[1 * 4 + 1] = (float)2 * vp.vp_near / tb;
		mp.m[0 * 4 + 2] = (float)0;	// (vp.right + vp.left) / rl;
		mp.m[1 * 4 + 2] = (float)0; // (vp.top + vp.btm) / tb;
		mp.m[2 * 4 + 2] = (float)-(vp.vp_far + vp.vp_near) / fn;
		mp.m[3 * 4 + 2] = (float)-1;
		mp.m[2 * 4 + 3] = (float)-2 * vp.vp_far * vp.vp_near / fn;
		mp.m[3 * 4 + 3] = (float)0;
	}
	proj_mat = mp;
}

#else	// 視台形が上下/左右対称でない場合。（どう使うの？ 3D視？）
void jhl3Dlib::set_proj_mat_norm(viewport_config & m_)
{
	vp = m_;
	matHomo4_full mp;

	int	rl, tb, fn;

	rl = vp.right - vp.left;
	tb = vp.top - vp.btm;
	fn = vp.vp_far - vp.vp_near;
	mp.m[0 * 4 + 0] = (float)2 * vp.vp_near / rl;
	mp.m[1 * 4 + 1] = (float)2 * vp.vp_near / tb;
	mp.m[0 * 4 + 2] = (float)(vp.right + vp.left) / rl;
	mp.m[1 * 4 + 2] = (float)(vp.top + vp.btm) / tb;
	mp.m[2 * 4 + 2] = (float)-(vp.vp_far + vp.vp_near) / fn;
	mp.m[3 * 4 + 2] = (float)-1;
	mp.m[2 * 4 + 3] = (float)-2 * vp.vp_far * vp.vp_near / fn;
	mp.m[3 * 4 + 3] = (float)0;
	proj_mat = mp;
}


void jhl3Dlib::set_proj_mat_ortho(viewport_config & m_)
{
	vp = m_;
	matHomo4_full mp;

	int	rl, tb, fn;

	rl = vp.right - vp.left;
	tb = vp.top - vp.btm;
	fn = vp.vp_far - vp.vp_near;
	mp.m[0 * 4 + 0] = (float)2 / rl;
	mp.m[1 * 4 + 1] = (float)2 / tb;
	mp.m[2 * 4 + 2] = (float)-2 / fn;
	mp.m[0 * 4 + 3] = (float)-(vp.right + vp.left) / rl;
	mp.m[1 * 4 + 3] = (float)-(vp.top + vp.btm) / tb;
	mp.m[2 * 4 + 3] = (float)-(vp.vp_far + vp.vp_near) / fn;

	mp.m[3 * 4 + 3] = 1;
	proj_mat = mp;
}
#endif


// デバイス座標まで変換する行列を求めてセット
// ゼロになるのとか、最適化する。
// …けど、モデル毎に1回だから、大してありがたくない気もする
/*
maximaコード
point1:matrix([x,y,z,1]);
trans_rot1:matrix([tr00,tr01,tr02,0],[tr10,tr11,tr12,0],[tr20,tr21,tr22,0],[0,0,0,1]);
trans1:matrix([1,0,0,tx1],[0,1,0,ty1],[0,0,1,yz1],[0,0,0,1]);
trans:trans1.trans_rot1;
mview:matrix([rx,sx,tx,ex],[ry,sy,ty,ey],[rz,sz,tz,ez],[0,0,0,1]);
mat_0:mview.trans;
mat_model_view:matrix([m0_00,m0_01,m0_02,m0_03],[m0_10,m0_11,m0_12,m0_13],[m0_20,m0_21,m0_22,m0_23],[0,0,0,1]);
mproj:matrix([m00,0,0,0],[0,m11,0,0],[0,0,m22,m23],[0,0,m32,m33]);
mdisp:matrix([wx,0,0,wx],[0,wy,0,wy],[0,0,1,0],[0,0,0,1]);
mat_1:mdisp.mproj;
mat_disp_proj:matrix([mt00,0,mt02,mt03],[0,mt11,mt12,mt13],[0,0,mt22,mt23],[0,0,mt32,mt33]);
mat:mat_disp_proj.mat_model_view;
result:mat.point1;
*/
void jhl3Dlib::setTransMat(const matHomo4 & mdl_mat)
{
	modelViewMat = view_mat*mdl_mat;	// mview x trans モデルビュー変換

#ifdef DISP_MAT_KAKIKUDASHI
	matHomo4_full m_proj_disp = proj_mat * disp_mat;
//	std::cout << "a:" << std::endl << m_proj_disp << std::endl;
#else	// 書き下し
	matHomo4_full	m_proj_disp = proj_mat;	// todo 実はゼロ多い... けど、単純全コピーで十分か？
	m_proj_disp.m[0 * 4 + 0] *= display.x;
	m_proj_disp.m[1 * 4 + 1] *= display.y;
	m_proj_disp.m[0 * 4 + 2] = m_proj_disp.m[3 * 4 + 2] * display.x;
	m_proj_disp.m[0 * 4 + 3] = m_proj_disp.m[3 * 4 + 3] * display.x;
	m_proj_disp.m[1 * 4 + 2] = m_proj_disp.m[3 * 4 + 2] * display.y;
	m_proj_disp.m[1 * 4 + 3] = m_proj_disp.m[3 * 4 + 3] * display.y;

//	std::cout << "b:" << std::endl << m_proj_disp << std::endl;
#endif

#if 0
	 transMat = m_proj_disp * modelViewMat;
	 // 
#else	// 書き下し
/* maxima code4
#m: modelViewMat
#mt : m_proj_disp
point1:matrix([x,y,z,1]);
mat_model_view:matrix([m_00,m_01,m_02,v_0],[m_10,m_11,m_12,v_1],[m_20,m_21,m_22,v_2],[0,0,0,1]);
mat_disp_proj:matrix([mt00,0,mt02,mt03],[0,mt11,mt12,mt13],[0,0,mt22,mt23],[0,0,mt32,mt33]);
mat:mat_disp_proj.mat_model_view;
result:mat.point1;
*/
	transMat.m[0 * 4 + 0] = modelViewMat.m[2 * 3 + 0] * m_proj_disp.m[0 * 4 + 2] + modelViewMat.m[0 * 3 + 0] * m_proj_disp.m[0 * 4 + 0];
	transMat.m[0 * 4 + 1] = modelViewMat.m[2 * 3 + 1] * m_proj_disp.m[0 * 4 + 2] + modelViewMat.m[0 * 3 + 1] * m_proj_disp.m[0 * 4 + 0];
	transMat.m[0 * 4 + 2] = modelViewMat.m[2 * 3 + 2] * m_proj_disp.m[0 * 4 + 2] + modelViewMat.m[0 * 3 + 2] * m_proj_disp.m[0 * 4 + 0];
	transMat.m[0 * 4 + 3] = m_proj_disp.m[0 * 4 + 2] * modelViewMat.v[2] + m_proj_disp.m[0 * 4 + 0] * modelViewMat.v[0] + m_proj_disp.m[0 * 4 + 3];
	transMat.m[1 * 4 + 0] = modelViewMat.m[2 * 3 + 0] * m_proj_disp.m[1 * 4 + 2] + modelViewMat.m[1 * 3 + 0] * m_proj_disp.m[1 * 4 + 1];
	transMat.m[1 * 4 + 1] = modelViewMat.m[2 * 3 + 1] * m_proj_disp.m[1 * 4 + 2] + modelViewMat.m[1 * 3 + 1] * m_proj_disp.m[1 * 4 + 1];
	transMat.m[1 * 4 + 2] = modelViewMat.m[2 * 3 + 2] * m_proj_disp.m[1 * 4 + 2] + modelViewMat.m[1 * 3 + 2] * m_proj_disp.m[1 * 4 + 1];
	transMat.m[1 * 4 + 3] = m_proj_disp.m[1 * 4 + 2] * modelViewMat.v[2] + m_proj_disp.m[1 * 4 + 1] * modelViewMat.v[1] + m_proj_disp.m[1 * 4 + 3];
	transMat.m[2 * 4 + 0] = modelViewMat.m[2 * 3 + 0] * m_proj_disp.m[2 * 4 + 2];
	transMat.m[2 * 4 + 1] = modelViewMat.m[2 * 3 + 1] * m_proj_disp.m[2 * 4 + 2];
	transMat.m[2 * 4 + 2] = modelViewMat.m[2 * 3 + 2] * m_proj_disp.m[2 * 4 + 2];
	transMat.m[2 * 4 + 3] = m_proj_disp.m[2 * 4 + 2] * modelViewMat.v[2] + m_proj_disp.m[2 * 4 + 3];
	transMat.m[3 * 4 + 0] = modelViewMat.m[2 * 3 + 0] * m_proj_disp.m[3 * 4 + 2];
	transMat.m[3 * 4 + 1] = modelViewMat.m[2 * 3 + 1] * m_proj_disp.m[3 * 4 + 2];
	transMat.m[3 * 4 + 2] = modelViewMat.m[2 * 3 + 2] * m_proj_disp.m[3 * 4 + 2];
	transMat.m[3 * 4 + 3] = m_proj_disp.m[3 * 4 + 2] * modelViewMat.v[2] + m_proj_disp.m[3 * 4 + 3]; 
#endif
/*
	std::cout << "model mat" << std::endl;
	std::cout << mdl_mat << std::endl;
	
	std::cout << "view mat" << std::endl;
	std::cout << view_mat << std::endl;

 	std::cout << "proj mat" << std::endl;
 	std::cout << proj_mat << std::endl;

 	std::cout << "m_proj_disp" << std::endl;
 	std::cout << m_proj_disp << std::endl;

 	std::cout << "modelViewMat (=view_mat * mdl_mat)" << std::endl;
 	std::cout << modelViewMat << std::endl;

 	std::cout << "transMat" << std::endl;
 	std::cout << transMat << std::endl;
//*/
}


// スクリーン座標上 p0,p1 の wari 分割割合から、標準視台形内の座標を割り出す
//   パースが掛かる前の座標が取れる
jhl_xyz jhl3Dlib::projback_disp_to_normal_box(float point, float line_len, const jhl_xyz& p0, const jhl_xyz& p1)
{
	jhl_xyz rv;
	if (line_len == 0) // 点が重なっているとき、ゼロ除算になってしまう箇所があるので回避
	{
		// 視点から見て重なってる時、手前の点を返す
		// todo あってるかな？
		
		if( p0.z <= p1.z )	// p1のほうが手前
		{
			rv.z = p1.z;
			rv.x = (p1.x / p1.z) * rv.z;
			rv.y = (p1.y / p1.z) * rv.z;
		}
		else
		{
			rv.z = p0.z;
			rv.x = (p0.x / p0.z) * rv.z;
			rv.y = (p0.y / p0.z) * rv.z;
		}
	}
	else
	{
		float wari = point / line_len;

		rv.z = 1 / ((1 - wari) / p0.z + wari / p1.z);
		rv.x = ((p0.x / p0.z) * (1 - wari) + (p1.x / p1.z) * wari) * rv.z;
		rv.y = ((p0.y / p0.z) * (1 - wari) + (p1.y / p1.z) * wari) * rv.z;
	}
	return(rv);
}

// ポリゴンをyでスライスしたとき、あるyのx_start - x_end の、zを塗る
// todo テクスチャ
void jhl3Dlib::projback_disp_to_normal_box_line(const jhl_xyz& p0, const jhl_xyz& p1, int y_force)
{
//	toria: p0.x < p1.x で来る
//	ほんとは呼び出し元では無くてこちらでやった方がいい気がするんだけど
	// せっかく参照で渡してるので p0, p1 をうまく入れ替える方法...

	jhl_xyz rv;

	int x_all = p1.x - p0.x;

	if (x_all == 0)
	{
		// todo 1pixだけ塗る？
		return;
	}

	float wari;
	for (int x = 0; x <= x_all; x++)
	{
		wari = (float)x / x_all;
		rv.z = 1.0f / ((1.0f - wari) / p0.z + wari / p1.z);
		rv.x = ((p0.x / p0.z) * (1.0f - wari) + (p1.x / p1.z) * wari) * rv.z;
		rv.y = ((p0.y / p0.z) * (1.0f - wari) + (p1.y / p1.z) * wari) * rv.z;

		// todo z check
		// 今はここで塗る (1pixずつ)
		painter->point_z(jhl_xy_i(p0.x + x, /*p0.y*/ y_force), 1.0-rv.z);	// todo 確認、zは逆数
//		painter->point_z(jhl_xy_i(p0.x + x, p0.y), rv.z);

		if (y_force != (int)p0.y) {
			volatile int p = 1;	// 誤差蓄積などで y がずれる事があるか？確認
			// ブレークを置くための無意味コード
		}

		// todo texture fetch
	}

	return;
}

#if 0


// 透視深度、使わない？
// Zやテクスチャ座標を計算するのに使う
jhl_xyz jhl3Dlib::_toushi_shindo(points, vp_far, vp_near)
{
	//def pers_r(p, vp_far, vp_near) :
	dest = []
		for p in points :
	x = -vp_near / p[2] * p[0]
		y = -vp_near / p[2] * p[1]
		z = -vp_far*vp_near / p[2] - (vp_far + vp_near) / 2
		z = z*(-2 / (vp_far - vp_near))
		dest.append((x, y, z))
		return dest
}


#endif


int jhl3Dlib::draw(const object& mdl)
{
	int rv = 0;	//	適当
	jhl_xyz	t_vert_disp[3];			// ディスプレイ座標へ変換後の座標（ただし、まだfloat,z(zは正規化状態)も持ってる）Temp(Vertex)_Display_space

	jhl3Dlib::setTgtObj( mdl );

	// 色設定 toria
	if (mdl.attrib_override)
	{
		painter->set_lineColor(mdl.color);	// シーンのモデル設定に設定してある属性
		painter->set_fillColor(mdl.color);
	}
	else
	{
		painter->set_lineColor(tgtMdl->attr_flat[0].color);	// 読み込んだモデル定義に定義された色 todo fillcolor
		painter->set_fillColor(tgtMdl->attr_flat[0].color);	// todo fillcolor
	}

	pol_def* t_poldef;

	switch ( jhl3Dlib::draw_type )
	{
	case drawType_vertex:
		for (int i = 0; i < tgtMdl->n_vert; i++)
		{
			jhl_xy_i _p = transToDisp(i);
			painter->point( _p );	// 頂点番号でよい
//			std::cout << tgtMdl->vert[i] << " -> " << t_vert[0];
		}
		break;

	case drawType_line:				// ワイヤフレーム（裏面消去なし)
	case drawType_line_front_face:	// ワイヤフレーム（裏面消去あり)
		for (int i = 0; i < tgtMdl->n_pol; i++)
		{
			t_poldef = &tgtMdl->poldef[i];

			t_vert_disp[0] = transToDisp( t_poldef->a );
			t_vert_disp[1] = transToDisp( t_poldef->b );
			t_vert_disp[2] = transToDisp( t_poldef->c );

			if( draw_type != drawType_line )	// 隠面消去有効無効分岐はここ
			{
				if ( check_side(t_vert_disp) < 0)
				{
					// 裏面。スキップ
					continue;
				}
			}
			painter->triangle(t_vert_disp);
		}
	break;

	case drawType_flat:				// 単色ポリゴン（光源無視、暗黙に裏面除外）使い道あるんか
	case drawType_flat_lighting:	// 単色ポリゴン（光源あり、暗黙に裏面除外）
		{
			int	y_sort[3] = { 0,1,2 };

			for (int i = 0; i < tgtMdl->n_pol; i++)
			{
				t_poldef = &tgtMdl->poldef[i];

				t_vert_disp[0] = transToDisp(t_poldef->a);	// todo 高速化：　裏面スキップを先にできる
				t_vert_disp[1] = transToDisp(t_poldef->b);
				t_vert_disp[2] = transToDisp(t_poldef->c);

				// 裏面ならスキップ
				if (check_side(t_vert_disp) < 0)
				{
					continue;
				}

				if (draw_type == drawType_flat_lighting)
				{
					// 面の色（フラットシェーディング、100%乱反射(面と視線の角度を考えない)）
					calc_lighting(t_poldef);	// あたってる光の計算のみ。これに色を掛けるのだ
					// memo なるほど、シェーダーが欲しくなるね！
				}
				else
				{
					light_calced = 1.0f;	// １固定
				}

				if (mdl.attrib_override)
				{
					painter->set_fillColor(mdl.color * light_calced);
				}
				else
				{
					painter->set_fillColor(tgtMdl->attr_flat[0].color * light_calced);
				}

				// 画面上、上から下に、左から右に塗っていきたい
				sort_y(t_vert_disp, y_sort);	// arg0:対象のポリゴン arg1:ソート結果（順番）

				// 2) y最小 から2つに点へ引く直線の式...　line()内で実装されてるんだろうけど
				//    yの画面上の方から走査
				{
					//				std::cout << "sorted vetrexes" << std::endl;
					//				std::cout << t_vert_disp[y_sort[0]] << " - " << t_vert_disp[y_sort[1]] << " , " << t_vert_disp[y_sort[2]] << std::endl;
					jhl_xyz rds[3] = { t_vert_disp[y_sort[0]], t_vert_disp[y_sort[1]],t_vert_disp[y_sort[2]] };	//(vec)R_Temp_Sorted

					float delta_x01 = grad(rds[0], rds[1]);
					float delta_x02 = grad(rds[0], rds[2]);
					float delta_x12 = grad(rds[1], rds[2]);
					float temp_x01 = rds[0].x;
					float temp_x02 = rds[0].x;
					float temp_x12 = rds[1].x;

					// todo 最初のラインが欠けるかも

					for (int y = (int)rds[0].y; y < (int)rds[1].y; y++)
					{
						//					std::cout << "y00: " << y << ", x : " << temp_x01 << " - " << temp_x02 << std::endl;
						painter->line_h(y, temp_x01, temp_x02);

						temp_x01 += delta_x01;
						temp_x02 += delta_x02;
					}

					for (int y = rds[1].y; y < rds[2].y - 1; y++)
					{
						//					std::cout << "y10: " << y << ", x : " << temp_x01 << " - " << temp_x12 << std::endl;
						painter->line_h(y, temp_x12, temp_x02);

						temp_x12 += delta_x12;
						temp_x02 += delta_x02;
					}

					// todo 最後のラインが欠けてる(上でループの最後が -1 。 はみ出し防止の安易な策)
				}
			}
		}
		break;

	case drawType_flat_z:				// 単色ポリゴン、Z計算あり（Z比較はまだ無し）（光源あり、暗黙に裏面除外）;
	{
		int	y_sort[3] = { 0,1,2 };
		min_max_clear();

		for (int i = 0; i < tgtMdl->n_pol; i++)
		{
			t_poldef = &tgtMdl->poldef[i];

			t_vert_disp[0] = transToDisp(t_poldef->a);
			t_vert_disp[1] = transToDisp(t_poldef->b);
			t_vert_disp[2] = transToDisp(t_poldef->c);

			// 裏面ならスキップ
			if (check_side(t_vert_disp) < 0)
			{
				continue;
			}

			// あたってる光の色の計算（フラットシェーディング、100%乱反射(面と視線の角度を考えない)）
			calc_lighting(t_poldef);	// これに色を掛けるのだ

										// 色設定 toria
			if (mdl.attrib_override)
			{
				painter->set_fillColor(mdl.color * light_calced);
			}
			else
			{
				painter->set_fillColor(tgtMdl->attr_flat[0].color * light_calced);
			}

			// debug z map 見やすくするためだけのもの
			min_max_update(t_vert_disp[0].z);			// 数字が大きい方が手前
			min_max_update(t_vert_disp[1].z);
			min_max_update(t_vert_disp[2].z);

			// ポリゴン頂点、yの小さい方からソート
			// yの上の方から描く（次に、左から右へ塗る）
			sort_y(t_vert_disp, y_sort);	// t_vert_disp:対象のポリゴン	y_sort:頂点ソート結果（順番）
			{
				//				std::cout << "sorted vetrexes" << std::endl;
				//				std::cout << t_vert_disp[y_sort[0]] << " - " << t_vert_disp[y_sort[1]] << " , " << t_vert_disp[y_sort[2]] << std::endl;
				jhl_xyz rds[3] = { t_vert_disp[y_sort[0]], t_vert_disp[y_sort[1]],t_vert_disp[y_sort[2]] };	//(vec)R_Temp_Sorted

				float delta_x01 = grad(rds[0], rds[1]);
				float delta_x02 = grad(rds[0], rds[2]);
				float delta_x12 = grad(rds[1], rds[2]);
				float temp_x01 = rds[0].x;
				float temp_x02 = rds[0].x;
				float temp_x12 = rds[1].x;

				// todo 最初のラインが欠けるかも
				jhl_xyz	scan_y[2];

				int	y_all01 = (int)(rds[1].y - rds[0].y);
				int	y_all02 = (int)(rds[2].y - rds[0].y);
				int y_start0 = (int)rds[0].y;

				for (int y = (int)rds[0].y; y < (int)rds[1].y; y++)
				{
					//					std::cout << "y00: " << y << ", x : " << temp_x01 << " - " << temp_x02 << std::endl;
					//					painter->point(jhl_xy_i(temp_x01, y), 0);
					//					painter->point(jhl_xy_i(temp_x02, y), 0);
					painter->line_h(y, temp_x01, temp_x02);	// 色は一気に塗ってしまう

					// z を塗る
					// △のｙの小さい方から水平に塗ってゆく
					// projback_disp_to_normal_box : 正規化視台形中（パースを殺した状態で）での座標
					// そこで、辺のどの辺（割合）か
					scan_y[0] = projback_disp_to_normal_box((y - y_start0), y_all01,
						rds[0], rds[1]);
					scan_y[1] = projback_disp_to_normal_box((y - y_start0), y_all02,
						rds[0], rds[2]);

#if 0 
					// 計算誤差でラインを飛ばしてしまう可能性があるので y を強制する
					// 確認コード
					y_[0] = p_wari[0].y;
					y_[1] = p_wari[1].y;

					if ((int)y_[0] != y || (int)y_[1] != y) {
						int i = 0;	// ブレークポイントを置くためのダミー
					}
#endif

					if (scan_y[0].x <= scan_y[1].x)
					{
						projback_disp_to_normal_box_line(scan_y[0], scan_y[1], y);
					}
					else
					{
						projback_disp_to_normal_box_line(scan_y[1], scan_y[0], y);
					}

					temp_x01 += delta_x01;
					temp_x02 += delta_x02;
				}

				int	y_all12 = (int)(rds[2].y - rds[1].y);
				int y_start1 = (int)rds[1].y;

				for (int y = rds[1].y; y < rds[2].y - 1; y++)
				{
					//					std::cout << "y10: " << y << ", x : " << temp_x01 << " - " << temp_x12 << std::endl;
					//					painter->point(jhl_xy_i(temp_x12, y), 0);
					//					painter->point(jhl_xy_i(temp_x02, y), 0);
					painter->line_h(y, temp_x12, temp_x02);

					// z を塗る
					scan_y[0] = projback_disp_to_normal_box((y - y_start1), y_all12,
						rds[1], rds[2]);
					scan_y[1] = projback_disp_to_normal_box((y - y_start0), y_all02,
						rds[0], rds[2]);
					if (scan_y[0].x <= scan_y[1].x)
					{
						projback_disp_to_normal_box_line(scan_y[0], scan_y[1], y);
					}
					else
					{
						projback_disp_to_normal_box_line(scan_y[1], scan_y[0], y);
					}

					temp_x12 += delta_x12;
					temp_x02 += delta_x02;
				}

				// todo 最後のラインが欠けてる(上でループの最後が -1 。 はみ出し防止の安易な策)
			}
		}
		std::cout << "zmin,max = " << z_min << ", " << z_max << std::endl;
		min_max_clear();
	}
	case drawType_tex:				// テクスチャ有り;
#if 0
	{
		int	y_sort[3] = { 0,1,2 };
		min_max_clear();

		for (int i = 0; i < tgtMdl->n_pol; i++)
		{
			t_poldef = &tgtMdl->poldef[i];

			t_vert_disp[0] = transToDisp(t_poldef->a);
			t_vert_disp[1] = transToDisp(t_poldef->b);
			t_vert_disp[2] = transToDisp(t_poldef->c);

			// todo 最初に変換してリスト、裏面のはリストに入れない

			// 裏面ならスキップ
			if (check_side(t_vert_disp) < 0)
			{
				continue;
			}

			// あたってる光の色の計算（フラットシェーディング、100%乱反射(面と視線の角度を考えない)）
			calc_lighting(t_poldef);	// これに色を掛けるのだ

										// 色設定 toria
			if (mdl.attrib_override)
			{
				painter->set_fillColor(mdl.color * light_calced);
			}
			else
			{
				painter->set_fillColor(tgtMdl->attr_flat[0].color * light_calced);
			}

			// debug z map 見やすくするためだけのもの
			min_max_update(t_vert_disp[0].z);
			min_max_update(t_vert_disp[1].z);
			min_max_update(t_vert_disp[2].z);

			// ポリゴン頂点、yの小さい方からソート
			// yの上の方から描く（次に、左から右へ塗る）
			sort_y(t_vert_disp, y_sort);	// t_vert_disp:対象のポリゴン	y_sort:頂点ソート結果（順番）
			{
				//				std::cout << "sorted vetrexes" << std::endl;
				//				std::cout << t_vert_disp[y_sort[0]] << " - " << t_vert_disp[y_sort[1]] << " , " << t_vert_disp[y_sort[2]] << std::endl;
				jhl_xyz rds[3] = { t_vert_disp[y_sort[0]], t_vert_disp[y_sort[1]],t_vert_disp[y_sort[2]] };	//(vec)R_Temp_Sorted

				float delta_x01 = grad(rds[0], rds[1]);
				float delta_x02 = grad(rds[0], rds[2]);
				float delta_x12 = grad(rds[1], rds[2]);
				float temp_x01 = rds[0].x;
				float temp_x02 = rds[0].x;
				float temp_x12 = rds[1].x;

				// todo 最初のラインが欠けるかも
				jhl_xyz	p_wari[2];

				int	y_all01 = (int)(rds[1].y - rds[0].y);
				int	y_all02 = (int)(rds[2].y - rds[0].y);
				int y_start0 = (int)rds[0].y;

				for (int y = (int)rds[0].y; y < (int)rds[1].y; y++)
				{
					//					std::cout << "y00: " << y << ", x : " << temp_x01 << " - " << temp_x02 << std::endl;
					//					painter->point(jhl_xy_i(temp_x01, y), 0);
					//					painter->point(jhl_xy_i(temp_x02, y), 0);
					painter->line_h(y, temp_x01, temp_x02);

					// z を塗る
					// △のｙの小さい方から水平に塗ってゆく
					// projback_disp_to_normal_box : 正規化視台形中（パースを殺した状態で）での座標
					// そこで、辺のどの辺（割合）か
					p_wari[0] = projback_disp_to_normal_box((y - y_start0), y_all01,
						rds[0], rds[1]);
					p_wari[1] = projback_disp_to_normal_box((y - y_start0), y_all02,
						rds[0], rds[2]);

#if 0 
					// 計算誤差でラインを飛ばしてしまう可能性があるので y を強制する
					// 確認コード
					y_[0] = p_wari[0].y;
					y_[1] = p_wari[1].y;

					if ((int)y_[0] != y || (int)y_[1] != y) {
						int i = 0;	// ブレークポイントを置くためのダミー
					}
#endif

					if (p_wari[0].x <= p_wari[1].x)
					{
						projback_disp_to_normal_box_line(p_wari[0], p_wari[1], y);
					}
					else
					{
						projback_disp_to_normal_box_line(p_wari[1], p_wari[0], y);
					}

					temp_x01 += delta_x01;
					temp_x02 += delta_x02;
				}

				int	y_all12 = (int)(rds[2].y - rds[1].y);
				int y_start1 = (int)rds[1].y;

				for (int y = rds[1].y; y < rds[2].y - 1; y++)
				{
					//					std::cout << "y10: " << y << ", x : " << temp_x01 << " - " << temp_x12 << std::endl;
					//					painter->point(jhl_xy_i(temp_x12, y), 0);
					//					painter->point(jhl_xy_i(temp_x02, y), 0);
					painter->line_h(y, temp_x12, temp_x02);

					// z を塗る
					p_wari[0] = projback_disp_to_normal_box((y - y_start1), y_all12,
						rds[1], rds[2]);
					p_wari[1] = projback_disp_to_normal_box((y - y_start0), y_all02,
						rds[0], rds[2]);
					if (p_wari[0].x <= p_wari[1].x)
					{
						projback_disp_to_normal_box_line(p_wari[0], p_wari[1], y);
					}
					else
					{
						projback_disp_to_normal_box_line(p_wari[1], p_wari[0], y);
					}

					temp_x12 += delta_x12;
					temp_x02 += delta_x02;
				}

				// todo 最後のラインが欠けてる(上でループの最後が -1 。 はみ出し防止の安易な策)
			}
		}
		std::cout << "zmin,max = " << z_min << ", " << z_max << std::endl;
		min_max_clear();
}
#endif

#if 0
	case zソート
#endif
		break;
#if 0
	case drawType_phong:				// フォンシェーディング
		各頂点の法線 ≡ その頂点を共有する全ポリゴンの法線の平均
		モデル読み込み時、またはアニメーション時にあらかじめ計算しておくべきだろう
			hwではどうしてるんだろう？
		グループごとにnの計算をする(ローカル座標で）
#endif
	default:
		break;
	}

	return rv;
}


En_draw_type jhl3Dlib::draw_type_next()
{
	if (draw_type < drawType_max_)
	{
		draw_type = (En_draw_type)(draw_type + 1);
	}
	else
	{
		draw_type = (En_draw_type)0;
	}
	std::cout << "draw type change to: " << draw_type << std::endl;
	return draw_type;
}


// もう一枚かぶせたい？
// todo 実は計算量少ない機が…特に最近のDSP命令あったら　オーバーヘッドのほうが大きいかも？
jhl_xyz jhl3Dlib::transToDisp(int vert_idx)
{
	if (p_TTDcache == NULL)
	{
		return(transMat * tgtMdl->verts[vert_idx]);	// キャッシュ未初期化 / 未使用
	}

	if( p_TTDcache[vert_idx] == jhl_xyz(0) )
	{
		p_TTDcache[vert_idx] = transMat * tgtMdl->verts[vert_idx];
		cache_miss++;
	}
	cache_total++;
	return(p_TTDcache[vert_idx]);
}


int jhl3Dlib::transToDisp_cache_init(int cacheSize)
{
	TTDcacheSize = cacheSize;
	p_TTDcache = new jhl_xyz[cacheSize]();
	if (p_TTDcache == NULL)
	{
		return 0;
	}
	else
	{
		transToDisp_cache_clear();
		return 1;
	}
}

void jhl3Dlib::transToDisp_cache_deinit()
{
	if (p_TTDcache != NULL)
	{
		delete[] p_TTDcache;
		p_TTDcache = NULL;
	}
}

void jhl3Dlib::transToDisp_cache_clear()
{
	if (p_TTDcache == NULL)
	{
		return;
	}

	// 配列だけど、アラインメントの都合でmemsetする前に実際にfillするサイズを算出
	// 無駄？（中でやってる？） // todo もっといい方法
	jhl_xyz* tbl2 = p_TTDcache;
	tbl2++;
	int stride = (int)tbl2 - (int)p_TTDcache;

	memset(p_TTDcache, 0, stride * TTDcacheSize);
	cache_total = 0;
	cache_miss = 0;
}



void modelData::dataDump(modelData& mdl, bool detail)
{
	using namespace std;

	cout << "vertex : " << mdl.n_vert << endl;
	if (detail) {
		jhl_xyz* t;
		for (int i = 0; i < mdl.n_vert; i++)
		{
			t = &mdl.verts[i];
			cout << i << ": ( " << t->x << ", " << t->y << ", " << t->z << " )" << endl;
		}
	}

	cout << "polygon : " << mdl.n_pol << endl;
	if (detail) {
		pol_def* t;
		for (int i = 0; i < mdl.n_pol; i++)
		{
			t = &mdl.poldef[i];
			cout << i << " : ( " << t->a << ", " << t->b << ", " << t->c << " )" << endl;
		}
	}

	cout << "attributes : " << endl;
	cout << "  attribute - flat" << endl;
	for (int i = 0; i < mdl.n_attr_flat; i++)
	{
		attrib_flat t = mdl.attr_flat[i];
		if (detail) {
			cout << " member : ";
			for (int j = 0; j < t.n_member; j++)
			{
				cout << t.member[j] << " ,";
			}
			cout << endl;
		}
		cout << " color : (" << t.color.r << ", " << t.color.g << ", " << t.color.b << ")" << endl;
	}

	cout << "  attribute - tex" << endl;
	for (int i = 0; i < mdl.n_attr_tex; i++)
	{
		attrib_tex t = mdl.attr_tex[i];
		if (detail) {
			cout << " member : ";
			if (t.texName != "")
			{
				cout << t.texName << endl;
			}
			if (t.texName2 != "")
			{
				cout << t.texName2 << endl;
			}
		}
	}
}

//toria
static void min_max_update(float& tgt)
{
	if (tgt< z_min) z_min = tgt;
	if (tgt> z_max) z_max = tgt;

}

static void min_max_clear()
{
	z_min = 1.0f;
	z_max = 0.0f;
}
