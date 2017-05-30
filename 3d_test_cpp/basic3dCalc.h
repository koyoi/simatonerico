#pragma once
#include <cmath>
#include <cstring>
#include <iostream>	// << オペレータ

// オペレータオーバーロードについて
// http://www.hiramine.com/programming/c_cpp/operator.html



class jhl_xy_i {
public:
	unsigned int x;
	unsigned int y;

public:
	jhl_xy_i() {};
	jhl_xy_i(int x_, int y_) :
		x(x_), y(y_)
	{};

	jhl_xy_i operator/(int div) const {
		jhl_xy_i t;
		t.x = x / div;
		t.y = y / div;
		return t;
	};
};


class jhl_xyz {
public:
	float x;	// 座標
	float y;
	float z;

	jhl_xyz() {};
	jhl_xyz(float x_, float y_, float z_) :
		x(x_), y(y_), z(z_)
	{}
	jhl_xyz(float i) :
		x(i), y(i), z(i)
	{}


	jhl_xyz& operator+=(jhl_xyz rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	jhl_xyz operator-(const jhl_xyz& rhs) const {
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

	jhl_xyz& operator*=(float rhs) {
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}

	jhl_xyz& operator*=(jhl_xyz rhs) {
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		return *this;
	}

	/*	// todo 外積と混乱しそうなので dot とかにする
	jhl_xyz& operator*=(jhl_xyz rhs) {
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		return *this;
	}
*/

	jhl_xyz operator/(float rhs) {
		jhl_xyz t;
		t.x = x / rhs;
		t.y = y / rhs;
		t.z = z / rhs;
		return t;
	}

	jhl_xyz& operator/=(float rhs) {
		x /= rhs;
		y /= rhs;
		z /= rhs;
		return *this;
	}

	bool operator==(jhl_xyz rhs) {
		return(x == rhs.x &&
			y == rhs.y &&
			z == rhs.z) ? true: false;
	}

	// ベクトルならでは
	// ノルム
	float norm() {
		return sqrtf(x * x + y * y + z * z);
	}

	jhl_xyz normalize() {
		return( jhl_xyz(x,y,z) / norm() );
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

	operator jhl_xy_i() {
		jhl_xy_i t((int)x,(int)y);
		return t;
	}

/*
	operator matHomo1() {
		matHomo1 t;
		t.m[0] = x;
		t.m[1] = y;
		t.m[2] = z;
		t.m[3] = 1;
		return t;
	}
*/

	friend std::ostream& operator<< (std::ostream& os, const jhl_xyz& p)
	{
		os << "[ " << p.x << ", \t" << p.y << ", \t" << p.z << "]";
		return os;
	};
};


// 座標
class jhl_xy {
public:
	float x;
	float y;

public:
	jhl_xy() {};
	jhl_xy(float _x, float _y) :
		x(_x),
		y(_y)
	{};
	jhl_xy(jhl_xyz _v) :
		x(_v.x),
		y(_v.y)
	{};
};


class jhl_size {
public:
	// float sx,sy,sz,sw
	float s[4];

public:
	jhl_size() 
	{}

	jhl_size(float s_) :
		s{ s_, s_, s_, 1 }
		//s{ 1,1,1, 1/s_ }
	{}

	jhl_size( float sx, float sy, float sz ):
		s{ sx, sy, sz, 1 }
	{}
};


class matHomo1 {
public:
	float m[4];

public:
	matHomo1()
	{}

	matHomo1(jhl_xyz t_)
	{
		m[0] = t_.x;
		m[1] = t_.y;
		m[2] = t_.z;
		m[3] = 1.0f;
	}


	operator jhl_xyz() {
		jhl_xyz t;
		t.x = m[0] / m[3];
		t.y = m[1] / m[3];
		t.z = m[2] / m[3];
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
	float r;	// 0-1
	float g;
	float b;

	jhl_rgb() {};
	jhl_rgb(float r_, float g_, float b_) :
		r(r_), g(g_), b(b_)
	{}

	jhl_rgb(int r_, int g_, int b_) :
		r((float)r_ / 255), g((float)g_ / 255), b((float)b_ / 255)
	{}

	jhl_rgb(int c[3]) :
		r((float)c[0] / 255), g((float)c[1] / 255), b((float)c[2]/ 255)
	{}

	jhl_rgb(float a_) :
		r(a_), g(a_), b(a_)
	{}

	jhl_rgb operator*(const float rhs) const {
		jhl_rgb t;
		t.r = r * rhs;
		t.g = g * rhs;
		t.b = b * rhs;
		return t;
	}

	jhl_rgb operator*(const jhl_rgb rhs) const {
		jhl_rgb t = *this;
		t.r *= rhs.r;
		t.g *= rhs.g;
		t.b *= rhs.b;
		return t;
	}

	jhl_rgb& operator+=(const jhl_rgb& rhs) {
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;
		return *this;
	}
};

// 方向ありライト（現状では平行光源）
class dir_light {
public:
	jhl_xyz	dir;
	jhl_rgb col;
};



struct texUv_{
	unsigned char u, v;
};

struct texUv {
	texUv_ d[3];
};

class mat33 {
public:
	float m[3*3];
};


class matHomo4_full;
class matHomo4;

// 3Dグラフィックスで使わない成分を省略した省メモリ・高速版（？）
class matHomo4 {
public:
	float m[3 * 3];
	float v[4];

	matHomo4() {};
	matHomo4(int i)
		:m{ (float)i, 0, 0,
		0, (float)i, 0,
		0, 0, (float)i },
		v{ 0, 0, 0, (float)i }	// ident() の方が早いかもね
	{};
	/*
	matHomo4(float a, float b, float c)
		:v{ a, b, c, 1 },
		m{ (float)1, 0, 0,
		0, (float)1, 0,
		0, 0, (float)1 }
	{};
	*/
	matHomo4(jhl_xyz pos)
		:v{ pos.x, pos.y, pos.z, 1 },
		m{ (float)1, 0, 0,
		0, (float)1, 0,
		0, 0, (float)1 }
	{};
	matHomo4(jhl_xyz a, jhl_xyz b, jhl_xyz c)	// ３つの縦ベクトルから作る。視線ベクトルくらい
		:m{
		a.x, b.x, c.x,
		a.y, b.y, c.y,
		a.z, b.z, c.z},
		v{0}
	{};
	matHomo4(jhl_xyz a, jhl_xyz b, jhl_xyz c, jhl_xyz d)	// 4つの縦ベクトルから作る。ビューマトリクス専用
		:m{
		a.x, b.x, c.x,
		a.y, b.y, c.y,
		a.z, b.z, c.z },
		v{ d.x, d.y, d.z, 1.f }
	{};
	matHomo4(jhl_size s_)
		:v{ 0,0,0, 1 },
		m{ s_.s[0], 0, 0,
		0, s_.s[1], 0,
		0, 0, s_.s[2] }
	{};

	void i() {
		m[0] = m[4] = m[8] = v[3] = 1;
	}

	void ident() {	// 初期化目的
		std::memset(this->m, 0, sizeof(this->m));
		std::memset(this->v, 0, sizeof(this->v));
		i();
	}

	// 平行移動
	void trans(float x, float y, float  z)
	{
		v[0] += x;
		v[1] += y;
		v[2] += z;
	}

	void trans_set(float x, float y, float  z)
	{
		v[0] = x;
		v[1] = y;
		v[2] = z;
	}

	// 拡大縮小
	void scale(float s) {
		v[3] /= s;	// wだけ
	}

	void scale(float sx, float sy, float sz) {
		m[0] *= sx;
		m[3 * 1 + 1] *= sy;
		m[3 * 2 + 2] *= sz;
	}

	void rot_axis_x(float rad) {
		matHomo4 r(1);
		r.m[3 * 1 + 1] = cos(rad);
		r.m[3 * 1 + 2] = -sin(rad);
		r.m[3 * 2 + 1] = sin(rad);
		r.m[3 * 2 + 2] = cos(rad);
		*this = r * (*this);
	}

	void rot_axis_y(float rad)
	{
		matHomo4 r(1);
		r.m[3 * 0 + 0] = cos(rad);
		r.m[3 * 0 + 2] = sin(rad);
		r.m[3 * 2 + 0] = -sin(rad);
		r.m[3 * 2 + 2] = cos(rad);
		*this = r * (*this);
	}

	void rot_axis_z(float rad)
	{
		matHomo4 r(1);
		r.m[3 * 0 + 0] = cos(rad);
		r.m[3 * 0 + 1] = -sin(rad);
		r.m[3 * 1 + 0] = sin(rad);
		r.m[3 * 1 + 1] = cos(rad);
		*this = r * (*this);
	}

	// (r, s, t) ベクトルを軸にs[rad]回転
	// ロドリゲスの公式
	// 単位ベクトルでないと拡縮、回転速度にも掛かる
	void rot_by_vec(float r, float s, float t, float rad)
	{
		matHomo4 tmp(1);

		float cs = cos(rad);
		float mcs = 1 - cs;
		float ss = sin(rad);
		float rmcs = r * mcs;
		float smcs = s * mcs;

		tmp.m[3 * 0 + 0] = r * rmcs + cs;
		tmp.m[3 * 0 + 1] = s * rmcs - t * ss;
		tmp.m[3 * 0 + 2] = t * rmcs + s * ss;
		tmp.m[3 * 1 + 0] = s * rmcs + t * ss;
		tmp.m[3 * 1 + 1] = s * smcs + cs;
		tmp.m[3 * 1 + 2] = t * smcs - r * ss;
		tmp.m[3 * 2 + 0] = t * rmcs - s * ss;
		tmp.m[3 * 2 + 1] = t * smcs + r * ss;
		tmp.m[3 * 2 + 2] = t * t * mcs + cs;

		*this = tmp * (*this);
	}

	void rot_by_vec(jhl_xyz v, float rad)
	{
		rot_by_vec(v.x, v.y, v.z, rad);
	}

	matHomo4& operator+=(matHomo4 rhs) {
		for( int i = 0; i<sizeof(m); i++)
		{
			m[i] += rhs.m[i];
		}
		return *this;
	}

	// 外積
	matHomo4 operator*(matHomo4 rhs) const {	
		/* maxima code
	m3:matrix([m_0,m_1,m_2,v_0],[m_3,m_4,m_5,v_1],[m_6,m_7,m_8,v_2],[0,0,0,v_3]);
	m4:matrix([rhs_m0,rhs_m1,rhs_m2,rhs_v0],[rhs_m3,rhs_m4,rhs_m5,rhs_v1],[rhs_m6,rhs_m7,rhs_m8,rhs_v2],[0,0,0,rhs_v3]);
	m3.m4;
	*/
        matHomo4 t;
		t.m[0] = m[0] * rhs.m[0] + m[1] * rhs.m[3] + m[2] * rhs.m[6];
		t.m[1] = m[0] * rhs.m[1] + m[1] * rhs.m[4] + m[2] * rhs.m[7];
		t.m[2] = m[0] * rhs.m[2] + m[1] * rhs.m[5] + m[2] * rhs.m[8];
		t.v[0] = m[0] * rhs.v[0] + m[1] * rhs.v[1] + m[2] * rhs.v[2] + rhs.v[3] * v[0];
		t.m[3] = m[3] * rhs.m[0] + m[4] * rhs.m[3] + m[5] * rhs.m[6];
		t.m[4] = m[3] * rhs.m[1] + m[4] * rhs.m[4] + m[5] * rhs.m[7];
		t.m[5] = m[3] * rhs.m[2] + m[4] * rhs.m[5] + m[5] * rhs.m[8];
		t.v[1] = m[3] * rhs.v[0] + m[4] * rhs.v[1] + m[5] * rhs.v[2] + rhs.v[3] * v[1];
		t.m[6] = m[6] * rhs.m[0] + m[7] * rhs.m[3] + m[8] * rhs.m[6];
		t.m[7] = m[6] * rhs.m[1] + m[7] * rhs.m[4] + m[8] * rhs.m[7];
		t.m[8] = m[6] * rhs.m[2] + m[7] * rhs.m[5] + m[8] * rhs.m[8];
		t.v[2] = m[6] * rhs.v[0] + m[7] * rhs.v[1] + m[8] * rhs.v[2] + rhs.v[3] * v[2];
		t.v[3] = rhs.v[3] * v[3];
		return t;
	}

	matHomo4& operator*=(matHomo4 rhs) {
		matHomo4 t;
		t.m[0] = m[0] * rhs.m[0] + m[1] * rhs.m[3] + m[2] * rhs.m[6];
		t.m[1] = m[0] * rhs.m[1] + m[1] * rhs.m[4] + m[2] * rhs.m[7];
		t.m[2] = m[0] * rhs.m[2] + m[1] * rhs.m[5] + m[2] * rhs.m[8];
		t.v[0] = m[0] * rhs.v[0] + m[1] * rhs.v[1] + m[2] * rhs.v[2] + rhs.v[3] * v[0];
		t.m[3] = m[3] * rhs.m[0] + m[4] * rhs.m[3] + m[5] * rhs.m[6];
		t.m[4] = m[3] * rhs.m[1] + m[4] * rhs.m[4] + m[5] * rhs.m[7];
		t.m[5] = m[3] * rhs.m[2] + m[4] * rhs.m[5] + m[5] * rhs.m[8];
		t.v[1] = m[3] * rhs.v[0] + m[4] * rhs.v[1] + m[5] * rhs.v[2] + rhs.v[3] * v[1];
		t.m[6] = m[6] * rhs.m[0] + m[7] * rhs.m[3] + m[8] * rhs.m[6];
		t.m[7] = m[6] * rhs.m[1] + m[7] * rhs.m[4] + m[8] * rhs.m[7];
		t.m[8] = m[6] * rhs.m[2] + m[7] * rhs.m[5] + m[8] * rhs.m[8];
		t.v[2] = m[6] * rhs.v[0] + m[7] * rhs.v[1] + m[8] * rhs.v[2] + rhs.v[3] * v[2];
		t.v[3] = rhs.v[3] * v[3];
		*this = t;
		return *this;
	}

	jhl_xyz operator*(jhl_xyz rhs) const {
		jhl_xyz rv;

		rv.x = (m[0] * rhs.x + m[1] * rhs.y + m[2] * rhs.z + v[0]);
		rv.y = (m[3] * rhs.x + m[4] * rhs.y + m[5] * rhs.z + v[1]);
		rv.z = (m[6] * rhs.x + m[7] * rhs.y + m[8] * rhs.z + v[2]);
		return rv;
	}


	matHomo4 operator+(matHomo4 rhs) const {
		matHomo4 t;
		for (int i = 0; i < 4; i++)
		{
			t.m[i] = m[i] + rhs.m[i];
			t.v[i] = v[i] + rhs.v[i];
		}
		for (int i = 4; i < 9; i++)
		{
			t.m[i] = m[i] + rhs.m[i];
		}
		return t;
	}

#if 0
	operator matHomo4_full() const {	// キャスト演算子
		/*
		//m3:matrix([m_0,m_1,m_2,v_0],[m_3,m_4,m_5,v_1],[m_6,m_7,m_8,v_2],[0,0,0,v_3]);
		//m4:matrix([rhs_m0,rhs_m1,rhs_m2,rhs_v0],[rhs_m3,rhs_m4,rhs_m5,rhs_v1],[rhs_m6,rhs_m7,rhs_m8,rhs_v2],[0,0,0,rhs_v3]);
		matHomo4_full t;
		t.m[0] = v[0] * rhs.m[12] + m[2] * rhs.m[8] + m[1] * rhs.m[4] + m[0] * rhs.m[0];
		t.m[1] = v[0] * rhs.m[13] + m[2] * rhs.m[9] + m[1] * rhs.m[5] + m[0] * rhs.m[1];
		t.m[2] = v[0] * rhs.m[14] + m[1] * rhs.m[6] + m[0] * rhs.m[2] + m[2] * rhs.m[10];
		t.m[3] = v[0] * rhs.m[15] + m[1] * rhs.m[7] + m[0] * rhs.m[3] + m[2] * rhs.m[11];
		t.m[4] = v[1] * rhs.m[12] + m[5] * rhs.m[8] + m[4] * rhs.m[4] + m[3] * rhs.m[0];
		t.m[5] = v[1] * rhs.m[13] + m[5] * rhs.m[9] + m[4] * rhs.m[5] + m[3] * rhs.m[1];
		t.m[6] = v[1] * rhs.m[14] + m[4] * rhs.m[6] + m[3] * rhs.m[2] + m[5] * rhs.m[10];
		t.m[7] = v[1] * rhs.m[15] + m[4] * rhs.m[7] + m[3] * rhs.m[3] + m[5] * rhs.m[11];
		t.m[8] = v[2] * rhs.m[12] + m[8] * rhs.m[8] + m[7] * rhs.m[4] + m[6] * rhs.m[0];
		t.m[9] = v[2] * rhs.m[13] + m[8] * rhs.m[9] + m[7] * rhs.m[5] + m[6] * rhs.m[1];
		t.m[10] = v[2] * rhs.m[14] + m[7] * rhs.m[6] + m[6] * rhs.m[2] + m[8] * rhs.m[10];
		t.m[11] = v[2] * rhs.m[15] + m[7] * rhs.m[7] + m[6] * rhs.m[3] + m[8] * rhs.m[11];
		t.m[12] = v[3] * rhs.m[12];
		t.m[13] = v[3] * rhs.m[13];
		t.m[14] = v[3] * rhs.m[14];
		t.m[15] = v[3] * rhs.m[15];
		return t;
		*/
		matHomo4_full t;
		t.m[0] = m[0];
		t.m[1] = m[1];
		t.m[2] = m[2];
		t.m[4] = m[3];
		t.m[5] = m[4];
		t.m[6] = m[5];
		t.m[8] = m[6];
		t.m[9] = m[7];
		t.m[10] = m[8];
		t.m[3] = v[0];
		t.m[7] = v[1];
		t.m[11] = v[2];
		t.m[15] = v[3];
		t.m[12] = 0;
		t.m[13] = 0;
		t.m[14] = 0;
		return t;
	}
#endif

	friend std::ostream& operator<< (std::ostream& os, const matHomo4& p)
	{
		os << "[ " << p.m[0] << ", \t" << p.m[1] << ", \t" << p.m[2] << ", \t" << p.v[0] << std::endl;
		os << "  " << p.m[3] << ", \t" << p.m[4] << ", \t" << p.m[5] << ", \t" << p.v[1] << std::endl;
		os << "  " << p.m[6] << ", \t" << p.m[7] << ", \t" << p.m[8] << ", \t" << p.v[2] << std::endl;
		os << "  " << "0"    << ", \t" << "0"    << ", \t" << "0"    << ", \t" << p.v[3] << " ]" << std::endl;
		return os;
	};

	void show() {
		std::cout << std::endl;
		std::cout << "[ " << m[0] << ", \t" << m[1] << ", \t" << m[2] << ", \t" << v[0] << std::endl;
		std::cout << "  " << m[3] << ", \t" << m[4] << ", \t" << m[5] << ", \t" << v[1] << std::endl;
		std::cout << "  " << m[6] << ", \t" << m[7] << ", \t" << m[8] << ", \t" << v[2] << std::endl;
		std::cout << "  " << "0"  << ", \t" << "0"  << ", \t" << "0"  << ", \t" << v[3] << " ]" << std::endl;
	}
}; 

class matHomo4_full {
public:
	float m[4 * 4];
	matHomo4_full() :
		m{ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }
	{}

	// 外積
	matHomo4_full operator*(matHomo4 rhs) const {
		/* maxima code
		m3:matrix([m_0,m_1,m_2,v_0],[m_3,m_4,m_5,v_1],[m_6,m_7,m_8,v_2],[0,0,0,v_3]);
		m4:matrix([rhs_m0,rhs_m1,rhs_m2,rhs_m3],[rhs_m4,rhs_m5,rhs_m6,rhs_m7],[rhs_m8,rhs_m9,rhs_m10,rhs_m11],[rhs_m12,rhs_m13,rhs_m14,rhs_m15]);
		m4.m3;
		*/
		matHomo4_full t;
		t.m[0] = m[2] * rhs.m[6] + m[1] * rhs.m[3] + m[0] * rhs.m[0];
		t.m[1] = m[2] * rhs.m[7] + m[1] * rhs.m[4] + m[0] * rhs.m[1];
		t.m[2] = m[2] * rhs.m[8] + m[1] * rhs.m[5] + m[0] * rhs.m[2];
		t.m[3] = m[3] * rhs.v[3] + m[2] * rhs.v[2] + m[1] * rhs.v[1] + m[0] * rhs.v[0];
		t.m[4] = m[6] * rhs.m[6] + m[5] * rhs.m[3] + m[4] * rhs.m[0];
		t.m[5] = m[6] * rhs.m[7] + m[5] * rhs.m[4] + m[4] * rhs.m[1];
		t.m[6] = m[6] * rhs.m[8] + m[5] * rhs.m[5] + m[4] * rhs.m[2];
		t.m[7] = m[7] * rhs.v[3] + m[6] * rhs.v[2] + m[5] * rhs.v[1] + m[4] * rhs.v[0];
		t.m[8] = m[10] * rhs.m[6] + m[9] * rhs.m[3] + m[8] * rhs.m[0];
		t.m[9] = m[10] * rhs.m[7] + m[9] * rhs.m[4] + m[8] * rhs.m[1];
		t.m[10] = m[10] * rhs.m[8] + m[9] * rhs.m[5] + m[8] * rhs.m[2];
		t.m[11] = m[11] * rhs.v[3] + m[10] * rhs.v[2] + m[9] * rhs.v[1] + m[8] * rhs.v[0];
		t.m[12] = m[14] * rhs.m[6] + m[13] * rhs.m[3] + m[12] * rhs.m[0];
		t.m[13] = m[14] * rhs.m[7] + m[13] * rhs.m[4] + m[12] * rhs.m[1];
		t.m[14] = m[14] * rhs.m[8] + m[13] * rhs.m[5] + m[12] * rhs.m[2];
		t.m[15] = m[15] * rhs.v[3] + m[14] * rhs.v[2] + m[13] * rhs.v[1] + m[12] * rhs.v[0];
		return t;
	}

	matHomo4_full& operator*=(matHomo4 rhs) {
		matHomo4_full t;
		t.m[0] = m[2] * rhs.m[6] + m[1] * rhs.m[3] + m[0] * rhs.m[0];
		t.m[1] = m[2] * rhs.m[7] + m[1] * rhs.m[4] + m[0] * rhs.m[1];
		t.m[2] = m[2] * rhs.m[8] + m[1] * rhs.m[5] + m[0] * rhs.m[2];
		t.m[3] = m[3] * rhs.v[3] + m[2] * rhs.v[2] + m[1] * rhs.v[1] + m[0] * rhs.v[0];
		t.m[4] = m[6] * rhs.m[6] + m[5] * rhs.m[3] + m[4] * rhs.m[0];
		t.m[5] = m[6] * rhs.m[7] + m[5] * rhs.m[4] + m[4] * rhs.m[1];
		t.m[6] = m[6] * rhs.m[8] + m[5] * rhs.m[5] + m[4] * rhs.m[2];
		t.m[7] = m[7] * rhs.v[3] + m[6] * rhs.v[2] + m[5] * rhs.v[1] + m[4] * rhs.v[0];
		t.m[8] = m[10] * rhs.m[6] + m[9] * rhs.m[3] + m[8] * rhs.m[0];
		t.m[9] = m[10] * rhs.m[7] + m[9] * rhs.m[4] + m[8] * rhs.m[1];
		t.m[10] = m[10] * rhs.m[8] + m[9] * rhs.m[5] + m[8] * rhs.m[2];
		t.m[11] = m[11] * rhs.v[3] + m[10] * rhs.v[2] + m[9] * rhs.v[1] + m[8] * rhs.v[0];
		t.m[12] = m[14] * rhs.m[6] + m[13] * rhs.m[3] + m[12] * rhs.m[0];
		t.m[13] = m[14] * rhs.m[7] + m[13] * rhs.m[4] + m[12] * rhs.m[1];
		t.m[14] = m[14] * rhs.m[8] + m[13] * rhs.m[5] + m[12] * rhs.m[2];
		t.m[15] = m[15] * rhs.v[3] + m[14] * rhs.v[2] + m[13] * rhs.v[1] + m[12] * rhs.v[0];
		*this = t;
		return *this;
	}

	matHomo1 operator*(matHomo1 rhs) const {
		matHomo1 t;
		t.m[0] = m[0] * rhs.m[0] + m[1] * rhs.m[1] + m[2] * rhs.m[2] + m[3] * rhs.m[3];
		t.m[1] = m[4] * rhs.m[0] + m[5] * rhs.m[1] + m[6] * rhs.m[2] + m[7] * rhs.m[3];
		t.m[2] = m[8] * rhs.m[0] + m[9] * rhs.m[1] + m[10] * rhs.m[2] + m[11] * rhs.m[3];
		t.m[3] = m[12] * rhs.m[0] + m[13] * rhs.m[1] + m[14] * rhs.m[2] + m[15] * rhs.m[3];
		return t;
	}


	// 同次行列からxyzに
	jhl_xyz operator*(jhl_xyz rhs) const {
		jhl_xyz rv;
		float w;
		matHomo1 t(rhs);

		w = m[12] * t.m[0] + m[13] * t.m[1] + m[14] * t.m[2] + m[15] * t.m[3];			// w で割る

		rv.x = (m[0] * t.m[0] + m[1] * t.m[1] + m[2] * t.m[2] + m[3] * t.m[3]) / w;
		rv.y = (m[4] * t.m[0] + m[5] * t.m[1] + m[6] * t.m[2] + m[7] * t.m[3]) / w;
		rv.z = (m[8] * t.m[0] + m[9] * t.m[1] + m[10] * t.m[2] + m[11] * t.m[3]) / w;
		return rv;
	}

	friend std::ostream& operator<< (std::ostream& os, const matHomo4_full& p)
	{
		os << "[ " << p.m[0] << ", \t" << p.m[1] << ", \t" << p.m[2] << ", \t" << p.m[3] << std::endl;
		os << "  " << p.m[4] << ", \t" << p.m[5] << ", \t" << p.m[6] << ", \t" << p.m[7] << std::endl;
		os << "  " << p.m[8] << ", \t" << p.m[9] << ", \t" << p.m[10] << ", \t" << p.m[11] << std::endl;
		os << "  " << p.m[12] << ", \t" << p.m[13] << ", \t" << p.m[14] << ", \t" << p.m[15] << " ]" << std::endl;
		return os;
	};

	void show() {
		std::cout << "[ " << m[ 0] << ", \t" << m[ 1] << ", \t" << m[ 2] << ", \t" << m[ 3] << std::endl;
		std::cout << "  " << m[ 4] << ", \t" << m[ 5] << ", \t" << m[ 6] << ", \t" << m[ 7] << std::endl;
		std::cout << "  " << m[ 8] << ", \t" << m[ 9] << ", \t" << m[10] << ", \t" << m[11] << std::endl;
		std::cout << "  " << m[12] << ", \t" << m[13] << ", \t" << m[14] << ", \t" << m[15] << " ]" << std::endl;
	}
};



void rigid_trans(matHomo4* trans_mat, const jhl_xyz& pos, const matHomo4& r, const jhl_size& size);


