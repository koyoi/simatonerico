#include "stdafx.h"

#include "hal_gfx_gdiplus.h"

#include <assert.h>

// 静的メンバ初期化
BYTE*			disp_gdiplus::img_data = nullptr;
int				disp_gdiplus::img_stride = 0;
unsigned short*	disp_gdiplus::img_z = nullptr;

int disp_gdiplus::font_name = 0;
int disp_gdiplus::font_size = 1;
gdi_scalar	disp_gdiplus::line_color_gdi;
gdi_scalar	disp_gdiplus::fill_color_gdi;

ULONG_PTR		disp_gdiplus::gdiplusToken = 0;
bool			disp_gdiplus::gdiplusInitialized = false;
Gdiplus::Bitmap*	disp_gdiplus::backBuffer = nullptr;
Gdiplus::Graphics*	disp_gdiplus::graphics = nullptr;
Gdiplus::Font*		disp_gdiplus::font = nullptr;
Gdiplus::FontFamily* disp_gdiplus::fontFamily = nullptr;

HWND			disp_gdiplus::hWnd_img = NULL;
HWND			disp_gdiplus::hWnd_z = NULL;
HINSTANCE		disp_gdiplus::hInstance = NULL;
bool			disp_gdiplus::windowClassRegistered = false;

// グローバルキー状態
static int g_lastKey = -1;

void disp_gdiplus::RegisterWindowClass()
{
	if (windowClassRegistered) return;

	hInstance = GetModuleHandle(NULL);

	WNDCLASSEXW wc = {};
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = L"GdiplusDisplayWindow";

	RegisterClassExW(&wc);
	windowClassRegistered = true;
}

HWND disp_gdiplus::CreateDisplayWindow(const wchar_t* title, int width, int height, int x, int y)
{
	RECT rect = { 0, 0, width, height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	return CreateWindowExW(
		0,
		L"GdiplusDisplayWindow",
		title,
		WS_OVERLAPPEDWINDOW,
		x, y,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL, NULL, hInstance, NULL
	);
}

LRESULT CALLBACK disp_gdiplus::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		if (hWnd == hWnd_img && backBuffer) {
			Gdiplus::Graphics g(hdc);
			g.DrawImage(backBuffer, 0, 0);
		}
		else if (hWnd == hWnd_z && img_z) {
			// Z値バッファの可視化
			int w = window_size.y;
			int h = window_size.x;

			BITMAPINFO bmi = {};
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = w;
			bmi.bmiHeader.biHeight = -h;  // トップダウン
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 24;
			bmi.bmiHeader.biCompression = BI_RGB;

			// Z値をグレースケールに変換
			BYTE* zVisData = new BYTE[w * h * 3];
			for (int i = 0; i < w * h; i++) {
				BYTE gray = (BYTE)(img_z[i] >> 8);
				zVisData[i * 3 + 0] = gray;
				zVisData[i * 3 + 1] = gray;
				zVisData[i * 3 + 2] = gray;
			}

			SetDIBitsToDevice(hdc, 0, 0, w, h, 0, 0, 0, h, zVisData, &bmi, DIB_RGB_COLORS);
			delete[] zVisData;
		}

		EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_KEYDOWN:
		g_lastKey = (int)wParam;
		return 0;

	case WM_CHAR:
		// WM_CHAR gives actual character code after translation
		g_lastKey = (int)wParam;
		return 0;

	case WM_CLOSE:
		// ウィンドウを非表示にするだけで破棄しない
		ShowWindow(hWnd, SW_HIDE);
		return 0;

	case WM_DESTROY:
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}


void disp_gdiplus::disp_init(jhl_xy_i window_size_)
{
	window_size.x = window_size_.y;
	window_size.y = window_size_.x;	// row, column と x,y はこれから揃えておくべきなのだ

	// GDI+ 初期化
	if (!gdiplusInitialized) {
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		gdiplusInitialized = true;
	}

	// ウィンドウクラス登録
	RegisterWindowClass();

	// バックバッファ作成 (BGR 24bit)
	backBuffer = new Gdiplus::Bitmap(window_size.y, window_size.x, PixelFormat24bppRGB);
	graphics = Gdiplus::Graphics::FromImage(backBuffer);

	// 直接ピクセルアクセス用のストライド計算
	img_stride = ((window_size.y * 3 + 3) / 4) * 4;  // 4バイト境界
	img_data = new BYTE[img_stride * window_size.x];
	memset(img_data, 0, img_stride * window_size.x);

	// Z値バッファ確保
	img_z = new unsigned short[window_size.x * window_size.y];
	memset(img_z, 0, sizeof(unsigned short) * window_size.x * window_size.y);

	// デフォルト描画色を白に設定
	line_color_gdi = gdi_scalar(255, 255, 255);
	line_width = 1;
}

void disp_gdiplus::disp_clear()
{
	// 画像バッファをクリア
	if (img_data) {
		memset(img_data, 0, img_stride * window_size.x);
	}
	if (graphics) {
		graphics->Clear(Gdiplus::Color(0, 0, 0));
	}

	// Z値バッファをクリア
	if (img_z) {
		memset(img_z, 0, sizeof(unsigned short) * window_size.x * window_size.y);
	}
}


void disp_gdiplus::disp_show()
{
	// cv::namedWindow 相当
	if (!hWnd_img) {
		hWnd_img = CreateDisplayWindow(L"img", window_size.y, window_size.x, CW_USEDEFAULT, CW_USEDEFAULT);
		ShowWindow(hWnd_img, SW_SHOW);
		UpdateWindow(hWnd_img);
	}
	if (!hWnd_z) {
		// Z値ウィンドウはメインの横に配置
		RECT rect;
		GetWindowRect(hWnd_img, &rect);
		hWnd_z = CreateDisplayWindow(L"img_z", window_size.y, window_size.x, rect.right + 10, rect.top);
		ShowWindow(hWnd_z, SW_SHOW);
		UpdateWindow(hWnd_z);
	}
}


void disp_gdiplus::point(jhl_xy_i& pos, int size) {
	// Draw square directly to img_data
	int half = size / 2;
	int x_start = (int)pos.x - half;
	int y_start = (int)pos.y - half;
	int x_end = x_start + size;
	int y_end = y_start + size;

	// Clamp to bounds
	if (x_start < 0) x_start = 0;
	if (y_start < 0) y_start = 0;
	if (x_end > (int)window_size.y) x_end = window_size.y;
	if (y_end > (int)window_size.x) y_end = window_size.x;

	for (int y = y_start; y < y_end; y++) {
		BYTE* p = img_data + y * img_stride + x_start * 3;
		for (int x = x_start; x < x_end; x++) {
			p[0] = (BYTE)line_color_gdi.val[0];  // B
			p[1] = (BYTE)line_color_gdi.val[1];  // G
			p[2] = (BYTE)line_color_gdi.val[2];  // R
			p += 3;
		}
	}
}

void disp_gdiplus::point(jhl_xy_i& pos, jhl_rgb color) {
	if (pos.x >= 0 && pos.x < window_size.y && pos.y >= 0 && pos.y < window_size.x) {
		BYTE* p = img_data + pos.y * img_stride + pos.x * 3;
		p[0] = (BYTE)(color.b * 255);
		p[1] = (BYTE)(color.g * 255);
		p[2] = (BYTE)(color.r * 255);
	}
}

void disp_gdiplus::point(jhl_xy_i& pos, gdi_scalar color) {
	if (pos.x >= 0 && pos.x < window_size.y && pos.y >= 0 && pos.y < window_size.x) {
		BYTE* p = img_data + pos.y * img_stride + pos.x * 3;
		p[0] = (BYTE)color.val[2];  // OpenCV Scalar is BGR, val[2] is R but we write BGR
		p[1] = (BYTE)color.val[1];
		p[2] = (BYTE)color.val[0];
	}
}

void disp_gdiplus::point(jhl_xy_i& pos, gdi_scalar* color) {
	if (pos.x >= 0 && pos.x < window_size.y && pos.y >= 0 && pos.y < window_size.x) {
		BYTE* p = img_data + pos.y * img_stride + pos.x * 3;
		p[0] = (BYTE)color->val[2];
		p[1] = (BYTE)color->val[1];
		p[2] = (BYTE)color->val[0];
	}
}

void disp_gdiplus::point(jhl_xy_i& pos, gdi_color3b* c) {
	if (pos.x >= 0 && pos.x < window_size.y && pos.y >= 0 && pos.y < window_size.x) {
		BYTE* p = img_data + pos.y * img_stride + pos.x * 3;
		p[0] = c->b;
		p[1] = c->g;
		p[2] = c->r;
	}
}


#if 1
void disp_gdiplus::point_z(jhl_xy_i& pos, float depth) {
	unsigned short temp_z;

	// 範囲外チェック
	if (!((0 <= pos.x && pos.x < window_size.x &&
		0 <= pos.y && pos.y < window_size.y)))
	{
		return;
	}

	depth = -depth;

	if (depth > 1.0f) depth = 1.0f;
	if (depth < -1.0f) depth = -1.0f;

	temp_z = (unsigned short)((depth + 1) * (65536 / 2));

	unsigned short* p_z = &img_z[pos.y * window_size.y + pos.x];
	if (*p_z <= temp_z)
	{
		*p_z = temp_z;
	}
}
#endif


bool disp_gdiplus::point_z_test(jhl_xy_i& pos, float depth) {
	unsigned short temp_z;

	if (!((0 <= pos.x && pos.x < window_size.x &&
		0 <= pos.y && pos.y < window_size.y)))
	{
		return false;
	}

	depth = -depth;

	if (depth > 1.0f) depth = 1.0f;
	if (depth < -1.0f) depth = -1.0f;

	temp_z = (unsigned short)((depth + 1) * (65536 / 2));

	unsigned short* p_z = &img_z[pos.y * window_size.y + pos.x];
	if (*p_z <= temp_z)
	{
		return true;
	}
	return false;
}

void disp_gdiplus::point_z_set(jhl_xy_i& pos, float depth) {
	unsigned short temp_z;

	depth = -depth;

	if (depth > 1.0f) depth = 1.0f;
	if (depth < -1.0f) depth = -1.0f;

	temp_z = (unsigned short)((depth + 1) * (65536 / 2));

	unsigned short* p_z = &img_z[pos.y * window_size.y + pos.x];
	*p_z = temp_z;
}


void disp_gdiplus::point_with_z(jhl_xy_i& pos, float depth) {
	unsigned short temp_z;

	if (!((0 <= pos.x && pos.x < window_size.x &&
		0 <= pos.y && pos.y < window_size.y)))
	{
		return;
	}

	if (depth > 1.0f) depth = 1.0f;
	if (depth < -1.0f) depth = -1.0f;

	depth = -depth;

	temp_z = (unsigned short)((depth + 1) * (65536 / 2));

	unsigned short* p_z = &img_z[pos.y * window_size.y + pos.x];
	if (*p_z <= temp_z)
	{
		*p_z = temp_z;

		BYTE* p = img_data + pos.y * img_stride + pos.x * 3;
		p[0] = (BYTE)fill_color_gdi.val[0];
		p[1] = (BYTE)fill_color_gdi.val[1];
		p[2] = (BYTE)fill_color_gdi.val[2];
	}
}


void disp_gdiplus::line(jhl_xy_i& start, jhl_xy_i& end)
{
	// Bresenham line algorithm - draw directly to img_data
	int x0 = (int)start.x, y0 = (int)start.y;
	int x1 = (int)end.x, y1 = (int)end.y;

	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;

	while (true) {
		// Draw pixel if within bounds
		if (x0 >= 0 && x0 < (int)window_size.y && y0 >= 0 && y0 < (int)window_size.x) {
			BYTE* p = img_data + y0 * img_stride + x0 * 3;
			p[0] = (BYTE)line_color_gdi.val[0];  // B
			p[1] = (BYTE)line_color_gdi.val[1];  // G
			p[2] = (BYTE)line_color_gdi.val[2];  // R
		}

		if (x0 == x1 && y0 == y1) break;

		int e2 = 2 * err;
		if (e2 > -dy) { err -= dy; x0 += sx; }
		if (e2 < dx) { err += dx; y0 += sy; }
	}
}

void disp_gdiplus::line_f(jhl_xy_i& start, jhl_xy_i& end)
{
	// Bresenham line algorithm with fill color - draw directly to img_data
	int x0 = (int)start.x, y0 = (int)start.y;
	int x1 = (int)end.x, y1 = (int)end.y;

	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;

	while (true) {
		if (x0 >= 0 && x0 < (int)window_size.y && y0 >= 0 && y0 < (int)window_size.x) {
			BYTE* p = img_data + y0 * img_stride + x0 * 3;
			p[0] = (BYTE)fill_color_gdi.val[0];
			p[1] = (BYTE)fill_color_gdi.val[1];
			p[2] = (BYTE)fill_color_gdi.val[2];
		}
		if (x0 == x1 && y0 == y1) break;
		int e2 = 2 * err;
		if (e2 > -dy) { err -= dy; x0 += sx; }
		if (e2 < dx) { err += dx; y0 += sy; }
	}
}

void disp_gdiplus::line_h(unsigned int v, float h_start, float h_end)
{
	// Optimized horizontal line - draw directly to img_data
	if (v >= window_size.x) return;

	int x_start = (int)(h_start + 0.5f);
	int x_end = (int)(h_end + 0.5f);
	if (x_start > x_end) { int tmp = x_start; x_start = x_end; x_end = tmp; }

	if (x_start < 0) x_start = 0;
	if (x_end >= (int)window_size.y) x_end = window_size.y - 1;
	if (x_start > x_end) return;

	BYTE* p = img_data + v * img_stride + x_start * 3;
	for (int x = x_start; x <= x_end; x++) {
		p[0] = (BYTE)fill_color_gdi.val[0];
		p[1] = (BYTE)fill_color_gdi.val[1];
		p[2] = (BYTE)fill_color_gdi.val[2];
		p += 3;
	}
}


void disp_gdiplus::circle(jhl_xy_i& pos, int radius, jhl_rgb& color, int thickness)
{
	Gdiplus::Color gdipColor((BYTE)(color.r * 255), (BYTE)(color.g * 255), (BYTE)(color.b * 255));

	if (thickness < 0) {
		// 塗りつぶし
		Gdiplus::SolidBrush brush(gdipColor);
		graphics->FillEllipse(&brush,
			(int)pos.x - radius, (int)pos.y - radius,
			radius * 2, radius * 2);
	}
	else {
		Gdiplus::Pen pen(gdipColor, (Gdiplus::REAL)thickness);
		graphics->DrawEllipse(&pen,
			(int)pos.x - radius, (int)pos.y - radius,
			radius * 2, radius * 2);
	}
}

void disp_gdiplus::rectangle(jhl_xy_i& start, jhl_xy_i& end, jhl_rgb& color, int thickness)
{
	Gdiplus::Color gdipColor((BYTE)(color.r * 255), (BYTE)(color.g * 255), (BYTE)(color.b * 255));

	int x = (start.x < end.x) ? (int)start.x : (int)end.x;
	int y = (start.y < end.y) ? (int)start.y : (int)end.y;
	int w = abs((int)end.x - (int)start.x);
	int h = abs((int)end.y - (int)start.y);

	if (thickness < 0) {
		Gdiplus::SolidBrush brush(gdipColor);
		graphics->FillRectangle(&brush, x, y, w, h);
	}
	else {
		Gdiplus::Pen pen(gdipColor, (Gdiplus::REAL)thickness);
		graphics->DrawRectangle(&pen, x, y, w, h);
	}
}


void disp_gdiplus::putText(const char* s, jhl_xy_i& pos, jhl_rgb& color)
{
	if (!font) {
		fontFamily = new Gdiplus::FontFamily(L"Arial");
		font = new Gdiplus::Font(fontFamily, 12, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	}

	// char* を wchar_t* に変換
	int len = MultiByteToWideChar(CP_ACP, 0, s, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s, -1, wstr, len);

	Gdiplus::SolidBrush brush(Gdiplus::Color(
		(BYTE)(color.r * 255),
		(BYTE)(color.g * 255),
		(BYTE)(color.b * 255)
	));

	Gdiplus::PointF point((Gdiplus::REAL)pos.x, (Gdiplus::REAL)pos.y);
	graphics->DrawString(wstr, -1, font, point, &brush);

	delete[] wstr;
}

void disp_gdiplus::disp_swap(void)
{
	// img_data をバックバッファに転送
	if (backBuffer && img_data) {
		Gdiplus::BitmapData bitmapData;
		Gdiplus::Rect rect(0, 0, window_size.y, window_size.x);

		if (backBuffer->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat24bppRGB, &bitmapData) == Gdiplus::Ok) {
			BYTE* dest = (BYTE*)bitmapData.Scan0;
			for (int y = 0; y < (int)window_size.x; y++) {
				memcpy(dest + y * bitmapData.Stride, img_data + y * img_stride, window_size.y * 3);
			}
			backBuffer->UnlockBits(&bitmapData);
		}
	}

	// cv::imshow 相当
	if (hWnd_img) {
		InvalidateRect(hWnd_img, NULL, FALSE);
		UpdateWindow(hWnd_img);
	}
	if (hWnd_z) {
		InvalidateRect(hWnd_z, NULL, FALSE);
		UpdateWindow(hWnd_z);
	}
}

void disp_gdiplus::disp_destroy()
{
	// ウィンドウ破棄
	if (hWnd_img) {
		DestroyWindow(hWnd_img);
		hWnd_img = NULL;
	}
	if (hWnd_z) {
		DestroyWindow(hWnd_z);
		hWnd_z = NULL;
	}

	// リソース解放
	delete graphics; graphics = nullptr;
	delete backBuffer; backBuffer = nullptr;
	delete[] img_data; img_data = nullptr;
	delete[] img_z; img_z = nullptr;
	delete font; font = nullptr;
	delete fontFamily; fontFamily = nullptr;

	// GDI+ シャットダウン
	if (gdiplusInitialized) {
		Gdiplus::GdiplusShutdown(gdiplusToken);
		gdiplusInitialized = false;
	}
}


// cv::waitKey の代替
int waitKeyGdiplus(int delay_ms)
{
	MSG msg;
	DWORD startTime = GetTickCount();
	g_lastKey = -1;

	while (true) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (g_lastKey != -1) {
				int key = g_lastKey;
				g_lastKey = -1;
				return key;
			}
		}

		if (delay_ms > 0) {
			if (GetTickCount() - startTime >= (DWORD)delay_ms) {
				return -1;
			}
		}
		else if (delay_ms == 0) {
			return -1;
		}

		Sleep(1);
	}
}
