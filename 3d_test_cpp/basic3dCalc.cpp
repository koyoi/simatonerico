#include "stdafx.h"

#include <iostream>
#include <cstring>

#include "basic3dCalc.h"
#include <cmath>


modelData*	Basic3dCalc::tgtMdl;
jhl_rgb		Basic3dCalc::light_ambient;
dir_light	Basic3dCalc::light_directional[2];


// �\������
float Basic3dCalc::check_side(int pol_idx)
{
	// �|���S���̒��_ p0,1,2 �̍��ʃx�N�g�� v1=p1-p0, v2=p2-p0 �̖@���x�N�g�����A����(���J�����ϊ���Ȃ̂ŁA[0,0,-1]) �ƌ����������H
	// �œK���ς݌���
	pol_def	pol = tgtMdl->poldef[pol_idx];
	jhl_xyz p[3];

	p[0] = tgtMdl->vert[pol.a];
	p[1] = tgtMdl->vert[pol.b];
	p[2] = tgtMdl->vert[pol.c];
	return( - (( p[1].x - p[0].x)*(p[2].y - p[0].y) - (p[0].x - p[2].x)*(p[1].y - p[0].y)) );
}



// ��������̐F���v�Z
// �����͊e���_�̍��W
// �����{���s��(�ʂƂ̓��ς����)�B�Ԓl�ƁA�ʂ̐F���|����
jhl_rgb Basic3dCalc::calc_lighting(int pol_idx)
{
	pol_def	pol = tgtMdl->poldef[pol_idx];
	jhl_xyz p[3];

	jhl_rgb rv = light_ambient;

	jhl_xyz v1 = p[1] - p[0];
	jhl_xyz v2 = p[2] - p[0];
	jhl_xyz n = v1*v2;	// �x�N�g����
	n /= n.norm();

	for (int i = 0; i < 2 ; i++)
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


void Basic3dCalc::view_mat(matHomo4& eye_vec, const jhl_xyz& eye_loc, const jhl_xyz& u_vec, const jhl_xyz& tgt_loc)
{
	// todo
}


matHomo4 Basic3dCalc::view_mat(const jhl_xyz& eye_loc, const jhl_xyz& u_vec, const jhl_xyz& tgt_loc)
{
	// t : ���������̋t�x�N�g��
	// r : �������������Ƃ����Ƃ��́Ax�����ɓ�����x�N�g��
	// s : ���Ay����
	jhl_xyz t,r,s;

	t = eye_loc - tgt_loc;
	r = u_vec * t;
	s = t * r;

	matHomo4 rv(r / r.norm(), s / s.norm(), t / t.norm(), -eye_loc);
	return(rv);
}


#if 0
matHomo4 Basic3dCalc::norm_ortho(top, btm, left, right, far, near)
{
	m0 = i4.copy()
		rl = right - left
		tb = top - btm
		fn = far - near
		m0[0, 0] = 2 / rl
		m0[1, 1] = 2 / tb
		m0[2, 2] = -2 / fn
		m0[0, 3] = -(right + left) / rl
		m0[1, 3] = -(top + btm) / tb
		m0[2, 3] = -(far + near) / fn
		return m0
}

matHomo4 Basic3dCalc::norm_pers(top, btm, left, right, far, near)
{
	mp = i4.copy()
		rl = right - left
		tb = top - btm
		fn = far - near
		mp[0, 0] = 2 * near / rl
		mp[1, 1] = 2 * near / tb
		mp[0, 2] = (right + left) / rl
		mp[1, 2] = (top + btm) / tb
		mp[2, 2] = -(far + near) / fn
		mp[3, 2] = -1
		mp[2, 3] = -2 * far*near / fn
		mp[3, 3] = 0
		return mp
}


// �X�N���[�����W�� p0,p1 �� wari ������������A�W��������`���̍��W������o�� 
jhl_xyz Basic3dCalc::proj_norm(wari, p0, p1)
{
	rv = [0., 0., 0.]

		rv[2] = 1 / ((1 - wari) / p0[2] + wari / p1[2])
		rv[0] = ((p0[0] / p0[2]) * (1 - wari) + (p1[0] / p1[2]) * wari) * rv[2]
		rv[1] = ((p0[1] / p0[2]) * (1 - wari) + (p1[1] / p1[2]) * wari) * rv[2]
		return(rv)
}



// �����[�x�i�H�j
// Z��e�N�X�`�����W���v�Z����̂Ɏg��
jhl_xyz Basic3dCalc::_toushi_shindo(points, far, near)
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
void Basic3dCalc::trans_vec(points, w) {
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


// �f�B�X�v���C���W�֕ϊ�
void Basic3dCalc::disp_trans(wid, hei)
{
	m = i4.copy()
		m[0, 0] = wid / 2
		m[1, 1] = hei / 2
		m[0, 3] = wid / 2
		m[1, 3] = hei / 2
		return m
}

void Basic3dCalc::disp_trans_inv(mat_disp_trans)
{
	m = i4.copy()
		m[0, 0] = 1 / (mat_disp_trans[0, 0])
		m[1, 1] = 1 / (mat_disp_trans[1, 1])
		m[2, 2] = 1
		m[3, 3] = 1
		m[0, 3] = -1
		m[1, 3] = -1
		return m
}
	

	// �����ϊ���̂����āA�f�B�X�v���C���W�֕ϊ�
void  	Basic3dCalc::trans_disp(points)
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

void modelData::dataDump(void)
{
	using namespace std;
	{
		cout << "vertex : " << n_vert << endl;
		jhl_xyz* t = new jhl_xyz[n_vert];
		t = vert;							// todo orz
		for (int i = 0; i < n_vert; i++)
		{
			cout << i << ": ( " << t[i].x << ", " << t[i].y << ", " << t[i].z << " )" << endl;
		}
		delete t;
	}

	{
		cout << "polygon : " << n_pol << endl;
		pol_def* t = new pol_def[n_pol];
		t = poldef;

		for (int i = 0; i < n_pol; i++)
		{
			cout << i << " : ( " << t[i].a << ", " << t[i].b << ", " << t[i].c << " )" << endl;
		}
	}

	{
		cout << "attributes : " << endl;
		for (int i = 0; i < n_group; i++)
		{
			grpAttrib t = attr[i];

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
