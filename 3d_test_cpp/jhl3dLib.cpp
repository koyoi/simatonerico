#include "stdafx.h"

#include "jhl3dLib.h"
#include "jhl3Dlib.h"
#include "readData.h"

//-----------------------------------------------------------

En_draw_type	jhl3Dlib::draw_type = drawType_vertex; // drawType_flat;
modelData*	jhl3Dlib::tgtMdl;
jhl_rgb		jhl3Dlib::light_ambient;
dir_light	jhl3Dlib::light_directional[2];

jhl_xy_i		jhl3Dlib::display;
viewport_config	jhl3Dlib::vp;

matHomo4		jhl3Dlib::view_mat;	//	�r���[�ϊ�(���f���ϊ��� tgtMdl ����)
matHomo4_full	jhl3Dlib::proj_mat;	//	���e�ϊ�
matHomo4		jhl3Dlib::disp_mat;	//	�f�B�X�v���C�ϊ�
matHomo4_full	jhl3Dlib::transMat;

disp_ocv2*		jhl3Dlib::painter;


//-----------------------------------------------------------

// �\������
float jhl3Dlib::check_side(int pol_idx)
{
	// �|���S���̒��_ p0,1,2 �̍��ʃx�N�g�� v1=p1-p0, v2=p2-p0 �̖@���x�N�g�����A����(���J�����ϊ���Ȃ̂ŁA[0,0,-1]) �ƌ����������H
	// �œK���ς݌���
	pol_def	pol = tgtMdl->poldef[pol_idx];
	jhl_xyz p[3];

	p[0] = tgtMdl->vert[pol.a];
	p[1] = tgtMdl->vert[pol.b];
	p[2] = tgtMdl->vert[pol.c];
	return(-((p[1].x - p[0].x)*(p[2].y - p[0].y) - (p[0].x - p[2].x)*(p[1].y - p[0].y)));
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
/*
	disp_mat = matHomo4();
	disp_mat.m[0 * 4 + 0] = (float)window.x / 2;
	disp_mat.m[1 * 4 + 1] = (float)window.y / 2;
	disp_mat.m[0 * 4 + 3] = (float)window.x / 2;
	disp_mat.m[1 * 4 + 3] = (float)window.y / 2;
	disp_mat.m[2 * 4 + 2] = 1.0f;
	disp_mat.m[3 * 4 + 3] = 1.0f;
*/
	display = window_ /2;
}

#if 0
set_disp_trans(const jhl_xy_i& window);�@�̂��߁A�p�~�\��
matHomo4 jhl3Dlib::disp_trans(const jhl_xy_i& window)
{
	matHomo4 m;
	m.m[0 * 4 + 0] = (float)window.x / 2;
	m.m[1 * 4 + 1] = (float)window.y / 2;
	m.m[0 * 4 + 3] = (float)window.x / 2;
	m.m[1 * 4 + 3] = (float)window.y / 2;
	m.m[2 * 4 + 2] = 1.0f;
	m.m[3 * 4 + 3] = 1.0f;
	return m;
}

matHomo4 jhl3Dlib::disp_trans_inv(matHomo4& mat_disp_trans)
{
	matHomo4 m;
	m.m[0 * 4 + 0] = 1 / (mat_disp_trans.m[0 * 4 + 0]);
	m.m[1 * 4 + 1] = 1 / (mat_disp_trans.m[1 * 4 + 1]);
	m.m[2 * 4 + 2] = 1;
	m.m[3 * 4 + 3] = 1;
	m.m[0 * 4 + 3] = -1;
	m.m[1 * 4 + 3] = -1;
	return m;
}
#endif

#if 1	// �㉺���E�Ώ̃o�[�W����
void jhl3Dlib::set_proj_mat_norm(viewport_config & m_)
{
	vp = m_;
	matHomo4_full mp;

	int	rl, tb, fn;

	rl = vp.right - vp.left;
	tb = vp.top - vp.btm;
	fn = vp.far - vp.near;
	mp.m[0 * 4 + 0] = (float)2 * vp.near / rl;
	mp.m[1 * 4 + 1] = (float)2 * vp.near / tb;
	mp.m[0 * 4 + 2] = (float)0;	// (vp.right + vp.left) / rl;
	mp.m[1 * 4 + 2] = (float)0; // (vp.top + vp.btm) / tb;
	mp.m[2 * 4 + 2] = (float)-(vp.far + vp.near) / fn;
	mp.m[3 * 4 + 2] = (float)-1;
	mp.m[2 * 4 + 3] = (float)-2 * vp.far * vp.near / fn;
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
	fn = vp.far - vp.near;
	mp.m[0 * 4 + 0] = (float)2 / rl;
	mp.m[1 * 4 + 1] = (float)2 / tb;
	mp.m[2 * 4 + 2] = (float)-2 / fn;
	mp.m[0 * 4 + 3] = (float)0;	// -(vp.right + vp.left) / rl;
	mp.m[1 * 4 + 3] = (float)0;	// -(vp.top + vp.btm) / tb;
	mp.m[2 * 4 + 3] = (float)-(vp.far + vp.near) / fn;

	mp.m[3 * 4 + 3] = 1;
	proj_mat = mp;
}
#else
void jhl3Dlib::set_proj_mat_norm(viewport_config & m_)
{
	vp = m_;
	matHomo4_full mp;

	int	rl, tb, fn;

	rl = vp.right - vp.left;
	tb = vp.top - vp.btm;
	fn = vp.far - vp.near;
	mp.m[0 * 4 + 0] = (float)2 * vp.near / rl;
	mp.m[1 * 4 + 1] = (float)2 * vp.near / tb;
	mp.m[0 * 4 + 2] = (float)(vp.right + vp.left) / rl;
	mp.m[1 * 4 + 2] = (float)(vp.top + vp.btm) / tb;
	mp.m[2 * 4 + 2] = (float)-(vp.far + vp.near) / fn;
	mp.m[3 * 4 + 2] = (float)-1;
	mp.m[2 * 4 + 3] = (float)-2 * vp.far * vp.near / fn;
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
	fn = vp.far - vp.near;
	mp.m[0 * 4 + 0] = (float)2 / rl;
	mp.m[1 * 4 + 1] = (float)2 / tb;
	mp.m[2 * 4 + 2] = (float)-2 / fn;
	mp.m[0 * 4 + 3] = (float)-(vp.right + vp.left) / rl;
	mp.m[1 * 4 + 3] = (float)-(vp.top + vp.btm) / tb;
	mp.m[2 * 4 + 3] = (float)-(vp.far + vp.near) / fn;

	mp.m[3 * 4 + 3] = 1;
	proj_mat = mp;
}
#endif


// �f�o�C�X���W�܂ŕϊ�����s������߂ăZ�b�g
// �[���ɂȂ�̂Ƃ��A�œK���ς݁B
/*
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
	//	matHomo4_full m_proj_disp = proj_mat * disp_mat;	�݂��Ƀ[�����炯�Ȃ̂ŏ�������
	matHomo4_full m_proj_disp = proj_mat;
	m_proj_disp.m[0 * 4 + 0] *= display.x;
	m_proj_disp.m[1 * 4 + 1] *= display.y;
	m_proj_disp.m[0 * 4 + 2] = m_proj_disp.m[3 * 4 + 2] * display.x;
	m_proj_disp.m[0 * 4 + 3] = m_proj_disp.m[3 * 4 + 3] * display.x;
	m_proj_disp.m[1 * 4 + 2] = m_proj_disp.m[3 * 4 + 2] * display.y;
	m_proj_disp.m[1 * 4 + 3] = m_proj_disp.m[3 * 4 + 3] * display.y;

	transMat = m_proj_disp * m_model_view;	// todo ��������

	std::cout << "model mat" << std::endl;
	std::cout << mdl_mat << std::endl << std::endl;

	std::cout << "view mat" << std::endl;
	std::cout << view_mat << std::endl << std::endl;

	std::cout << "proj mat" << std::endl;
	std::cout << proj_mat << std::endl << std::endl;

	std::cout << "disp mat" << std::endl;
	std::cout << m_proj_disp << std::endl << std::endl;

	std::cout << "view_mat*mdl_mat" << std::endl;
	std::cout << m_model_view << std::endl << std::endl;

	std::cout << "transMat" << std::endl;
	std::cout << transMat << std::endl << std::endl;

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
jhl_xyz jhl3Dlib::_toushi_shindo(points, far, near)
{
	//def pers_r(p, far, near) :
	dest = []
		for p in points :
	x = -near / p[2] * p[0]
		y = -near / p[2] * p[1]
		z = -far*near / p[2] - (far + near) / 2
		z = z*(-2 / (far - near))
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


void modelData::dataDump(modelData& mdl)
{
	using namespace std;
	{
		cout << "vertex : " << mdl.n_vert << endl;
		jhl_xyz* t = new jhl_xyz[mdl.n_vert];
		t = mdl.vert;							// todo orz
		for (int i = 0; i < mdl.n_vert; i++)
		{
			cout << i << ": ( " << t[i].x << ", " << t[i].y << ", " << t[i].z << " )" << endl;
		}
		delete t;
	}

	{
		cout << "polygon : " << mdl.n_pol << endl;
		pol_def* t = new pol_def[mdl.n_pol];
		t = mdl.poldef;

		for (int i = 0; i < mdl.n_pol; i++)
		{
			cout << i << " : ( " << t[i].a << ", " << t[i].b << ", " << t[i].c << " )" << endl;
		}
	}

	{
		cout << "attributes : " << endl;
		for (int i = 0; i < mdl.n_group; i++)
		{
			grpAttrib t = mdl.attr[i];

			cout << " member : ";
			for (int j = 0; j < t.n_member; j++)
			{
				cout << t.member[j] << " ,";
			}
			cout << endl;

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
}



/*
t_work = t.dot(t_work)       // ���f���̉�]�A�g��k��(�A�E�ړ�)�s��̍X�V

temp = r.dot(t_work)         // ���f���̈ړ�
temp = eye_vec.dot(temp)     // ���_�̎w��
temp = area.dot(temp)        // ���e�ϊ�       // todo z �����K���͈͂���o�Ă��܂��H�H�@
//    print("mat: obj trans")
//    print(temp)
//    print("vec: model points")
//    print(points)
poi_trans = trans_vec(points, temp) //���X�̕ϊ������f���Ɏ{��
//    print("transed points")
//   for v in pols :
//       print(v)
*/



int jhl3Dlib::drawLines(polMdl & mdl)
{
	int rv = 0;
	tgtMdl = &mdl.model;

	jhl_xyz	t_vert[3];
	jhl_xyz	t_vert_disp[3]; 

	setTransMat( mdl.model_mat );

	switch ( jhl3Dlib::draw_type )
	{
	case drawType_vertex:
		for (int i = 0; i < mdl.model.n_vert; i++)
		{
			t_vert[0] = jhl3Dlib::transMat * mdl.model.vert[i];
			painter->point(t_vert[0]);
		}

		break;
	case drawType_line:
#if 0
		if (draw_type == 1) : // ���C���t���[���i�e�ʏ����Ȃ�)
			for vecs in points_disp :
		t.append(tuple(map(int, vecs[0:2]))) // cv2.line �����������󂯕t���Ȃ�
			for d in polygons :
		im = cv2.line(im, t[d[0]], t[d[1]], line_color, line_width);
		im = cv2.line(im, t[d[1]], t[d[2]], line_color, line_width);
		im = cv2.line(im, t[d[2]], t[d[0]], line_color, line_width);
#endif
		break;
	case drawType_line_front_face:
#if 0
		elif(draw_type == 2) : // ���C���t���[���A�e�ʏ�������
			pols_drawn = 0;
		for vecs in points_disp :
		t.append(tuple(map(int, vecs[0:2]))) // cv2.line �����������󂯕t���Ȃ�
			for d in polygons :
		if (check_side((points_disp[d[0]], points_disp[d[1]], points_disp[d[2]])) > 0) :
			//                print(line_color)
			//                print(t[d[0]], t[d[1]], t[d[2]])
			im = cv2.line(im, t[d[0]], t[d[1]], line_color, line_width);
		im = cv2.line(im, t[d[1]], t[d[2]], line_color, line_width);
		im = cv2.line(im, t[d[2]], t[d[0]], line_color, line_width);
		pols_drawn += 1;
		print("polygons drawn: ", pols_drawn);

#endif
		break;
	case drawType_flat:
		break;
#if 0
		elif(draw_type == 3) : // �P�F�|���S���i��������j;
			z_min = 255
			z_max = 0
			z_calc = 0

			pols_drawn = 0
			light_color = []
			for d in polygons :
		pd0 = points_disp[d[0]] // �����ڂ��Ă���|���S���̊e���_   Points_disp[Dn]
			pd1 = points_disp[d[1]]
			pd2 = points_disp[d[2]]


			// ���ʂ͕`�悵�Ȃ�
			if (check_side((pd0, pd1, pd2)) > 0) :
				// (�����Ƃ̊p�x���K�v�Ȃ�A���̂Ƃ�... �V�F�[�_�Č��H)
				light_color = (calc_lighting((points[d[0]], points[d[1]], points[d[2]]))) * 255
				light_color_int = (int(light_color[0]), int(light_color[1]), int(light_color[2]))
				//                light_color_int = map(int, light_color.flatten())
				//                print("light_color_int", light_color_int)
				// y �\�[�g�i X�����@���VRAM �A�h���X�C���N�������g�ŃA�N�Z�X����������
				//                print("points_disp")
				//                for ppp in d :
				//                    print ppp
				//                print pd0
				//                print pd1
				//                print pd2
				// todo sorted �ƌ������\�b�h�����邼
				if (pd0[1] <= pd1[1] and pd0[1] <= pd2[1]) :
					idx0 = d[0]
					if (pd1[1] <= pd2[1]) :
						idx1 = d[1]
						idx2 = d[2]
					else :
						idx1 = d[2]
						idx2 = d[1]
						elif(pd1[1] <= pd0[1] and pd1[1] <= pd2[1]) :
						idx0 = d[1]
						if (pd0[1] <= pd2[1]) :
							idx1 = d[0]
							idx2 = d[2]
						else :
							idx1 = d[2]
							idx2 = d[0]
				else:
		idx0 = d[2]
			if (pd0[1] <= pd1[1]) :
				idx1 = d[0]
				idx2 = d[1]
			else :
				idx1 = d[1]
				idx2 = d[0]
				//                print("sorted")
				//                print(idx0, idx1, idx2)
				//                print(points_disp[idx0], points_disp[idx1], print(points_disp[idx2])

				// �f�B�X�v���C��ňꃉ�C�����Ƃ��r�߂�
				// todo ����[���̑Ή�

				pd0s = points_disp[idx0] // �h��Ԃ��̓s���� y �\�[�g����    PointsDispSorted
				pd1s = points_disp[idx1]
				pd2s = points_disp[idx2]

				a1s = (pd1s[0] - pd0s[0]) / (pd1s[1] - pd0s[1] + 0.00001)   // todo toria �[�����Z�΍�
				a2s = (pd2s[0] - pd0s[0]) / (pd2s[1] - pd0s[1] + 0.00001)
				a3s = (pd2s[0] - pd1s[0]) / (pd2s[1] - pd1s[1] + 0.00001)
				//                print("katamuki", a1s, a2s, a3s)

				// p0 - p1 �܂�y�𑝂₵�Ȃ���A�P���C�������������ēh�� ... z���l���Ȃ��Ă������炢�낢��ȗ�
				for y_ in range(int(pd0s[1]) + 1, int(pd1s[1])) :
					x0s = pd0s[0] + a1s * (y_ - pd0s[1])   // pd0s[1] : �����l
					x1s = pd0s[0] + a2s * (y_ - pd0s[1])
					im = cv2.line(im, (int(x0s), y_), (int(x1s), y_), light_color_int, line_width)
					//                print(",")

					//// �S�~�A�͂ݏo���h�~�iint�ۂ߂̂���)
					x0s = pd1s[0]
					x1s = pd0s[0] + a2s * (pd1s[1] - pd0s[1])
					y_ = int(pd1s[1])
					im = cv2.line(im, (int(x0s), y_), (int(x1s), y_), light_color_int, line_width)

					// p1 - p2 �܂�y�𑝂₵�Ȃ���A�P���C�������������ēh��
					for y_ in range(int(pd1s[1]) + 1, int(pd2s[1])) :
						x0s = pd1s[0] + a3s * (y_ - pd1s[1])
						x1s = pd0s[0] + a2s * (y_ - pd0s[1])
						im = cv2.line(im, (int(x0s), y_), (int(x1s), y_), light_color_int, line_width)
						//                    img_z = cv2.line(img_z, (int(x0s), y_), (int(x1s), y_), light_color_int[0], line_width)

						// z map �쐬 //
						p0s = points[idx0]
						p1s = points[idx1]
						p2s = points[idx2]
						//                print("triangle_orig")
						//                print(p0s, ",", ",", p1s, ",", ",", p2s)
						//                print("triangle_disp")
						//                print(pd0s, ",", pd1s, ",", pd2s)

						//                print("")
						//1                print("t0_wari,t0_x,y,z,,t1_wari,t1_x,y,z,, scr_y, scr_x1_t,x2_t")

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
	switch (draw_type)
	{
	case(drawType_vertex):
		draw_type = drawType_line;
		break;
	case(drawType_line):
		draw_type = drawType_line_front_face;
		break;
	case(drawType_line_front_face):
		draw_type = drawType_flat;
		break;
	case(drawType_flat):
		draw_type = drawType_vertex;
		break;
	default:
		draw_type = drawType_flat;
		break;
	};
	return draw_type;
}
