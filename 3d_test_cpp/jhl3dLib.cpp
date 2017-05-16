#include "stdafx.h"

#include "jhl3dLib.h"
#include "readData.h"

/*
����
https://www.ogis-ri.co.jp/otc/hiroba/technical/CppDesignNote/


*/

//-----------------------------------------------------------

En_draw_type	jhl3Dlib::draw_type = drawType_flat;
modelData*		jhl3Dlib::tgtMdl;

jhl_rgb*		jhl3Dlib::light_ambient;
dir_light*		jhl3Dlib::light_directional;
int				jhl3Dlib::num_light_dir;
jhl_rgb			jhl3Dlib::light_calced;		// �L���ȑS�����ƁA�ʂ̌X�����l�������́B

jhl_xy_i		jhl3Dlib::display;
viewport_config	jhl3Dlib::vp;

matHomo4		jhl3Dlib::view_mat;		//	�r���[�ϊ�(���f���ϊ��� tgtMdl ����)
matHomo4_full	jhl3Dlib::proj_mat;		//	���e�ϊ�
#ifdef DISP_MAT_KAKIKUDASHI
matHomo4		jhl3Dlib::disp_mat;		//	�f�B�X�v���C�ϊ�
#endif

matHomo4 		jhl3Dlib::modelViewMat;	// ���f���r���[�ϊ�
matHomo4_full	jhl3Dlib::transMat;		// ���f���r���[�ϊ�����f�B�X�v���C�ϊ��܂Ŋ܂ޓ��e�s��i��ʖ��H�j

disp_ocv2*		jhl3Dlib::painter;


// �f�B�X�v���C���W�ւ̕ϊ��Ɏg�p(�L���b�V���Ȃ�)
jhl_xyz*		jhl3Dlib::p_TTDcache;
int				jhl3Dlib::TTDcacheSize;
char*			jhl3Dlib::mdl_name;

int	cache_miss;
int	cache_total;


// toria z�`�F�b�N
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
		// ����[��
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
		swap(y_sort[1], y_sort[2]);		// y_sort[] �̒��g�͓s�x���������Ȃ��ł����̂�
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

// �\������
//	�n���̂͂̓f�B�X�v���C���W
// �J�����̕��������Ă���Ɛ��B
float jhl3Dlib::check_side(jhl_xyz* verts)
{
	// �|���S���̒��_ verts[3] �̍��ʃx�N�g�� v1=p1-p0, v2=p2-p1 �̖@���x�N�g�����A
	// ����(���J�����ϊ���Ȃ̂ŁA[0,0,-1]) �ƌ����������H
	// �œK���ς݌���

	// jhl_xyz v1 = verts[1] - verts[0];
	// jhl_xyz v2 = verts[2] - verts[1];
	float v1_x = verts[1].x - verts[0].x;
	float v1_y = verts[1].y - verts[0].y;
	float v2_x = verts[2].x - verts[1].x;
	float v2_y = verts[2].y - verts[1].y;

	return(-(v1_y*v2_x - v1_x*v2_y));
}



// ��������̐F���v�Z
// 	set_tgt_pObj(tgtMdl->verts);	// ��X�̌v�Z�����f��.�|���S���ԍ� �����ň�����悤��
// �őΏۂ̃��f�����Z�b�g�ς݂ł��邱�ƁBtodo �m�F�o����悤��
// todo ���W�n�@�����ϊ��O�Ɍv�Z���Ȃ��Ă͂Ȃ�Ȃ��̂ł́H
// �����{���s��(�ʂƂ̓��ς����)�B�Ԓl�ƁA�ʂ̐F���|���Ă���
//jhl_rgb jhl3Dlib::calc_lighting(pol_def* pol)
void jhl3Dlib::calc_lighting(pol_def* pol)
{
	jhl_xyz p[3];

//	jhl_rgb rv = *light_ambient;
	light_calced = *light_ambient;

	// ���ڂ̃|���S���̖@��
	jhl_xyz v1 = tgtMdl->verts[ pol->b ] - tgtMdl->verts[ pol->a ];
	jhl_xyz v2 = tgtMdl->verts[ pol->c ] - tgtMdl->verts[ pol->b ];
	
	jhl_xyz n = modelViewMat * (v1 * v2);	// �x�N�g����
	n = n.normalize();

	for (int i = 0; i < num_light_dir; i++)
	{
		float coeff = -n.dot( light_directional[i].dir);	// ���R�A������̓��e�͖���
		if (coeff > 0)
		{
			light_calced += light_directional[i].col * coeff;
		}
	}
	
	// todo toriaezu �P�ŃN���b�v
	if( light_calced.r > 1.0f ){  light_calced.r = 1.0f; }
	if( light_calced.g > 1.0f ){  light_calced.g = 1.0f; }
	if( light_calced.b > 1.0f ){  light_calced.b = 1.0f; }
//	return (light_calced);
}


void jhl3Dlib::setTgtObj(const object & tgt)
{
	tgtMdl = tgt.p_model;		// setTgt() �݂����ɂ������������I�ɕ�����H�@�ʖ����������ł����Ȃ�����...
	setTransMat(tgt.model_mat);	// ���f���r���[�ϊ��`�f�B�X�v���C���W�ϊ��܂ň�C�ɃZ�b�g

	transToDisp_cache_clear();
}


// �r���[�}�g���N�X����
// t : ���������̋t�x�N�g��
// r : �������������Ƃ����Ƃ��́Ax�����ɓ�����x�N�g��
// s : ���Ay����
void jhl3Dlib::set_view_mat(const jhl_xyz& eye_loc, const jhl_xyz& u_vec, const jhl_xyz& tgt_loc)
{
	jhl_xyz t, r, s;

	t = eye_loc - tgt_loc;
	r = u_vec * t;
	s = t * r;

	view_mat = matHomo4(r / r.norm(), s / s.norm(), t / t.norm(), -eye_loc);
}


// �f�B�X�v���C���W�֕ϊ�
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


#if 1	// �㉺���E�Ώ̃o�[�W����
void jhl3Dlib::set_proj_mat(viewport_config & m_, bool ortho)
{
	vp = m_;
	matHomo4_full mp;

	int	rl, tb, fn;

	rl = vp.right - vp.left;
	tb = vp.top - vp.btm;
	fn = vp.vp_far - vp.vp_near;

	if (ortho) {
		// ���p
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
		// �p�[�X����
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

#else	// ����`���㉺/���E�Ώ̂łȂ��ꍇ�B�i�ǂ��g���́H 3D���H�j
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


// �f�o�C�X���W�܂ŕϊ�����s������߂ăZ�b�g
// �[���ɂȂ�̂Ƃ��A�œK������B
// �c���ǁA���f������1�񂾂���A�債�Ă��肪�����Ȃ��C������
/*
maxima�R�[�h
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
	modelViewMat = view_mat*mdl_mat;	// mview x trans ���f���r���[�ϊ�

#ifdef DISP_MAT_KAKIKUDASHI
	matHomo4_full m_proj_disp = proj_mat * disp_mat;
//	std::cout << "a:" << std::endl << m_proj_disp << std::endl;
#else	// ��������
	matHomo4_full	m_proj_disp = proj_mat;	// todo ���̓[������... ���ǁA�P���S�R�s�[�ŏ\�����H
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
#else	// ��������
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


// �X�N���[�����W�� p0,p1 �� wari ������������A�W��������`���̍��W������o�� 
jhl_xyz jhl3Dlib::projback_disp_to_normal_box(float point, float all, jhl_xyz& p0, jhl_xyz& p1)
{
	jhl_xyz rv;
	if (all == 0)
	{
		// ���_���猩�ďd�Ȃ��Ă鎞�A��O�̓_��Ԃ�
		// todo �����Ă邩�ȁH
		
		if( p0.z <= p1.z )	// p1�̂ق�����O
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
		float wari = point / all;

		rv.z = 1 / ((1 - wari) / p0.z + wari / p1.z);
		rv.x = ((p0.x / p0.z) * (1 - wari) + (p1.x / p1.z) * wari) * rv.z;
		rv.y = ((p0.y / p0.z) * (1 - wari) + (p1.y / p1.z) * wari) * rv.z;
	}
	return(rv);
}

// z�o�b�t�@(��)���h��܂�
void jhl3Dlib::projback_disp_to_normal_box_line(jhl_xyz& p0, jhl_xyz& p1, int y_force)
{
//	toria: p0.x < p1.x �ŗ���
//	�ق�Ƃ͌Ăяo�����ł͖����Ă�����ł�������������C������񂾂���

	jhl_xyz rv;
	// toria

	int x_all = p1.x - p0.x;

	if (x_all == 0)
	{
		// todo 1pix�����h��H
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
		// ���͂����œh�� (1pix����)
		painter->point_z(jhl_xy_i(p0.x + x, /*p0.y*/ y_force), rv.z);
//		painter->point_z(jhl_xy_i(p0.x + x, p0.y), rv.z);

		if (y_force != (int)p0.y) {
			int p = 1;
		}

		// todo texture fetch
	}

	return;
}

#if 0


// �����[�x�A�g��Ȃ��H
// Z��e�N�X�`�����W���v�Z����̂Ɏg��
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
	int rv = 0;	//	�K��
	jhl_xyz	t_vert_disp[3];			// �f�B�X�v���C���W�֕ϊ���̍��W�i�������A�܂�float,z(z�͐��K�����)�������Ă�j

	jhl3Dlib::setTgtObj( mdl );

	// �F�ݒ� toria
	if (mdl.attrib_override)
	{
		painter->set_lineColor(mdl.color);	// �V�[���̃��f���ݒ�ɐݒ肵�Ă��鑮��
		painter->set_fillColor(mdl.color);
	}
	else
	{
		painter->set_lineColor(tgtMdl->attr[0].color);	// �ǂݍ��񂾃��f����`�ɒ�`���ꂽ�F todo fillcolor
		painter->set_fillColor(tgtMdl->attr[0].color);	// todo fillcolor
	}

	pol_def* t_poldef;

	switch ( jhl3Dlib::draw_type )
	{
	case drawType_vertex:

		for (int i = 0; i < tgtMdl->n_vert; i++)
		{
			jhl_xy_i _p = transToDisp(i);
			painter->point( _p );	// ���_�ԍ��ł悢
//			std::cout << tgtMdl->vert[i] << " -> " << t_vert[0];
		}
		break;

	case drawType_line:				// ���C���t���[���i���ʏ����Ȃ�)
	case drawType_line_front_face:	// ���C���t���[���i���ʏ�������)
		for (int i = 0; i < tgtMdl->n_pol; i++)
		{
			t_poldef = &tgtMdl->poldef[i];

			t_vert_disp[0] = transToDisp( t_poldef->a );
			t_vert_disp[1] = transToDisp( t_poldef->b );
			t_vert_disp[2] = transToDisp( t_poldef->c );

			if( draw_type != drawType_line )	// �B�ʏ����L����������͂���
			{
				if ( check_side(t_vert_disp) < 0)
				{
					// ���ʁB�X�L�b�v
					continue;
				}
			}
			painter->triangle(t_vert_disp);
		}
	break;

	case drawType_flat:				// �P�F�|���S���i���������A�Öقɗ��ʏ��O�j�g���������
	case drawType_flat_lighting:	// �P�F�|���S���i��������A�Öقɗ��ʏ��O�j
		{
			int	y_sort[3] = { 0,1,2 };

			for (int i = 0; i < tgtMdl->n_pol; i++)
			{
				t_poldef = &tgtMdl->poldef[i];

				t_vert_disp[0] = transToDisp(t_poldef->a);
				t_vert_disp[1] = transToDisp(t_poldef->b);
				t_vert_disp[2] = transToDisp(t_poldef->c);

				// ���ʂȂ�X�L�b�v
				if (check_side(t_vert_disp) < 0)
				{
					continue;
				}

				if (draw_type == drawType_flat_lighting)
				{
					// �ʂ̐F�i�t���b�g�V�F�[�f�B���O�A100%������(�ʂƎ����̊p�x���l���Ȃ�)�j
					calc_lighting(t_poldef);	// �������Ă���̌v�Z�̂݁B����ɐF���|����̂�
				}
				else
				{
					light_calced = 1.0f;
				}

				if (mdl.attrib_override)
				{
					painter->set_fillColor(mdl.color * light_calced);
				}
				else
				{
					painter->set_fillColor(tgtMdl->attr[0].color * light_calced);
				}

				// 1) �|���S�����_�̉�ʏ�y�\�[�g
				sort_y(t_vert_disp, y_sort);	// arg0:�Ώۂ̃|���S�� arg1:�\�[�g���ʁi���ԁj

				// 2) y�ŏ� ����2�ɓ_�ֈ��������̎�...�@line()���Ŏ�������Ă�񂾂낤����
				//    y�̉�ʏ�̕����瑖��
				{
					//				std::cout << "sorted vetrexes" << std::endl;
					//				std::cout << t_vert_disp[y_sort[0]] << " - " << t_vert_disp[y_sort[1]] << " , " << t_vert_disp[y_sort[2]] << std::endl;

					float delta_x01 = grad(t_vert_disp[y_sort[0]], t_vert_disp[y_sort[1]]);
					float delta_x02 = grad(t_vert_disp[y_sort[0]], t_vert_disp[y_sort[2]]);
					float delta_x12 = grad(t_vert_disp[y_sort[1]], t_vert_disp[y_sort[2]]);
					float temp_x01 = t_vert_disp[y_sort[0]].x;
					float temp_x02 = t_vert_disp[y_sort[0]].x;
					float temp_x12 = t_vert_disp[y_sort[1]].x;

					// todo �ŏ��̃��C���������邩��

					for (int y = (int)t_vert_disp[y_sort[0]].y; y < (int)t_vert_disp[y_sort[1]].y; y++)
					{
						//					std::cout << "y00: " << y << ", x : " << temp_x01 << " - " << temp_x02 << std::endl;
						painter->line_h(y, temp_x01, temp_x02);

						temp_x01 += delta_x01;
						temp_x02 += delta_x02;
					}

					for (int y = t_vert_disp[y_sort[1]].y; y < t_vert_disp[y_sort[2]].y - 1; y++)
					{
						//					std::cout << "y10: " << y << ", x : " << temp_x01 << " - " << temp_x12 << std::endl;
						painter->line_h(y, temp_x12, temp_x02);

						temp_x12 += delta_x12;
						temp_x02 += delta_x02;
					}

					// todo �Ō�̃��C���������Ă�(��Ń��[�v�̍Ōオ -1 �B �͂ݏo���h�~�̈��Ղȍ�)
				}
			}
		}

	case drawType_flat_z:				// �P�F�|���S���AZ����i��������A�Öقɗ��ʏ��O�j;
	{
		int	y_sort[3] = { 0,1,2 };
		min_max_clear();

		for (int i = 0; i < tgtMdl->n_pol; i++)
		{
			t_poldef = &tgtMdl->poldef[i];

			t_vert_disp[0] = transToDisp(t_poldef->a);
			t_vert_disp[1] = transToDisp(t_poldef->b);
			t_vert_disp[2] = transToDisp(t_poldef->c);

			// ���ʂȂ�X�L�b�v
			if (check_side(t_vert_disp) < 0)
			{
				continue;
			}

			// �ʂ̐F�i�t���b�g�V�F�[�f�B���O�A100%������(�ʂƎ����̊p�x���l���Ȃ�)�j
			calc_lighting(t_poldef);	// �������Ă���̌v�Z�̂݁B����ɐF���|����̂�

			// �F�ݒ� toria
			if (mdl.attrib_override)
			{
				painter->set_fillColor(mdl.color * light_calced);
			}
			else
			{
				painter->set_fillColor(tgtMdl->attr[0].color * light_calced);
			}

			// z map ���₷�����邽��
			min_max_update(t_vert_disp[0].z);
			min_max_update(t_vert_disp[1].z);
			min_max_update(t_vert_disp[2].z);

			// �|���S�����_�Ay�̏�����������\�[�g
			sort_y(t_vert_disp, y_sort);	// arg0:�Ώۂ̃|���S�� arg1:�\�[�g���ʁi���ԁj
			{
				//				std::cout << "sorted vetrexes" << std::endl;
				//				std::cout << t_vert_disp[y_sort[0]] << " - " << t_vert_disp[y_sort[1]] << " , " << t_vert_disp[y_sort[2]] << std::endl;

				float delta_x01 = grad(t_vert_disp[y_sort[0]], t_vert_disp[y_sort[1]]);
				float delta_x02 = grad(t_vert_disp[y_sort[0]], t_vert_disp[y_sort[2]]);
				float delta_x12 = grad(t_vert_disp[y_sort[1]], t_vert_disp[y_sort[2]]);
				float temp_x01 = t_vert_disp[y_sort[0]].x;
				float temp_x02 = t_vert_disp[y_sort[0]].x;
				float temp_x12 = t_vert_disp[y_sort[1]].x;

				// todo �ŏ��̃��C���������邩��
				jhl_xyz	p_wari[2];
				int y_hist = t_vert_disp[y_sort[0]].y;

//				float y_[3];

				int	y_all01 = (int)t_vert_disp[y_sort[1]].y - (int)t_vert_disp[y_sort[0]].y;
				int	y_all02 = (int)t_vert_disp[y_sort[2]].y - (int)t_vert_disp[y_sort[0]].y;
				int y_start0 = (int)t_vert_disp[y_sort[0]].y;

				for (int y = (int)t_vert_disp[y_sort[0]].y; y < (int)t_vert_disp[y_sort[1]].y; y++)
				{
					//					std::cout << "y00: " << y << ", x : " << temp_x01 << " - " << temp_x02 << std::endl;
					//					painter->point(jhl_xy_i(temp_x01, y), 0);
					//					painter->point(jhl_xy_i(temp_x02, y), 0);
					painter->line_h(y, temp_x01, temp_x02);

					// z ��h��
					p_wari[0] = projback_disp_to_normal_box((y - y_start0), y_all01,
						t_vert_disp[y_sort[0]], t_vert_disp[y_sort[1]]);
					p_wari[1] = projback_disp_to_normal_box((y - y_start0), y_all02,
						t_vert_disp[y_sort[0]], t_vert_disp[y_sort[2]]);

#if 0 
					// �v�Z�덷�Ń��C�����΂��Ă��܂��\��������̂� y ����������
// �m�F�R�[�h
					y_[0] = p_wari[0].y;
					y_[1] = p_wari[1].y;

					if ((int)y_[0] != y || (int)y_[1] != y) {
						int i = 0;	// �u���[�N�|�C���g��u�����߂̃_�~�[
					}
#endif

					if( p_wari[0].x <= p_wari[1].x )
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

				int	y_all12 = (int)t_vert_disp[y_sort[2]].y - (int)t_vert_disp[y_sort[1]].y;
				int y_start1 = (int)t_vert_disp[y_sort[1]].y;

				for (int y = t_vert_disp[y_sort[1]].y; y < t_vert_disp[y_sort[2]].y - 1; y++)
				{
					//					std::cout << "y10: " << y << ", x : " << temp_x01 << " - " << temp_x12 << std::endl;
					//					painter->point(jhl_xy_i(temp_x12, y), 0);
					//					painter->point(jhl_xy_i(temp_x02, y), 0);
					painter->line_h(y, temp_x12, temp_x02);

					// z ��h��
					p_wari[0] = projback_disp_to_normal_box((y - y_start1), y_all12,
						t_vert_disp[y_sort[1]], t_vert_disp[y_sort[2]]);
					p_wari[1] = projback_disp_to_normal_box((y - y_start0), y_all02,
						t_vert_disp[y_sort[0]], t_vert_disp[y_sort[2]]);
					if( p_wari[0].x <= p_wari[1].x )
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

				// todo �Ō�̃��C���������Ă�(��Ń��[�v�̍Ōオ -1 �B �͂ݏo���h�~�̈��Ղȍ�)
			}
		}
		std::cout << "zmin,max = " << z_min << ", " << z_max << std::endl;
		min_max_clear();
	}
#if 0
	case z�\�[�g
#endif
		break;
#if 0
	case drawType_phong:				// �t�H���V�F�[�f�B���O
		�e���_�̖@�� �� ���̒��_�����L����S�|���S���̖@���̕���
		���f���ǂݍ��ݎ��A�܂��̓A�j���[�V�������ɂ��炩���ߌv�Z���Ă����ׂ����낤
			hw�ł͂ǂ����Ă�񂾂낤�H
		�O���[�v���Ƃ�n�̌v�Z������(���[�J�����W�Łj
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


// �����ꖇ���Ԃ������H
// todo ���͌v�Z�ʏ��Ȃ��@���c���ɍŋ߂�DSP���߂�������@�I�[�o�[�w�b�h�̂ق����傫�������H
jhl_xyz jhl3Dlib::transToDisp(int vert_idx)
{
	if (p_TTDcache == NULL)
	{
		return(transMat * tgtMdl->verts[vert_idx]);	// �L���b�V���������� / ���g�p
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

	// �z�񂾂��ǁA�A���C�������g�̓s����memset����O�Ɏ��ۂ�fill����T�C�Y���Z�o
	// ���ʁH�i���ł���Ă�H�j // todo �����Ƃ������@
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
	for (int i = 0; i < mdl.n_group; i++)
	{
		grpAttrib t = mdl.attr[i];
		if (detail) {
			cout << " member : ";
			for (int j = 0; j < t.n_member; j++)
			{
				cout << t.member[j] << " ,";
			}
			cout << endl;
		}

		if (t.pTex == 0)
		{
			cout << " no texture" << endl;
		}
		else
		{
			// todo to be written
			//	char*	texName;
			// char*	pTex;
			// texUv*	uv;
		}

		cout << " color : (" << t.color.r << ", " << t.color.g << ", " << t.color.b << ")" << endl;
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
