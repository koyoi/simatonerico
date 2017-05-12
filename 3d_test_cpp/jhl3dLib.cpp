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
jhl_rgb			jhl3Dlib::light_ambient;
dir_light		jhl3Dlib::light_directional[2];

jhl_xy_i		jhl3Dlib::display;
viewport_config	jhl3Dlib::vp;

matHomo4		jhl3Dlib::view_mat;	//	�r���[�ϊ�(���f���ϊ��� tgtMdl ����)
matHomo4_full	jhl3Dlib::proj_mat;	//	���e�ϊ�
#ifdef DISP_MAT_KAKIKUDASHI
matHomo4		jhl3Dlib::disp_mat;	//	�f�B�X�v���C�ϊ�
#endif
matHomo4_full	jhl3Dlib::transMat;

disp_ocv2*		jhl3Dlib::painter;


// �f�B�X�v���C���W�ւ̕ϊ��Ɏg�p(�L���b�V��)
jhl_xyz*		jhl3Dlib::p_TTDcache;
int				jhl3Dlib::TTDcacheSize;
jhl_xyz*		jhl3Dlib::p_verts;

int	cache_miss;
int	cache_total;

//-----------------------------------------------------------
static void swap(int& a, int& b)
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
float jhl3Dlib::check_side(jhl_xyz verts[3])
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
// �����͊e���_�̍��W
// �����{���s��(�ʂƂ̓��ς����)�B�Ԓl�ƁA�ʂ̐F���|����
jhl_rgb jhl3Dlib::calc_lighting(int pol_idx)
{
	pol_def	pol = tgtMdl->poldef[pol_idx];
	jhl_xyz p[3];

	jhl_rgb rv = light_ambient;

	jhl_xyz v1 = p[1] - p[0];
	jhl_xyz v2 = p[2] - p[0];
	jhl_xyz n = v1*v2;	// �x�N�g����
	n /= n.norm();

	for (int i = 0; i < 2; i++)
	{
		float e = -n.dot(light_directional[i].dir);
		if (e > 0)
		{
			//        print(" add light ", 1.*e*l[1])
			rv += light_directional[i].col * e;
		}
	}
	return (rv);
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
void jhl3Dlib::setTransMat(matHomo4 & mdl_mat)
{
	matHomo4 m_model_view = view_mat*mdl_mat;	// mview x trans ���f���r���[�ϊ�

#ifdef DISP_MAT_KAKIKUDASHI
	matHomo4_full m_proj_disp = proj_mat * disp_mat;
//	std::cout << "a:" << std::endl << m_proj_disp << std::endl;
#else	// ��������
	matHomo4_full	m_proj_disp = proj_mat;	// todo ���̓[������
	m_proj_disp.m[0 * 4 + 0] *= display.x;
	m_proj_disp.m[1 * 4 + 1] *= display.y;
	m_proj_disp.m[0 * 4 + 2] = m_proj_disp.m[3 * 4 + 2] * display.x;
	m_proj_disp.m[0 * 4 + 3] = m_proj_disp.m[3 * 4 + 3] * display.x;
	m_proj_disp.m[1 * 4 + 2] = m_proj_disp.m[3 * 4 + 2] * display.y;
	m_proj_disp.m[1 * 4 + 3] = m_proj_disp.m[3 * 4 + 3] * display.y;

//	std::cout << "b:" << std::endl << m_proj_disp << std::endl;
#endif

#if 0
	 transMat = m_proj_disp * m_model_view;
	 // 
#else	// ��������
/* maxima code
#m: m_model_view
#mt : m_proj_disp
point1:matrix([x,y,z,1]);
mat_model_view:matrix([m_00,m_01,m_02,v_0],[m_10,m_11,m_12,v_1],[m_20,m_21,m_22,v_2],[0,0,0,1]);
mat_disp_proj:matrix([mt00,0,mt02,mt03],[0,mt11,mt12,mt13],[0,0,mt22,mt23],[0,0,mt32,mt33]);
mat:mat_disp_proj.mat_model_view;
result:mat.point1;
*/
	transMat.m[0 * 4 + 0] = m_model_view.m[2 * 3 + 0] * m_proj_disp.m[0 * 4 + 2] + m_model_view.m[0 * 3 + 0] * m_proj_disp.m[0 * 4 + 0];
	transMat.m[0 * 4 + 1] = m_model_view.m[2 * 3 + 1] * m_proj_disp.m[0 * 4 + 2] + m_model_view.m[0 * 3 + 1] * m_proj_disp.m[0 * 4 + 0];
	transMat.m[0 * 4 + 2] = m_model_view.m[2 * 3 + 2] * m_proj_disp.m[0 * 4 + 2] + m_model_view.m[0 * 3 + 2] * m_proj_disp.m[0 * 4 + 0];
	transMat.m[0 * 4 + 3] = m_proj_disp.m[0 * 4 + 2] * m_model_view.v[2] + m_proj_disp.m[0 * 4 + 0] * m_model_view.v[0] + m_proj_disp.m[0 * 4 + 3];
	transMat.m[1 * 4 + 0] = m_model_view.m[2 * 3 + 0] * m_proj_disp.m[1 * 4 + 2] + m_model_view.m[1 * 3 + 0] * m_proj_disp.m[1 * 4 + 1];
	transMat.m[1 * 4 + 1] = m_model_view.m[2 * 3 + 1] * m_proj_disp.m[1 * 4 + 2] + m_model_view.m[1 * 3 + 1] * m_proj_disp.m[1 * 4 + 1];
	transMat.m[1 * 4 + 2] = m_model_view.m[2 * 3 + 2] * m_proj_disp.m[1 * 4 + 2] + m_model_view.m[1 * 3 + 2] * m_proj_disp.m[1 * 4 + 1];
	transMat.m[1 * 4 + 3] = m_proj_disp.m[1 * 4 + 2] * m_model_view.v[2] + m_proj_disp.m[1 * 4 + 1] * m_model_view.v[1] + m_proj_disp.m[1 * 4 + 3];
	transMat.m[2 * 4 + 0] = m_model_view.m[2 * 3 + 0] * m_proj_disp.m[2 * 4 + 2];
	transMat.m[2 * 4 + 1] = m_model_view.m[2 * 3 + 1] * m_proj_disp.m[2 * 4 + 2];
	transMat.m[2 * 4 + 2] = m_model_view.m[2 * 3 + 2] * m_proj_disp.m[2 * 4 + 2];
	transMat.m[2 * 4 + 3] = m_proj_disp.m[2 * 4 + 2] * m_model_view.v[2] + m_proj_disp.m[2 * 4 + 3];
	transMat.m[3 * 4 + 0] = m_model_view.m[2 * 3 + 0] * m_proj_disp.m[3 * 4 + 2];
	transMat.m[3 * 4 + 1] = m_model_view.m[2 * 3 + 1] * m_proj_disp.m[3 * 4 + 2];
	transMat.m[3 * 4 + 2] = m_model_view.m[2 * 3 + 2] * m_proj_disp.m[3 * 4 + 2];
	transMat.m[3 * 4 + 3] = m_proj_disp.m[3 * 4 + 2] * m_model_view.v[2] + m_proj_disp.m[3 * 4 + 3]; 
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

 	std::cout << "m_model_view (=view_mat * mdl_mat)" << std::endl;
 	std::cout << m_model_view << std::endl;

 	std::cout << "transMat" << std::endl;
 	std::cout << transMat << std::endl;
//*/
}


// �X�N���[�����W�� p0,p1 �� wari ������������A�W��������`���̍��W������o�� 
// todo �P���C����C��
jhl_xyz jhl3Dlib::proj_disp_to_normal_box(float wari, jhl_xyz& p0, jhl_xyz& p1)
{
	jhl_xyz rv;

	rv.z = 1 / ((1 - wari) / p0.z + wari / p1.z);
	rv.x = ((p0.x / p0.z) * (1 - wari) + (p1.x / p1.z) * wari) * rv.z;
	rv.y = ((p0.y / p0.z) * (1 - wari) + (p1.y / p1.z) * wari) * rv.z;
	return(rv);
}


#if 0


// �����[�x�i�H�j
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


// �|�C���g�ɓ����ϊ��܂ł̕ϊ���K�p
void jhl3Dlib::trans_vec(points, w) {
	dest = []
		for v in points :
	temp = v[:]     // deep copy
		temp.append(1.)
		temp_w = np.ravel(w.dot(temp))
		if (temp_w[3] != 0) :
			x = temp_w[0] / temp_w[3]
			y = temp_w[1] / temp_w[3]
			z = temp_w[2] / temp_w[3]
			dest.append((x, y, z))
		else:
	print("w == 0 !!!")
		return dest
}




// �����ϊ���̂����āA�f�B�X�v���C���W�֕ϊ�
void  	jhl3Dlib::trans_disp(points)
{
	temp = np.matrix([0., 0, 0, 0])
		dest = []
		for p in points :
	temp[0, 0:3] = p[0:3]
		temp[0, 3] = 1
		dest.append(((viewport_trans.dot(temp.T)).T).tolist()[0])   // �C������
		return dest
}

#endif


void modelData::dataDump(modelData& mdl, bool detail)
{
	using namespace std;

	cout << "vertex : " << mdl.n_vert << endl;
	if(detail){
		jhl_xyz* t;
		for (int i = 0; i < mdl.n_vert; i++)
		{
			t = &mdl.vert[i];
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



int jhl3Dlib::draw(object& mdl)
{
	int rv = 0;
	tgtMdl = mdl.p_model;

	jhl_xyz	t_vert_disp[3];

	setTransMat( mdl.model_mat );
	set_transToDisp_pObj(tgtMdl->vert);

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

	pol_def t_poldef;
	jhl_xyz* p_vert;

	switch ( jhl3Dlib::draw_type )
	{
	case drawType_vertex:

		for (int i = 0; i < tgtMdl->n_vert; i++)
		{
			t_vert_disp[0] = transToDisp( i );
			painter->point(t_vert_disp[0]);
//			std::cout << tgtMdl->vert[i] << " -> " << t_vert[0];
		}
		break;

	case drawType_line:				// ���C���t���[���i���ʏ����Ȃ�)
	case drawType_line_front_face:	// ���C���t���[���i���ʏ�������)
		for (int i = 0; i < tgtMdl->n_pol; i++)
		{
			t_poldef = tgtMdl->poldef[i];
			p_vert = tgtMdl->vert;

			t_vert_disp[0] = transToDisp( t_poldef.a );
			t_vert_disp[1] = transToDisp( t_poldef.b );
			t_vert_disp[2] = transToDisp( t_poldef.c );

			if( jhl3Dlib::draw_type != drawType_line )
			{
				if (jhl3Dlib::check_side(t_vert_disp) < 0)
				{
					// ���ʁB�X�L�b�v
					continue;
				}
			}
			painter->triangle(t_vert_disp);
		}
	break;

	case drawType_flat:				// �P�F�|���S���i���s�����P�̂݁A�Öقɗ��ʏ��O�j;
	{
		int	y_sort[3] = { 0,1,2 };

		p_vert = tgtMdl->vert;

		for (int i = 0; i < tgtMdl->n_pol; i++)
		{
			t_poldef = tgtMdl->poldef[i];
			p_vert = tgtMdl->vert;

			t_vert_disp[0] = transToDisp(t_poldef.a);
			t_vert_disp[1] = transToDisp(t_poldef.b);
			t_vert_disp[2] = transToDisp(t_poldef.c);

			// ���ʂȂ�X�L�b�v
			if (jhl3Dlib::check_side(t_vert_disp) < 0)
			{
				continue;
			}

			// 1) �|���S�����_�̉�ʏ�y�\�[�g
			sort_y(t_vert_disp, y_sort);

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
				float temp_x12  = t_vert_disp[y_sort[1]].x;

				// todo �͂ݏo���N���b�v
				for (int y = t_vert_disp[y_sort[0]].y; y < t_vert_disp[y_sort[1]].y; y++)
				{
//					std::cout << "y: " << y << ", x : " << temp_x01 << " - " << temp_x02 << std::endl;
//					painter->point(jhl_xy_i(temp_x01, y), 0);
//					painter->point(jhl_xy_i(temp_x02, y), 0);
					painter->line_h(y, temp_x01, temp_x02);

					temp_x01 += delta_x01;
					temp_x02 += delta_x02;
				}

				for (int y = t_vert_disp[y_sort[1]].y; y < t_vert_disp[y_sort[2]].y; y++)
				{
//					std::cout << "y: " << y << ", x : " << temp_x01 << " - " << temp_x12 << std::endl;
//					painter->point(jhl_xy_i(temp_x12, y), 0);
//					painter->point(jhl_xy_i(temp_x02, y), 0);
					painter->line_h(y, temp_x12, temp_x02);

					temp_x12 += delta_x12;
					temp_x02 += delta_x02;
				}
			}
		}

#if 0
						// idx0 �` idx1 �́A�����W�ł� t : (1 - t) �ɕ�������_
						t0s_all = pd1s[1] - pd0s[1]   // �X�N���[�����y�����A��������i���͂��������̃��[�v���j
						t1s_all = pd2s[1] - pd0s[1]   // ���K����Ԃ̒��ł̈ʒu���m�邽�߁A�X�N���[����ł̂��̓_�ɂ�镪���������m�肽��
						for t0s in range(int(t0s_all)) :
							// �N���[����Ő������������Ă��āAx�ɕ��s�Ɉ����āA�����Ȃ߂�Ƃ��Ap0 - p1��ł���n�_, p0 - p2��ł���I�_�̌����W
							pt0 = proj_norm(t0s / t0s_all, p0s, p1s)
							pt0.append(1.)
							pt1 = proj_norm(t0s / t1s_all, p0s, p2s)
							pt1.append(1.)
							//1                    print(t0s / t0s_all, pt0[0:3], "", t0s / t1s_all, pt1[0:3], "", pd0s[1] + t0s)
							// ���� pt0 - pt1 ���X�N���[����łP�s�N�Z�����A�����W�͂Ȃ�Ȃ̂��`�F�b�N
							// ���邽�߂ɁA�����_ t0s �ł́Ax�����v�Z
							temp0 = (viewport_trans.dot((np.matrix(pt0).T)))[0] // x ��������Ȃ�
							temp1 = (viewport_trans.dot((np.matrix(pt1).T)))[0]

							x_start = int(min(temp0, temp1))
							x_end = int(max(temp0, temp1))

							x_all = x_end - x_start
							//2                    print("x scan (disp) :", x_start, " - ", x_end)
							//2                    print("wari_x, norm_x,y,z,disp_x,y,z")
							for x_work in range(x_all) : // �X�N���[�����1pix����
								z_calc += 1
								pt_scan_wari = (x_work / x_all)
								pix_norm = proj_norm(pt_scan_wari, pt0, pt1)
								//2                        print(pt_scan_wari, pix_norm, trans_disp([pix_norm]))

								//                        print(int(z_buff_test[1]), int(z_buff_test[0]), ((z_buff_test[2] + 1) / 2) * 255)
								//                        print("pos: ", x_start + x_work, pd0s[1] + t0s)
								if (pix_norm[2] <= 1 and pix_norm[2] >= -1) :
									z_buff_test = trans_disp([pix_norm])[0]
									//                            z_temp = int(((pix_norm[2] + 1) / 2) * 255)
									z_temp = int((((pix_norm[2] + 1) / 2) * 255 - 251) * 50) // debug �͈͂�ς��Ă݂�
																							 //                            print(z_temp)
									z_min = min(z_min, z_temp)
									z_max = max(z_max, z_temp)
									if (int(img_z[int(pd0s[1] + t0s)][int(x_start + x_work)]) < z_temp) :
										//                                print(int(pd0s[1] + t0s), int(x_start + x_work), ")", int(img_z[int(pd0s[1] + t0s), int(x_start + x_work)]), "<= ", z_temp)
										img_z[int(pd0s[1] + t0s)][int(x_start + x_work)] = z_temp
									else:
		print(int(pd0s[1] + t0s), int(x_start + x_work), ")", int(img_z[int(pd0s[1] + t0s), int(x_start + x_work)]), )


			// z map �����܂� //





			pols_drawn += 1
			print("polygons drawn: ", pols_drawn)
			print("z_min,max", z_min, z_max)
			//        print("// z calc", z_calc)
								else:
#endif
	}
	case drawType_flat_z:				// �P�F�|���S���i���s�����P�̂݁A�Öقɗ��ʏ��O�j;
		break;
	case drawType_flat_lighting:				// �P�F�|���S���i��������j;
//		break;
	default:
		break;
	}


#if 0
	elif(draw_type == 0) :
		x = 0
		elif(draw_type == 2) :
		x = 0
#endif
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
// todo �L���b�V���̃t���b�V���̃^�C�~���O�B
// ����t���[�����ȂǂŁA�������f�������C���X�^���X���`�悷��Ƃ��Ƃ��B
jhl_xyz jhl3Dlib::transToDisp(int vert_idx)
{
	if (p_TTDcache == NULL)
	{
		return(transMat * p_verts[vert_idx]);	// �L���b�V���������� / ���g�p
	}

	if( p_TTDcache[vert_idx] == jhl_xyz(0) )
	{
		p_TTDcache[vert_idx] = transMat * p_verts[vert_idx];
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

	// �z�񂾂��ǁA�A���C�������g�̓s����memset����O�Ɏ��ۂɓh��Ȃ��Ă͂Ȃ�Ȃ��T�C�Y���Z�o
	// ���ʁH�i���ł���Ă�H�j
	jhl_xyz* tbl2 = p_TTDcache;
	tbl2++;
	int stride = (int)tbl2 - (int)p_TTDcache;

	memset(p_TTDcache, 0, stride * TTDcacheSize);
	cache_total = 0;
	cache_miss = 0;
}