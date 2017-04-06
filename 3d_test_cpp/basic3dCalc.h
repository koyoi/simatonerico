#pragma once
#include <cmath>
#include <cstring>
#include <iostream>	// << オペレータ


class jhl_xyz {
public:
	float x;	// 座標
	float y;
	float z;

	jhl_xyz() {};
	jhl_xyz(float x_, float y_, float z_):
		x( x_ ), y( y_ ), z( z_ )
	{}

	jhl_xyz& operator+=(jhl_xyz rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	jhl_xyz operator-( const jhl_xyz& rhs) const {
		jhl_xyz t;
		t.x = x - rhs.x;
		t.y = y - rhs.y;
		t.z = z - rhs.z;
		return(t);
	}

	jhl_xyz operator-() const {
		jhl_xyz t;
		t.x = -x;
		t.y = -y;
		t.z = -z;
		return(t);
	}

	jhl_xyz operator/(float rhs) {
		jhl_xyz t;
		t.x = x / rhs;
		t.y = y / rhs;
		t.z = z / rhs;
		return t;
	}

	jhl_xyz& operator/=( float rhs ) {
		x /= rhs;
		y /= rhs; 
		z /= rhs;
		return *this;
	}

	// ベクトルならでは
	// ノルム
	float norm() {
		return sqrtf(x * x + y + y + z + z);
	}

	// 内積
	float dot(jhl_xyz b) {
		return x * b.x + y * b.y + z * b.z;
	}

	// 外積
	jhl_xyz operator*(jhl_xyz rhs) {	
		jhl_xyz t;
		t.x = y * rhs.z - z * rhs.y;
		t.y = z * rhs.x - x * rhs.z;
		t.z = x * rhs.y - y * rhs.x;
		return t;
	}

	jhl_xyz operator*(const jhl_xyz rhs) const {
		jhl_xyz t;
		t.x = y * rhs.z - z * rhs.y;
		t.y = z * rhs.x - x * rhs.z;
		t.z = x * rhs.y - y * rhs.x;
		return t;
	}


};

// 頂点へのインデックス
// xxx using pol_def = int[3]	;	// うまくreadData() で扱えなくって... orz
struct pol_def {
	unsigned int a;	
	unsigned int b;
	unsigned int c;
};


class jhl_rgb {
public:
/*
	unsigned char r;	// 0-255
	unsigned char g;
	unsigned char b;
	*/
	float r;	// 0-255
	float g;
	float b;

	jhl_rgb() {};
//	jhl_rgb(int r_, int g_, int b_) {
	jhl_rgb(float r_, float g_, float b_):
		r( r_ ), g( g_ ), b( b_ )
	{}

	jhl_rgb operator*(float rhs) {
		jhl_rgb t;
		t.r = r * rhs;
		t.g = g * rhs;
		t.b = b * rhs;
		return *this;
	}

	jhl_rgb& operator+=(jhl_rgb rhs) {
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;
		return *this;
	}
};


class dir_light {
public:
	jhl_xyz	dir;
	jhl_rgb col;
};

struct texUv{
	unsigned char u, v;
};


class mat33 {
public:
	float m[3*3];
};

class matHomo4 {
public:
	float m[4 * 4];

	matHomo4() {};
	matHomo4(int i)
		:m{	(float)i, 0, 0, 0 ,
			0, (float)i, 0, 0 ,
			0, 0, (float)i, 0 ,
			0, 0, 0, (float)i }	// ident() の方が早いかもね
	{};
	matHomo4(jhl_xyz a, jhl_xyz b, jhl_xyz c)	// ３つの縦ベクトルから作る
		:m{ 
		a.x, b.x, c.x, 0 ,
		a.y, b.y, c.y, 0 ,
		a.z, b.z, c.z, 0 ,
		0,   0,   0,   0 }
	{};
	matHomo4(jhl_xyz a, jhl_xyz b, jhl_xyz c, jhl_xyz d)	// 4つの縦ベクトルから作る
		:m{ 
		a.x, b.x, c.x, d.x,
		a.y, b.y, c.y, d.y,
		a.z, b.z, c.z, d.z,
		0,   0,   0,   0 }
	{};


	void i() {
		m[0] = m[5] = m[10] = m[15] = 1;
	}

	void ident() {
		std::memset(this, 0, sizeof(matHomo4));
		m[0] = m[5] = m[10] = m[15] = 1;
	}

	// 平行移動
	void trans(float x, float  y, float  z)
	{
		m[3] += x;
		m[4 * 1 + 3] += y;
		m[4 * 2 + 3] += z;
	}

	// 拡大縮小
	void scale(float s) {
/*
		t[0, 0] = s
		t[1, 1] = s
		t[2, 2] = s
		*/
		m[4 * 3 + 3] /= s;	// wだけ
	}

	void scale(float sx, float sy, float sz) {
		m[ 0] *= sx;
		m[ 1 + 4 *1 ] *= sy;
		m[ 2 + 4 *2 ] *= sz;
	}

	void rot_axis_x(float rad) { // a[rad]
		m[4 * 1 + 1] = cos(rad);
		m[4 * 1 + 2] = -sin(rad);
		m[4 * 2 + 1] = sin(rad);
		m[4 * 2 + 2] = cos(rad);
	}

	void rot_axis_y(float rad)
	{
		m[4 * 0 + 0] = cos(rad);
		m[4 * 0 + 2] = sin(rad);
		m[4 * 2 + 0] = -sin(rad);
		m[4 * 2 + 2] = cos(rad);
	}

	void rot_axis_z(float rad)
	{
		m[4 * 0 + 0] = cos(rad);
		m[4 * 0 + 1] = -sin(rad);
		m[4 * 1 + 0] = sin(rad);
		m[4 * 1 + 1] = cos(rad);
	}

	// (r, s, t) ベクトルを軸にs[rad]回転
	// ロドリゲスの公式
	// 単位ベクトルでないと拡縮、回転速度にも掛かる
	void rot_by_vec(float r, float s, float t, float rad)
	{	
		float cs = cos(rad);
		float mcs = 1 - cs;
		float ss = sin(rad);
		float rmcs = r * mcs;
		float smcs = s * mcs;

		m[4 * 0 + 0] = r * rmcs + cs;
		m[4 * 0 + 1] = s * rmcs - t * ss;
		m[4 * 0 + 2] = t * rmcs + s * ss;
		m[4 * 1 + 0] = s * rmcs + t * ss;
		m[4 * 1 + 1] = s * smcs + cs;
		m[4 * 1 + 2] = t * smcs - r * ss;
		m[4 * 2 + 0] = t * rmcs - s * ss;
		m[4 * 2 + 1] = t * smcs + r * ss;
		m[4 * 2 + 2] = t * t * mcs + cs;
	}

	friend std::ostream& operator<< (std::ostream& os, const matHomo4& p)
	{
		os << std::endl;
		os << "[ " << p.m[0] << ", \t" << p.m[1] << ", \t" << p.m[2] << ", \t" << p.m[3] << std::endl;
		os << "  " << p.m[4] << ", \t" << p.m[5] << ", \t" << p.m[6] << ", \t" << p.m[7] << std::endl;
		os << "  " << p.m[8] << ", \t" << p.m[9] << ", \t" << p.m[10] << ", \t" << p.m[11] << std::endl;
		os << "  " << p.m[12] << ", \t" << p.m[13] << ", \t" << p.m[14] << ", \t" << p.m[15] << " ]" << std::endl;
		return os;
	};

	void show() {
		// todo 書き直し
		std::cout << std::endl;
		std::cout << "[ " << m[ 0] << ", \t" << m[ 1] << ", \t" << m[ 2] << ", \t" << m[ 3] << std::endl;
		std::cout << "  " << m[ 4] << ", \t" << m[ 5] << ", \t" << m[ 6] << ", \t" << m[ 7] << std::endl;
		std::cout << "  " << m[ 8] << ", \t" << m[ 9] << ", \t" << m[10] << ", \t" << m[11] << std::endl;
		std::cout << "  " << m[12] << ", \t" << m[13] << ", \t" << m[14] << ", \t" << m[15] << " ]" << std::endl;
	}
};


// xxx using matHomo1 = float[4]; xxx
class matHomo1 {
public:
	float m[4];
};


struct viewport_config {
	float far;
	float near;
	float left, right;
	float top, btm;
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

#if 0
struct polFlat : polAttrib
{
	jhl_rgb	color;
};


struct polTex : polAttrib
{
	char*	texName;
	char*	pTex;
	texUv*	uv;
};
#endif


class modelData {
public:
	int			n_vert;
	jhl_xyz*	vert;	// vert は配列のつもりなのだが、これでいいらしい
	int			n_pol;
	pol_def*	poldef;
	int			n_group;
	grpAttrib*	attr;

	void dataDump( void );
};


// -------------------------

class Basic3dCalc
{
public:
	static modelData*	tgtMdl;
	static jhl_rgb		light_ambient;
	static dir_light	light_directional[2];

	static float		check_side(int pol_idx);
	static jhl_rgb		calc_lighting(int pol_idx);

	static int			setTgtMdl(modelData* t) { tgtMdl = t; };

	static jhl_rgb		set_light_ambient(jhl_rgb l) { light_ambient = l; };
	static dir_light	set_light_directional(int idx, dir_light ld) {
		if (idx < 2) {
			light_directional[idx] = ld;
		}
	}

	static void view_mat(matHomo4& eye_vec, const jhl_xyz& eye_loc, const jhl_xyz& u_vec, const jhl_xyz& tgt_loc);
	static matHomo4 view_mat(const jhl_xyz& eye_loc, const jhl_xyz& u_vec, const jhl_xyz& tgt_loc);


};


