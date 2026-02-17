#pragma once

#include "hal_graphics.h"
#include <windows.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

// cv::Vec3b の代替
struct gdi_color3b {
	BYTE b, g, r;
	gdi_color3b() : b(0), g(0), r(0) {}
	gdi_color3b(BYTE b_, BYTE g_, BYTE r_) : b(b_), g(g_), r(r_) {}
};

// cv::Scalar の代替
struct gdi_scalar {
	double val[4];
	gdi_scalar() { val[0] = val[1] = val[2] = val[3] = 0; }
	gdi_scalar(double v0, double v1, double v2, double v3 = 0) {
		val[0] = v0; val[1] = v1; val[2] = v2; val[3] = v3;
	}
};

int waitKeyGdiplus(int delay_ms);

class disp_gdiplus : public disp_base
{
public:

private:
	static int font_name;
	static int font_size;

	static gdi_scalar	line_color_gdi;
	static gdi_scalar	fill_color_gdi;

	// 画像バッファ (RGB 24bit)
	static BYTE*		img_data;
	static int			img_stride;

	// Z値バッファ (16bit)
	static unsigned short*	img_z;

	// GDI+ 関連
	static ULONG_PTR		gdiplusToken;
	static bool				gdiplusInitialized;
	static Gdiplus::Bitmap*	backBuffer;
	static Gdiplus::Graphics* graphics;
	static Gdiplus::Font*	font;
	static Gdiplus::FontFamily* fontFamily;

	// Win32 ウィンドウ
	static HWND			hWnd_img;
	static HWND			hWnd_z;
	static HINSTANCE	hInstance;
	static bool			windowClassRegistered;

	static void RegisterWindowClass();
	static HWND CreateDisplayWindow(const wchar_t* title, int width, int height, int x, int y);
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

public:
	void	disp_init(jhl_xy_i window_size_);
	void	disp_show();
	void	disp_clear();
	void	disp_swap();
	void	disp_destroy();

	void	set_lineColor(jhl_rgb c) {
		line_color = c;
		line_color_gdi = gdi_scalar(c.b * 255, c.g * 255, c.r * 255);
	}
	void	set_fillColor(jhl_rgb c) {
		fill_color = c;
		fill_color_gdi = gdi_scalar(c.b * 255, c.g * 255, c.r * 255);
	}

	void	point(jhl_xy_i& pos, int size = 3);
	void	point(jhl_xy_i& pos, jhl_rgb c);
	void	point(jhl_xy_i& pos, gdi_scalar color);
	void	point(jhl_xy_i& pos, gdi_scalar* color);
	void	point(jhl_xy_i& pos, gdi_color3b* c);
	void	line(jhl_xy_i& start, jhl_xy_i& end);
	void	line_f(jhl_xy_i& start, jhl_xy_i& end);
	void	line_h(unsigned int v, float h_start, float h_end);

	void	point_z(jhl_xy_i& pos, float depth);
	bool	point_z_test(jhl_xy_i& pos, float depth);
	void	point_z_set(jhl_xy_i& pos, float depth);
	void	point_with_z(jhl_xy_i& pos, float depth);

	void	circle(jhl_xy_i& pos, int r, jhl_rgb& color, int thickness = 1);
	void	rectangle(jhl_xy_i& start, jhl_xy_i& end, jhl_rgb& color, int thickness = 1);

	void	fontInit()
	{
		font_name = 0;
		font_size = 1;
	}
	void	putText(const char* s, jhl_xy_i& pos, jhl_rgb& color);

};
