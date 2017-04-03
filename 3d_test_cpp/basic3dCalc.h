#pragma once
#include <cmath>


class jhl_xyz {
public:
	float x;	// 座標
	float y;
	float z;

	float norm(jhl_xyz n) {
		return sqrtf(n.x * n.x + n.y + n.y + n.z + n.z);
	}

	float dot(jhl_xyz b) {
		return x * b.x + y * b.y + z * b.z;
	}

	jhl_xyz operator*(jhl_xyz rhs) {
		jhl_xyz t;
		t.x = y * rhs.z - z * rhs.y;
		t.y = z * rhs.x - x * rhs.z;
		t.z = x * rhs.y - y * rhs.x;
		return t;
	}

	jhl_xyz operator-(jhl_xyz rhs) {
		jhl_xyz t;
		t.x = rhs.x - x;
		t.y = rhs.y - y;
		t.z = rhs.z - z;
		return t;
	}

	jhl_xyz operator/( float rhs ) {
		x /= rhs;
		y /= rhs; 
		z /= rhs;
		return *this;
	}

	jhl_xyz& operator+=(jhl_xyz rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}
};

// 頂点へのインデックス
// using pol_def = int[3]	;	// うまくreadData() で扱えなくって... orz
struct pol_def {
	unsigned int a;	
	unsigned int b;
	unsigned int c;
};


class jhl_rgb {
public:
	unsigned char r;	// 0-255
	unsigned char g;
	unsigned char b;

	jhl_rgb() {};
	jhl_rgb(int r_, int g_, int b_) {
		r = r_;
		g = g_;
		b = b_;
	}

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


struct dir_light {
	jhl_xyz	dir;
	jhl_rgb col;
};

struct texUv{
	unsigned char u, v;
};


using matHomo4 = float[4 * 4];
using matHomo1 = float[4];


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


struct polAttrib
{
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
	pol_def*	poldef;	// vert は配列のつもりなのだが、これでいいらしい
	polAttrib*	attr;
};


// -------------------------

class Basic3dCalc
{
public:
	static modelData	tgtMdl;
	static jhl_rgb		light_ambient;
	static dir_light	light_directional[2];

	static float		check_side(int pol_idx);
	static jhl_rgb		calc_lighting(int pol_idx);

	static int			setTgtMdl(modelData& t) { tgtMdl = t; };

	static jhl_rgb		set_light_ambient(jhl_rgb l) { light_ambient = l; };
	static dir_light	set_light_directional(int idx, dir_light ld) {
		if (idx < 2) {
			light_directional[idx] = ld;
		}
	}


	static void Basic3dCalc::ident_matHomo4(matHomo4&);	


};


