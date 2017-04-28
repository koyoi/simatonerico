#include "stdafx.h"

#include <iostream>
#include <cstring>

#include "basic3dCalc.h"
#include <cmath>

void rigid_trans(matHomo4* trans_mat, const jhl_xyz& pos, const matHomo4& r, const jhl_size& size)
{
	/*
 maxima code
 pos:matrix([1,0,0,x],[0,1,0,y],[0,0,1,z],[0,0,0,1]);
 r:matrix([m0,m1,m2,v0],[m3,m4,m5,v1],[m6,m7,m8,v2],[0,0,0,1]);
 size:matrix([sx,0,0,0],[0,sy,0,0],[0,0,sz,0],[0,0,0,sw]);
 pos.(r.size);
 */
	trans_mat->m[0]         = r.m[0 * 3 + 0] * size.s[0];
	trans_mat->m[1 * 3 + 0] = r.m[1 * 3 + 0] * size.s[0];
	trans_mat->m[2 * 3 + 0] = r.m[2 * 3 + 0] * size.s[0];
	trans_mat->m[0 * 3 + 1] = r.m[0 * 3 + 1] * size.s[1];
	trans_mat->m[1 * 3 + 1] = r.m[1 * 3 + 1] * size.s[1];
	trans_mat->m[2 * 3 + 1] = r.m[2 * 3 + 1] * size.s[1];
	trans_mat->m[0 * 3 + 2] = r.m[0 * 3 + 2] * size.s[2];
	trans_mat->m[1 * 3 + 2] = r.m[1 * 3 + 2] * size.s[2];
	trans_mat->m[2 * 3 + 2] = r.m[2 * 3 + 2] * size.s[2];
	trans_mat->v[0] = (r.v[0] + pos.x * r.v[3]) * size.s[3];
	trans_mat->v[1] = (r.v[1] + pos.y * r.v[3]) * size.s[3];
	trans_mat->v[2] = (r.v[2] + pos.z * r.v[3]) * size.s[3];
	trans_mat->v[3] = r.v[3] * size.s[3];
}
