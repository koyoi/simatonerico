#include "stdafx.h"

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "jhl3dLib.h"
#include "readData.h"

#ifdef _WIN32_
#include "hal_gfx_ocv.h"
//#include "windows.h"
disp_ocv2	painter;

#else	// ameba

#endif


jhl_xy_i window = { 640, 480 };

#if 0
const std::string data_file[] = { "C:\\Users\\N2232\\Documents\\vs_proj\\3dtest_cpp\\data\\dat.txt",
								  "C:\\Users\\N2232\\Documents\\vs_proj\\3dtest_cpp\\data\\dat_cube.txt" };
#else
const std::string data_file[] = { "L:\\users\\mayura.kage7\\Documents\\My Dropbox\\jhl_3d\\3d_test_cpp\\data\\dat.txt",
								  "L:\\users\\mayura.kage7\\Documents\\My Dropbox\\jhl_3d\\3d_test_cpp\\data\\dat_cube.txt" };
#endif




// ---------------------------------------------------
static void mouse_event(int event, int x, int y, int flags, void* param);
static int proc_key(char);
static void draw_info();


// ---------------------------------------------------
struct sceneObj
{
	//	jhl_xyz		ofst;
	matHomo4	trans;
	matHomo4	acc;
	jhl_xyz		size;
	//	jhl_xyz	move_vect_ofst;		// todo モデルローカルの原点から、オフセットをつけて、特に回転を掛けたいときのため。
	bool	is_moved;
	// 他、雑多
};



// ---------------------------------------------------
#define N_PARA_LIGHTS	2
const int NUM_OBJ = 2;

int		param1 = 0;
int		param2 = 0;
long	time_elapsed = 0;

viewport_config	viewport_area;
bool	area_changed = true;

sceneObj	obj[NUM_OBJ];		//	位置・回転、速度その他諸々

bool	frame_pause = false;



int
main(int argc, char *argv[])
{
	CvScalar font_color_info_defaut = CV_RGB( 0, 255, 255 );
	unsigned long	frame_time = 300;	// [ms]

	jhl3Dlib::set_painter(painter);

	{
		const int far = 40;
		const int near = 5;
		const int left = -5;
		const int right = -left;
		const int top = -left*(window.x / window.y);
		const int btm = -top;
		viewport_area = { far, near,  left, right, top, btm };
	}
	jhl3Dlib::set_proj_mat_norm(viewport_area);
//	jhl3Dlib::set_proj_mat_ortho(viewport_area);


	jhl_rgb light_ambient = { .15f, .15f, .15f };
	dir_light lights[ N_PARA_LIGHTS ];			// 並行光源　方向、色。方向は、正規化してないと不正になるかも

	painter.disp_init(window);	// 640 x 480 の3レイヤー(BGR)で色バッファを生成・初期化

	polMdl		models[NUM_OBJ];	//	ポリゴンモデルそのものと、それに適用するアフィン変換の最終的なマトリックス


	// ファイル読み込み
	std::cout << "read data" << std::endl;

	int rv = 0;
	for (int i = 0; i < NUM_OBJ; i++)
	{
		if (read_data(models[i].model, data_file[i]) <= 0) {
			std::cout << "file read error. abort." << std::endl;
			exit(-1);
		}
		else
		{
			std::cout << std::endl << "obj id: " << i << std::endl;
			modelData::dataDump( models[i].model );
			// std::cout << "id:" << i << "  vertxes : " << objects[i].model.n_vert << " polygons : " << objects[i].model.n_pol << std::endl;
		}
	}

	// 初期設定
	jhl_rgb line_color = jhl_rgb(210, 150, 130);
	int line_width = 1;

	lights[0].dir = jhl_xyz(0.f, 1.f, 0.f);
	lights[0].col = jhl_rgb(0.f, .7f, .5f);

	lights[1].dir = jhl_xyz(0.1f, -.78f, 0.2f);
	lights[1].col = jhl_rgb(.5f, 0.f, 0.f);


	// ビュー行列生成
	jhl_xyz eye = jhl_xyz(0.f, 0.f, 20.f);
	jhl_xyz tgt = jhl_xyz(0.f, 0.f, 0.f);
	jhl_xyz	u   = jhl_xyz(0.f, 1.f, 0.f);
	jhl3Dlib::set_view_mat(eye, u, tgt);  
	std::cout << jhl3Dlib::view_mat << std::endl;


	// オブジェクトの位置
	obj[0].trans = matHomo4(jhl_xyz (-2,1,-3) );
//	obj[0].trans.rot_axis_x(0.2f);
//	obj[0].trans.rot_axis_y(0.3f);
//	obj[0].trans.rot_axis_z(0.4f);
//	obj[0].trans.rot_by_vec(0.1f, 0.12f, 0.15f, 0.21f);
	obj[0].acc = matHomo4(1);
	obj[0].acc.rot_axis_x(0.2f);
	obj[0].acc.rot_axis_y(0.3f);
	obj[0].acc.rot_axis_z(0.4f);
	obj[0].acc.rot_by_vec(0.1f, 0.12f, 0.15f, 0.21f);
	obj[0].size = jhl_xyz(1);
	obj[0].size *= jhl_xyz(1, 2, 3);
	obj[0].is_moved = true;

	obj[1].trans = matHomo4(jhl_xyz(1, 1, 1));
	obj[1].acc = matHomo4(1);
	obj[1].acc.rot_axis_x(0.2f);
	obj[1].acc.rot_axis_y(0.3f);
	obj[1].acc.rot_axis_z(0.4f);
	obj[1].is_moved = true;


	// ディスプレイ座標系へ
	jhl3Dlib::set_disp_trans(window);
	std::cout << "viewport_trans" << jhl3Dlib::disp_mat << std::endl;

	painter.disp_init(window);

	// マウスイベント時に関数mouse_eventの処理を行う
#ifdef _WIN32_
	cv::setMouseCallback("img", mouse_event);//   重いし、おかしい？
#else
#endif

	painter.disp_show();
#ifdef _WIN32_
	painter.fontInit();
#else
#endif

	time_elapsed = 0;

	matHomo4 t_work(1);

	std::cout << "\"q\" to exit." << std::endl;

	char k = 1;
	while (k != 'q')
	{
		k = cv::waitKey(1);
		proc_key(k);
		if (frame_pause) {
// todo			_sleep(100);
			continue;    // 次のループへ
		}

		std::cout << std::endl << "frame " << time_elapsed << std::endl;

		if (area_changed)	// UI操作
		{
			jhl3Dlib::set_proj_mat_norm(viewport_area);
			//	jhl3Dlib::set_proj_mat_ortho(viewport_area);
			area_changed = false;
		}

		// if(r_moved)	// UI操作
		{
			// ここでは処理不要
		}

		painter.disp_clear();

		// 時刻更新でのオブジェクト変更だとか
		for (int i = 0; i < NUM_OBJ; i++)
		{
			obj[i].trans *= obj[i].acc;
			models[i].model_mat = obj[i].trans * obj[i].size;
		}

		int rv;
		// 実際の描画
		for (int i = 0; i < NUM_OBJ; i++)
		{
			std::cout << "obj_trans mat" << std::endl;
			std::cout << obj[i].trans << std::endl << std::endl;

			std::cout << "obj size mat" << std::endl;
			std::cout << obj[i].size << std::endl << std::endl;


			// 局所的な変換（アニメーションやボーン変形）
			rv = jhl3Dlib::drawLines(models[i]);

		}
		// ui
		draw_info();

		painter.disp_swap();

// todo		_sleep(frame_time);
		time_elapsed += 1;
	}

	painter.disp_destroy();

	std::cout << "--- end ---" << std::endl;

#if 0
		// open cv test app
	// 初期化時に塗りつぶす
	cv::Mat red_img(cv::Size(640, 480), CV_8UC3, cv::Scalar(0, 0, 255));
	cv::Mat white_img(cv::Size(640, 480), CV_8UC3, cv::Scalar::all(255));
	cv::Mat black_img = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);

	// 初期化後に塗りつぶす
	cv::Mat green_img = red_img.clone();
	green_img = cv::Scalar(0, 255, 0);

	cv::namedWindow("red image", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::namedWindow("white image", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::namedWindow("black image", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::namedWindow("green image", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("white image", white_img);
	cv::imshow("black image", black_img);
	cv::imshow("green image", green_img);
	cv::waitKey(0);

#endif
}




int proc_key(char key)
{
	// カメラの変化
	switch (key)
	{
	case('g'):
		viewport_area.far += 1;
		area_changed = 1;
		break;
	case('b'):
		viewport_area.far -= 1;
		if (viewport_area.far == viewport_area.near)
		{
			viewport_area.far += 1;
		}
		area_changed = 1;
		break;
	case('f'):
		viewport_area.near += 1;
		if (viewport_area.far == viewport_area.near)
		{
			viewport_area.near -= 1;
		}
		area_changed = 1;
		break;
	case('v'):
		viewport_area.near -= 1;
		area_changed = 1;
		break;


		// レンダリングタイプ変更
	case('t'):
		jhl3Dlib::draw_type_next();
		break;

		// オブジェクトの位置
	case('s'):
		obj[0].trans.trans(0, 1, 0);
		break;
	case('x'):
		obj[0].trans.trans(0, -1, 0);
		break;
	case('z'):
		obj[0].trans.trans(-1, 0, 0);
		break;
	case('c'):
		obj[0].trans.trans(1, 0, 0);
		break;
	case('w'):
		// reset
		obj[0].trans.v[0] = 0;
		obj[0].trans.v[1] = 0;
		obj[0].trans.v[2] = 0;
		obj[0].trans.v[3] = 1.f;
		break;

		// シーン制御
	case('p'):
		frame_pause = !frame_pause;
		if (frame_pause)
		{
			std::cout << "pause. (p)" << std::endl;
		}

		break;
	default:
		break;
	}

	return 0;
}


void draw_info()
{
#if 0
	text = "pos: (x,y,z) : " + str(r_x) + ", " + str(r_y) + ", " + str(r_z)
		cv2.putText(img, text, (0, 12), font, font_size, font_color_info_defaut)
		text = str(time_elapsed)
		cv2.putText(img, text, (0, 24), font, font_size, font_color_info_defaut)
		text = "near(f/v) : " + str(area_near)
		cv2.putText(img, text, (0, 36), font, font_size, font_color_info_defaut)
		text = "far (g/b) : " + str(area_far)
		cv2.putText(img, text, (0, 48), font, font_size, font_color_info_defaut)
#endif
}

static void 
mouse_event(int event, int x, int y, int flags, void* param)
{
	// イベントテスト
	{
		// 左クリックで赤い円形を生成
		if (event == cv::EVENT_LBUTTONUP)
		{
			painter.circle(jhl_xy_i(x, y), 50, jhl_rgb(0, 0, 255));
		}

		// 右クリック + Shiftキーで緑色のテキストを生成
		else if (event == cv::EVENT_RBUTTONUP && (flags & cv::EVENT_FLAG_SHIFTKEY))
		{
			painter.putText("CLICK!!", jhl_xy_i(x, y),
				/*cv2.FONT_HERSHEY_SIMPLEX, 5,*/
				jhl_rgb(0, 255, 0)/*, 3, cv2.CV_AA*/);
		}

		// 右クリックのみで青い四角形を生成
		else if (event == cv::EVENT_RBUTTONUP)
		{
			painter.rectangle(jhl_xy_i(x - 100, y - 100), jhl_xy_i(x + 100, y + 100), jhl_rgb(255, 0, 0), -1);
		}
	}

#if 0
//	print("mouse event")
	if( event == cv::EVENT_LBUTTONUP )
	{
		if (x > window_h / 2)
		{
			param1 += (y > window_v / 2) ? 1 : -1;
		}
		else
		{
			param2 += (y > window_v / 2) ? 1 : -1;
		}
		print(x, y, param1, param2):
		print("mouse event"):
	}
#endif
}

