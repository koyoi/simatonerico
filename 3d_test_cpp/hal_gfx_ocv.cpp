#include "stdafx.h"

#ifndef USE_GDIPLUS

#include "hal_gfx_ocv.h"
#include <opencv2/opencv.hpp>

#include <assert.h>


cv::Mat	disp_ocv2::img;
cv::Mat	disp_ocv2::img_z;

int disp_ocv2::font_name;
int disp_ocv2::font_size;
cv::Scalar	disp_ocv2::line_color_cv;
cv::Scalar	disp_ocv2::fill_color_cv;


void disp_ocv2::
disp_init(jhl_xy_i window_size_)
{
	window_size.x = window_size_.y;
	window_size.y = window_size_.x;	// row ,column �� x,y �͂��ꂩ����Ă���ۂ��̂�
	img = cv::Mat::zeros(window_size.x, window_size.y, CV_8UC3);
	img_z = cv::Mat::zeros(window_size.x, window_size.y, CV_16U);
}

void disp_ocv2::
disp_clear()
{
	img = 0;
	img_z = 0;
}


void disp_ocv2::
disp_show()
{
	cv::namedWindow("img", cv::WINDOW_AUTOSIZE );	// auto �Ƃ́A�R���e���c�̃T�C�Y�Ɏ����ύX�̈Ӗ�
	cv::namedWindow("img_z", cv::WINDOW_AUTOSIZE );
}


void disp_ocv2::
point(jhl_xy_i& pos, int size) {
	cv::circle(img, 
		cv::Point(pos.x, pos.y), 
		size, 
		line_color_cv,
		-1/* -1:fill */
	);
}

void disp_ocv2::
point(jhl_xy_i& pos, jhl_rgb color) {
	cv::circle(img,
		cv::Point(pos.x, pos.y),
		1,
		cv::Scalar(color.b, color.g, color.r),
		0
	);
	cv::Vec3b *tgt = img.ptr<cv::Vec3b>(pos.y);
	tgt[(int)(pos.x)] = cv::Vec3b(color.b, color.g, color.r);


}

void disp_ocv2::
point(jhl_xy_i& pos, cv::Scalar color) {
	cv::Vec3b *tgt = img.ptr<cv::Vec3b>(pos.y);
	tgt[(int)(pos.x)] = cv::Vec3b(color.val[2], color.val[1], color.val[0]);
}

void disp_ocv2::
point(jhl_xy_i& pos, cv::Scalar *color) {
	cv::Vec3b *tgt = img.ptr<cv::Vec3b>(pos.y);
	tgt[(int)(pos.x)] = cv::Vec3b(color->val[2], color->val[1], color->val[0]);
}

void disp_ocv2::
point(jhl_xy_i& pos, cv::Vec3b* c) {
	cv::Vec3b *tgt = img.ptr<cv::Vec3b>(pos.y);
	tgt[(int)(pos.x)] = *c;
}


#if 1
void disp_ocv2::
point_z(jhl_xy_i& pos, float depth) {
	ushort temp_z;
	static	float toria_z_min = 0.0;
	static	float toria_z_max = 1.0;

	// �͈͊O�`�F�b�N
	if (!((0 <= pos.x && pos.x < window_size.x &&
		0 <= pos.y && pos.y < window_size.y)))
	{
		return;
	}

	depth = -depth;	// z �N���A�� 0 fill �ɂ��邽�߁B�}�C�i�X���|����ƁA��O:1 ��:-1�i���Ƃ�unsigned�ɃV�t�g�j

					// �O�a
	if (depth > 1.0f)
	{
		//		assert(depth <= 1.0f);	// �U�ƂȂ�ƃu���[�N
		depth = 1.0f;
	};
	if (depth < -1.0f)
	{
		//		assert(depth >= -1.0f);
		depth = -1.0f;
	};

#if 0
	// �������̓s���ŁA�K���ɃX�P�[��
	if (depth < toria_z_min)
		depth = toria_z_min;
	if (depth > toria_z_max)
		depth = toria_z_max;

	depth -= toria_z_min;
	depth /= (toria_z_max - toria_z_min);
	temp_z = (ushort)((depth /*+1*/) * 65536/*32768*/);
#endif
	temp_z = (ushort)((depth + 1) * (65536 / 2));

	ushort *tgt_z = img_z.ptr<ushort>(pos.y);
	ushort *p_z = &tgt_z[(int)(pos.x)];
	if (*p_z < temp_z)
	{

		// todo �e�N�X�`�������߂̏ꍇ������
		*p_z = temp_z;

		//  ���Ƃ�
		// 	cv::Vec3b *tgt = img.ptr<cv::Vec3b>(pos.y);
		//	tgt[(int)(pos.x)] = cv::Vec3b(fill_color_cv[0], fill_color_cv[1], fill_color_cv[2]);
	}
}
#endif


bool disp_ocv2::
point_z_test(jhl_xy_i& pos, float depth) {
	ushort temp_z;
	static	float toria_z_min = 0.0;
	static	float toria_z_max = 1.0;

	// �͈͊O�`�F�b�N
	if( !((0 <= pos.x && pos.x < window_size.x &&
		0 <= pos.y && pos.y < window_size.y)) )
	{
		return false;
	}

	depth = -depth;	// z �N���A�� 0 fill �ɂ��邽�߁B�}�C�i�X���|����ƁA��O:1 ��:-1�i���Ƃ�unsigned�ɃV�t�g�j

	// �O�a
	if (depth > 1.0f) 
	{ 
//		assert(depth <= 1.0f);	// �U�ƂȂ�ƃu���[�N
		depth = 1.0f; 
	};
	if (depth < -1.0f) 
	{ 
//		assert(depth >= -1.0f);
		depth = -1.0f;
	};

#if 0
	// �������̓s���ŁA�K���ɃX�P�[��
	if (depth < toria_z_min)
		depth = toria_z_min;
	if (depth > toria_z_max)
		depth = toria_z_max;

	depth -= toria_z_min;
	depth /= (toria_z_max - toria_z_min);
	temp_z = (ushort)((depth /*+1*/) * 65536/*32768*/);
#endif
	temp_z = (ushort)((depth + 1) * (65536 / 2));

	ushort *tgt_z = img_z.ptr<ushort>(pos.y);
	ushort *p_z = &tgt_z[(int)(pos.x)];
	if (*p_z < temp_z)
	{

		// �e�N�X�`�������߂̏ꍇ������̂ŁA���̊֐��ł̓e�X�g�̂�
		//		*p_z = temp_z;
		return true;

		//  ���Ƃ�
		// 	cv::Vec3b *tgt = img.ptr<cv::Vec3b>(pos.y);
		//	tgt[(int)(pos.x)] = cv::Vec3b(fill_color_cv[0], fill_color_cv[1], fill_color_cv[2]);
	}
	return false;
}

void disp_ocv2::
point_z_set(jhl_xy_i& pos, float depth) {
	ushort temp_z;

	// �͈͊O�`�F�b�N�� z_test �ōς�ł���̂ŏȗ�

	depth = -depth;	// z �N���A�� 0 fill �ɂ��邽�߁B�}�C�i�X���|����ƁA��O:1 ��:-1�i���Ƃ�unsigned�ɃV�t�g�j

	// todo ������ȗ��ł���͂�
	// �O�a
	if (depth > 1.0f)
	{
		//		assert(depth <= 1.0f);	// �U�ƂȂ�ƃu���[�N
		depth = 1.0f;
	};
	if (depth < -1.0f)
	{
		//		assert(depth >= -1.0f);
		depth = -1.0f;
	};

	temp_z = (ushort)((depth + 1) * (65536 / 2));

	ushort *tgt_z = img_z.ptr<ushort>(pos.y);
	ushort *p_z = &tgt_z[(int)(pos.x)];
	*p_z = temp_z;
}


void disp_ocv2::
point_with_z(jhl_xy_i& pos, float depth) {
	ushort temp_z;

	// �͈͊O�`�F�b�N
	if (!((0 <= pos.x && pos.x < window_size.x &&
		0 <= pos.y && pos.y < window_size.y)))
	{
		return;
	}

	// �O�a
	if (depth > 1.0f)
	{
		//		assert(depth <= 1.0f);	// �U�ƂȂ�ƃu���[�N
		depth = 1.0f;
	};
	if (depth < -1.0f)
	{
		//		assert(depth >= -1.0f);
		depth = -1.0f;
	};

	depth = -depth ;

#if 0
	static	float toria_z_min = -.8;
	static	float toria_z_max = -.4;
	// �������̓s���ŁA�K���ɃX�P�[��
	if (depth < toria_z_min)
		depth = toria_z_min;
	if (depth > toria_z_max)
		depth = toria_z_max;

	depth -= toria_z_min;
	depth /= (toria_z_max - toria_z_min);

//	temp_z = (ushort)(depth * 65536);
#endif
	temp_z = (ushort)((depth + 1) * (65536 / 2));

	ushort *tgt_z = img_z.ptr<ushort>(pos.y);
	ushort *p_z = &tgt_z[(int)(pos.x)];
	if (*p_z < temp_z)
	{
		*p_z = temp_z;

		cv::Vec3b *tgt = img.ptr<cv::Vec3b>(pos.y);
		tgt[(int)(pos.x)] = cv::Vec3b(fill_color_cv[0], fill_color_cv[1], fill_color_cv[2] );
	}
}


void disp_ocv2::
line(jhl_xy_i& start, jhl_xy_i& end)
//	�n���A�h���X�̓f�B�X�v���C���W
{
	cv::line(img,
		cv::Point(start.x, start.y),
		cv::Point(end.x, end.y),
		line_color_cv,
		line_width
	);
}

// line�ł�fill����(�F�w�肪fill)
void disp_ocv2::
line_f(jhl_xy_i& start, jhl_xy_i& end)
//	�n���A�h���X�̓f�B�X�v���C���W
{
	cv::line(img,
		cv::Point(start.x, start.y),
		cv::Point(end.x, end.y),
		fill_color_cv,
		line_width
	);
}

void disp_ocv2::
line_h(unsigned int v, float h_start, float h_end)
{
	line_f(jhl_xy_i(h_start +0.5f , v), jhl_xy_i(h_end+0.5f, v));	// �Ȃ񂿂���Ďl�̌ܓ�
//	line(jhl_xy_i(h_start, v), jhl_xy_i(h_end, v));
}


void disp_ocv2::
circle(jhl_xy_i& pos, int radius, jhl_rgb& color, int thickness)
{
	cv::circle(img, cv::Point(pos.x, pos.y), radius, CV_RGB(color.r, color.g, color.b), thickness); // �ȗ��p�����[�^�L��
}

void disp_ocv2::
rectangle(jhl_xy_i& start, jhl_xy_i& end, jhl_rgb& color, int thickness)
{
	cv::rectangle(img, 
		cv::Point(start.x, start.y), 
		cv::Point(end.x, end.y),
		CV_RGB(color.r, color.g, color.b), 
		thickness);
}


void disp_ocv2::
putText(const char* s, jhl_xy_i& pos, jhl_rgb& color)
{
	cv::putText(img, s, cv::Point(pos.x, pos.y),
		font_name,
		font_size,
		CV_RGB(color.r*255, color.g*255, color.b*255),	// todo �ϊ��֐��I��
		/*thickness*/1,
		CV_AA);
}

void disp_ocv2::
disp_swap(void)
{
	cv::imshow("img", img);
	cv::imshow("img_z", img_z);
}

void disp_ocv2::disp_destroy()
{
	cv::destroyAllWindows();
}

#endif // !USE_GDIPLUS
