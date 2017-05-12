#include "stdafx.h"

#ifdef _WIN32_
#include "windows.h"
#endif


#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "jhl3dLib.h"
#include "readData.h"

#ifdef _WIN32_
#include "hal_gfx_ocv.h"
disp_ocv2	painter;

#else	// ameba

#endif


jhl_xy_i window = { 640, 480 };

#ifdef N2232
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
	jhl_xyz		pos;
	matHomo4	acc;
	matHomo4	trans;
	jhl_size	size = 1;
	//	jhl_xyz	move_vect_ofst;		// todo ローカルの原点以外を中心に回転したいとき
	bool		is_moved;
	object		obj;

	bool		attrib_override;
	jhl_rgb		color;
	// 他、雑多
};


// ---------------------------------------------------
#define N_PARA_LIGHTS	2
#define NUM_MODEL	2
#define NUM_OBJ		3

int		param1 = 0;
int		param2 = 0;
long	frame = 0;

viewport_config	viewport_area;
bool	area_changed = true;

bool	frame_pause = false;
jhl_rgb	font_color_info_defaut = jhl_rgb(0, 255, 255);

modelData	models[NUM_MODEL];	//	ポリゴンモデルそのものと、それに適用するアフィン変換の最終的なマトリックス
sceneObj	obj[NUM_OBJ];		//	位置・回転、速度その他諸々


void mat_test(void);


int
main(int argc, char *argv[])
{
	mat_test();

	unsigned long	frame_time = 300;	// [ms]

	jhl3Dlib::set_painter(painter);
	painter.disp_init(window);	// 640 x 480 の3レイヤー(BGR)で色バッファを生成・初期化

	{
		int vp_far = 80;
		int vp_near = 5;
		float left = -20;
		float right = -left;
		float top = -left*((float)window.y / window.x);
		float btm = -top;
		viewport_area = { vp_far, vp_near,  (int)left, (int)right, (int)top, (int)btm };
	}
	jhl3Dlib::set_proj_mat(viewport_area, false);	// false : パースあり。

	jhl_rgb		light_ambient = { .15f, .15f, .15f };
	dir_light	lights[ N_PARA_LIGHTS ];			// 並行光源　方向、色。方向は、正規化してないと不正になるかも

	jhl_rgb		line_color = jhl_rgb(210, 150, 130);	// todo ここ？
	int			line_width = 1;

	lights[0].dir = jhl_xyz(0.f, 1.f, 0.f);
	lights[0].col = jhl_rgb(0.f, .7f, .5f);

	lights[1].dir = jhl_xyz(0.1f, -.78f, 0.2f);
	lights[1].col = jhl_rgb(.5f, 0.f, 0.f);


	// ビュー行列生成
	jhl_xyz eye = jhl_xyz(0.f, 0.f, 20.f);
	jhl_xyz tgt = jhl_xyz(0.f, 0.f, 0.f);
	jhl_xyz	u   = jhl_xyz(0.f, 1.f, 0.f);
	jhl3Dlib::set_view_mat(eye, u, tgt);  
//	std::cout << "view_mat" << std::endl;
//	std::cout << jhl3Dlib::view_mat << std::endl;

	// ディスプレイ変換
	jhl3Dlib::set_disp_trans(window);

	jhl3Dlib::set_draw_type( drawType_line_front_face );

	// ファイル読み込み
	std::cout << "read data" << std::endl;
	int rv = 0;
	for (int i = 0; i < NUM_MODEL; i++)
	{
		std::cout << "read file : " << data_file[i] << std::endl;
		if (read_data(models[i], data_file[i]) <= 0) {
			std::cout << "file read error. abort." << std::endl;
			exit(-1);
		}
		else
		{
			std::cout << "obj id: " << i << std::endl;
			modelData::dataDump(models[i]);
			std::cout << std::endl;
			// std::cout << "id:" << i << "  vertxes : " << objects[i].model.n_vert << " polygons : " << objects[i].model.n_pol << std::endl;
		}
	}

	// オブジェクトの位置
	obj[0].pos = jhl_xyz(0, 0, -5);
	obj[0].trans = 1;
//	obj[0].trans.rot_axis_x(0.2f);
//	obj[0].trans.rot_axis_y(0.3f);
//	obj[0].trans.rot_axis_z(0.4f);
//	obj[0].trans.rot_by_vec(0.1f, 0.12f, 0.15f, 0.21f);
	obj[0].acc = 1;
	obj[0].acc.rot_axis_x(0.5f/3.14);
//	obj[0].acc.rot_axis_y(0.3f/3.14);
//	obj[0].acc.rot_axis_z(0.2f/3.14);
	obj[0].acc.rot_by_vec( jhl_xyz(0.1f, 0.12f, 0.15f).normalize(), 0.2f);	// 続けてたらnormalizeのfp16量子化誤差の蓄積でわずかに各宿はいるかも試練が、それを言ったら。
	obj[0].size = 1;
//	obj[0].size *= jhl_size(1, 2, 3);
	obj[0].is_moved = true;
	obj[0].obj.p_model = &models[0];


	obj[1].size = 1;
	obj[1].pos = jhl_xyz(5, 2, -10);
	//	obj[1].trans = matHomo4(jhl_xyz(5, 2, -10));
	obj[1].trans = 1;
	obj[1].acc = matHomo4(1);
//	obj[1].acc.rot_axis_x(0.2f);
	obj[1].acc.rot_axis_y((float)0.3f/3.14);
//	obj[1].acc.rot_axis_z(0.4f);
	obj[1].is_moved = true;
	obj[1].obj.p_model = &models[1];

	obj[2].pos = jhl_xyz(-2, 4, -15);
	obj[2].trans = 1;
	obj[2].acc = 1;
	obj[2].acc.rot_axis_x((float)(0.25f / 3.14));
	//	obj[0].acc.rot_by_vec(0.1f, 0.12f, 0.15f, 0.21f);
	obj[2].size = 1.5;
	//	obj[2].size *= jhl_size(1, 2, 3);
	obj[2].obj.p_model = &models[0];
	obj[2].obj.attrib_override = true;
	obj[2].obj.color = jhl_rgb(255, 220, 50);


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

	frame = 0;

	std::cout << std::endl;
	std::cout << "start loop." << std::endl;
	std::cout << "\"q\" to exit." << std::endl;

	char k = 1;
	while (k != 'q')
	{
		k = cv::waitKey(1);
		proc_key(k);
		if (frame_pause) {
			Sleep(frame_time);
			continue;    // 次のループへ
		}

		std::cout << std::endl << "frame " << frame << std::endl;

		if (area_changed)	// UI操作
		{
			jhl3Dlib::set_proj_mat(viewport_area, false);
			area_changed = false;
		}

		// if(r_moved)	// UI操作
		{
			// ここでは処理不要
		}

		// 時刻更新でのオブジェクト変更だとか
		for (int i = 0; i < NUM_OBJ; i++)
		{
			// 局所的な変換（アニメーションやボーン変形）
			// todo

			obj[i].trans = obj[i].acc * obj[i].trans;
			rigid_trans( &obj[i].obj.model_mat, obj[i].pos, obj[i].trans, obj[i].size);
		}

		painter.disp_clear();
		int rv;
		// 実際の描画
#if 0
		for (int i = 0; i < NUM_OBJ; i++)
#else
		for (int i = 0; i < 1; i++)
#endif
		{
			rv = jhl3Dlib::draw(obj[i].obj);
		}

		// ui
		draw_info();

		painter.disp_swap();

		Sleep(frame_time);
		frame += 1;

	}

	// 終了　適当だけど
	painter.disp_destroy();

	std::cout << "--- end ---" << std::endl;

}




int proc_key(char key)
{
	// カメラの変化
	switch (key)
	{
		#if 0
	case('g'):
		viewport_area.vp_far += 1;
		area_changed = 1;
		break;
	case('b'):
		viewport_area.vp_far -= 1;
		if (viewport_area.vp_far == viewport_area.vp_near)
		{
			viewport_area.vp_far += 1;
		}
		area_changed = 1;
		break;
	case('f'):
		viewport_area.vp_near += 1;
		if (viewport_area.vp_far == viewport_area.vp_near)
		{
			viewport_area.vp_near -= 1;
		}
		area_changed = 1;
		break;
	case('v'):
		viewport_area.vp_near -= 1;
		area_changed = 1;
		break;

#endif
		// レンダリングタイプ変更
	case('t'):
		jhl3Dlib::draw_type_next();
		break;

		// オブジェクトの位置
	case('s'):
		obj[0].pos.y +=1;
		break;
	case('x'):
		obj[0].pos.y -= 1;
		break;
	case('z'):
		obj[0].pos.x += 1;
		break;
	case('c'):
		obj[0].pos.x -= 1;
		break;
	case('f'):
		obj[0].pos.z += 1;
		break;
	case('v'):
		obj[0].pos.z -= 1;
		break;
	case('w'):
		obj[0].pos.x = 0;
		obj[0].pos.y = 0;
		obj[0].pos.z = 0;
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
#if true
	char text[63];
	sprintf(text, "frame: %4d", frame);
	painter.putText( text, jhl_xy_i(0, 12), font_color_info_defaut);
	
	sprintf(text, "obj1 pos: (x, y, z) : %f, %f, %f", obj[0].pos.x, obj[0].pos.y, obj[0].pos.z);
	painter.putText( text, jhl_xy_i(0, 24), font_color_info_defaut);
//	text = "vp_near(f/v) : " + str(area_near);
	//painter.putText( text, jhl_xy_i(0, 36), font_color_info_defaut);
//	text = "vp_far (g/b) : " + str(area_far);
	//painter.putText( text, jhl_xy_i(0, 48), font_color_info_defaut);
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

void mat_test() {
	// homo4 * homo4


}
