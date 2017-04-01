#pragma once
#include <cmath>

struct jhl_xyz {
	double x;
	double y;
	double z;
};

// using pol_def = int[3]	;	// ‚¤‚Ü‚­readData() ‚Åˆµ‚¦‚È‚­‚Á‚Ä... orz
struct pol_def {
	double a;
	double b;
	double c;
};

struct jhl_rgb {
	double r;
	double g;
	double b;
};

struct dir_light {
	jhl_xyz	d;
	jhl_rgb c;
};


using matHomo4 = double[4 * 4];
using matHomo1 = double[4];


struct viewport_config {
	double far;
	double near;
	double left, right;
	double top, btm;
};

enum En_draw_type {
	drawType_vertex,
	drawType_line,
	drawType_line_front_face,
	drawType_flat
};

class Basic3dCalc
{
private:

public:
	static bool Basic3dCalc::check_side(int pol_idx);

};

