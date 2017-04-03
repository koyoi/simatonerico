#include "stdafx.h"

#include <iostream>
#include <cstring>

#include "basic3dCalc.h"
#include <cmath>


modelData*	Basic3dCalc::tgtMdl;
jhl_rgb		Basic3dCalc::light_ambient;
dir_light	Basic3dCalc::light_directional[2];


// 表裏判定
float Basic3dCalc::check_side(int pol_idx)
{
	// ポリゴンの頂点 p0,1,2 の作る面ベクトル v1=p1-p0, v2=p2-p0 の法線ベクトルが、視線(＝カメラ変換後なので、[0,0,-1]) と向き合うか？
	// 最適化済み結果
	pol_def	pol = tgtMdl->poldef[pol_idx];
	jhl_xyz p[3];

	p[0] = tgtMdl->vert[pol.a];
	p[1] = tgtMdl->vert[pol.b];
	p[2] = tgtMdl->vert[pol.c];
	return( - (( p[1].x - p[0].x)*(p[2].y - p[0].y) - (p[0].x - p[2].x)*(p[1].y - p[0].y)) );
}



// あたる光の色を計算
// 引数は各頂点の座標
// 環境光＋平行光(面との内積を取る)。返値と、面の色を掛けて
jhl_rgb Basic3dCalc::calc_lighting(int pol_idx)
{
	pol_def	pol = tgtMdl->poldef[pol_idx];
	jhl_xyz p[3];

	jhl_rgb rv = light_ambient;

	jhl_xyz v1 = p[1] - p[0];
	jhl_xyz v2 = p[2] - p[0];
	jhl_xyz n = v1*v2;	// ベクトル積
//	n /= n.norm;	todo ???

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


void Basic3dCalc::ident_matHomo4(matHomo4& t)
{
	std::memse memt[]
}

#if 0


// /// 行列計算など　///

// 平行移動
jhl_xyz Basic3dCalc::trans(x, y, z)
{
	t = i4.copy()
		t[0, 3] = x
		t[1, 3] = y
		t[2, 3] = z
		return t
}
	// 拡大縮小
jhl_xyz Basic3dCalc::scale(s) {
	t = i4.copy()
		t[0, 0] = s
		t[1, 1] = s
		t[2, 2] = s
		return t
}

jhl_xyz Basic3dCalc::scale3(sx, sy, sz) {
	t = i4.copy()
		t[0, 0] = sx
		t[1, 1] = sy
		t[2, 2] = sz
		return t
}

jhl_xyz Basic3dCalc::rot_axis_x(a) { // a[rad]
	t = i4.copy()
		t[1, 1] = math.cos(a)
		t[1, 2] = -math.sin(a)
		t[2, 1] = math.sin(a)
		t[2, 2] = math.cos(a)
		return t
}

jhl_xyz Basic3dCalc::rot_axis_y(a)
{
	t = i4.copy()
		t[0, 0] = math.cos(a)
		t[0, 2] = math.sin(a)
		t[2, 0] = -math.sin(a)
		t[2, 2] = math.cos(a)
		return t
}
jhl_xyz Basic3dCalc::rot_axis_z(a)
{
	t = i4.copy()
		t[0, 0] = math.cos(a)
		t[0, 1] = -math.sin(a)
		t[1, 0] = math.sin(a)
		t[1, 1] = math.cos(a)
		return t
}

jhl_xyz Basic3dCalc::rot_by_vec(l, m, n, s) : // (l, m, n) ベクトルを軸にs[rad]回転
{	// 単位ベクトルでないと拡縮、回転速度にも掛かる
	t = i4.copy()
		cs = math.cos(s)
		ss = math.sin(s)

		t[0, 0] = l*l*(1 - cs) + cs
		t[0, 1] = l*m*(1 - cs) - n*ss
		t[0, 2] = l*n*(1 - cs) + m*ss
		t[1, 0] = l*m*(1 - cs) + n*ss
		t[1, 1] = m*m*(1 - cs) + cs
		t[1, 2] = m*n*(1 - cs) - l*ss
		t[2, 0] = l*n*(1 - cs) - m*ss
		t[2, 1] = m*n*(1 - cs) + l*ss
		t[2, 2] = n*n*(1 - cs) + cs
		return t
}



void Basic3dCalc::view_mat(eye_loc, u_vec, tgt_loc)
{
	// t : 視線方向の逆ベクトル
	// r : 視線をｚ方向としたときの、x方向に当たるベクトル
	// s : 同、y方向
	t = eye_loc - tgt_loc
		r = np.cross(u_vec, t)
		s = np.cross(t, r)

		t_n = np.linalg.norm(t)
		r_n = np.linalg.norm(r)
		s_n = np.linalg.norm(s)

		rv = i4.copy()
		rv[0, 0:3] = r[0:3] / r_n
		rv[1, 0:3] = s[0:3] / s_n
		rv[2, 0:3] = t[0:3] / t_n
		rv[0, 3] = -eye_loc[0]
		rv[1, 3] = -eye_loc[1]
		rv[2, 3] = -eye_loc[2]

		//    print("rv")
		//    print(rv)
		return rv
}


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


// スクリーン座標上 p0,p1 の wari 分割割合から、標準視差台形内の座標を割り出す 
jhl_xyz Basic3dCalc::proj_norm(wari, p0, p1)
{
	rv = [0., 0., 0.]

		rv[2] = 1 / ((1 - wari) / p0[2] + wari / p1[2])
		rv[0] = ((p0[0] / p0[2]) * (1 - wari) + (p1[0] / p1[2]) * wari) * rv[2]
		rv[1] = ((p0[1] / p0[2]) * (1 - wari) + (p1[1] / p1[2]) * wari) * rv[2]
		return(rv)
}



// 透視深度（？）
// Zやテクスチャ座標を計算するのに使う
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


// ポイントに透視変換までの変換を適用
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


// ディスプレイ座標へ変換
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
	

	// 透視変換後のを入れて、ディスプレイ座標へ変換
void  	Basic3dCalc::trans_disp(points)
{
	temp = np.matrix([0., 0, 0, 0])
		dest = []
		for p in points :
	temp[0, 0:3] = p[0:3]
		temp[0, 3] = 1
		dest.append(((viewport_trans.dot(temp.T)).T).tolist()[0])   // イヤ実装
		return dest
}

#endif

