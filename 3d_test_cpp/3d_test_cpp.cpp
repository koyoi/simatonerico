#include "stdafx.h"

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "basic3dCalc.h"
#include "readData.h"

#include "hal_graphics.h"


const int window_v = 480;
const int window_h = 640;
const std::string data_file = { "C:\\Users\\N2232\\Documents\\vs_proj\\3dtest_cpp\\data\\dat.txt" };


#ifdef _WIN32_
#define print	printf

#endif



int
main(int argc, char *argv[])
{
	CvScalar font_color_info_defaut = CV_RGB( 0, 255, 255 );
	En_draw_type	draw_type = drawType_flat;

	jhl_xyz	obj_pos = { 3, 2, -20 };
	bool	obj_pos_changed = true;

	int		param1 = 0;
	int		param2 = 0;

	viewport_config	viewport_area = { 40,5,  -5,5, 5,-5 };
	viewport_area.top = viewport_area.right * window_h / window_v;
	viewport_area.btm = -viewport_area.top;

	bool	viewport_changed = true;

	double  frame_pause = 0;
	double	frame_time = 0.3;

	jhl_rgb light_ambient = { .15, .15, .15 };
	dir_light lights[] = {		// ���s�����@�����A�F�B�����́A���K�����ĂȂ��ƕs���ɂȂ邩��
		{ { 0, 1, 0}, {0, .7, .5 }},
		{ { 0.1, -.78, 0.2}, { .5, 0., 0. }} };


// ***************************************************************
	// 640 x 480 ��3���C���[(BGR)�ŐF�o�b�t�@�𐶐��E������
//	disp_init(window_v, window_h);

	modelData pol_model;
	(void)read_data(pol_model, data_file);

	print("read data");
	print("vertxes : ", pol_model.n_vert, " polygons : ", pol_model.n_pol, "\n" );

	jhl_rgb line_color = jhl_rgb(210, 150, 130);
	int line_width = 1;

	t = np.copy(i4) // �I�u�W�F�N�g�̕ό`�s��
	//t = trans(1, 2, 3)*t
	//t = scale(1.1)*t
	t = rot_axis_x(math.radians(2))*t
	t = rot_axis_y(math.radians(3.5))*t
	t = rot_axis_z(math.radians(4.8))*t
	//t = rot_by_vec(0, 0, 0, math.radians(5))*t


#if 0
		// todo �����܂� //
		r = trans(r_x, r_y, r_z)    // �I�u�W�F�N�g�̈ʒu
		print("r")
		print(r)

		eye = np.array([0., 0, 20])
		tgt = np.array([0., 0, 0])
		u = np.array([0., 1, 0])

		eye_vec = view_mat(eye, u, tgt)  // ���_
		print("eye")
		print(eye_vec)


		viewport_trans = disp_trans(window_h, window_v)
		print("viewport_trans")
		print(viewport_trans)
		viewport_trans_inv = np.linalg.inv(viewport_trans)   // �X�N���[�����W����A���̐��K�����W�ցB�e�N�X�`����������肷��̂Ɏg��


		cv2.namedWindow("img")//), cv2.WINDOW_KEEPRATIO)

		// �}�E�X�C�x���g���Ɋ֐�mouse_event�̏������s��
		//cv2.setMouseCallback("img", mouse_event)   �d�����A���������H

		cv2.imshow("img", img)

		font = cv2.FONT_HERSHEY_PLAIN
		font_size = 1

		time_elapsed = 0


		t_work = np.copy(t)   // �ό`�s��

		print("\"q\" to exit.")
		k = 1
		while (k != ord("q")) :
			k = cv2.waitKey(1) & 0xFF
			proc_key(k)
			if (frame_pause == 1) :
				time.sleep(0.1)
				continue    // ���̃��[�v��

				print("")
				print("frame", time_elapsed)

				if (area_changed == 1) :
					area_changed = 0
					//area = norm_ortho(area_size, -area_size, area_size*window_aspect, -area_size*window_aspect, area_far, area_near)
					area = norm_pers(area_size, -area_size, area_size*window_aspect, -area_size*window_aspect, area_far, area_near)
					print("area")
					print(area)

					if (r_moved == 1) :
						r_moved = 0;
	r = trans(r_x, r_y, r_z)    // �I�u�W�F�N�g�̈ʒu
		print("r")
		print(r)


		img.fill(0)
		img_z.fill(0)

		t_work = t.dot(t_work)       // ���f���̉�]�A�g��k��(�A�E�ړ�)�s��̍X�V

		temp = r.dot(t_work)         // ���f���̈ړ�
		temp = eye_vec.dot(temp)     // ���_�̎w��
		temp = area.dot(temp)        // ���e�ϊ�       // todo z �����K���͈͂���o�Ă��܂��H�H�@
		//    print("mat: obj trans")
		//    print(temp)
		//    print("vec: model points")
		//    print(points)
		poi_trans = trans_vec(points, temp) //���X�̕ϊ������f���Ɏ{��
		//    print("transed points")
		//   for v in pols :
	//       print(v)
		drawLines(3, img, poi_trans, polygons[0][2])
		//    drawLines(2, img, poi_trans, polygons[0][2])

		//    quit()

		// ui
		draw_info()

		cv2.imshow("img", img)
		cv2.imshow("img_z", img_z)

		time.sleep(frame_time)
		time_elapsed += 1

		cv2.destroyAllWindows()

		print("--- end ---")



		// ���������ɓh��Ԃ�
		cv::Mat red_img(cv::Size(640, 480), CV_8UC3, cv::Scalar(0, 0, 255));
	cv::Mat white_img(cv::Size(640, 480), CV_8UC3, cv::Scalar::all(255));
	cv::Mat black_img = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);

	// ��������ɓh��Ԃ�
	cv::Mat green_img = red_img.clone();
	green_img = cv::Scalar(0, 255, 0);

	cv::namedWindow("red image", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::namedWindow("white image", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::namedWindow("black image", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::namedWindow("green image", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("red image", red_img);
	cv::imshow("white image", white_img);
	cv::imshow("black image", black_img);
	cv::imshow("green image", green_img);
	cv::waitKey(0);

#endif
}



int proc_key(unsigned char key)
{
#if 0
	global r_x, r_y, r_z, r_moved
		global area_far, area_near, area_changed
		global draw_type
		global frame_pause

		// �J�����̕ω�
		if (k == ord("g")) :
			area_far += 1
			area_changed = 1
			if (area_far == area_near) :
				area_far -= 1
				elif(k == ord("b")) :
				area_far -= 1
				area_changed = 1
				if (area_far == area_near) :
					area_far += 1
					elif(k == ord("f")) :
					area_near += 1
					area_changed = 1
					if (area_far == area_near) :
						area_near -= 1
						elif(k == ord("v")) :
						area_near -= 1
						area_changed = 1
						if (area_far == area_near) :
							area_near += 1

							// �����_�����O�^�C�v�ύX
							elif(k == ord("t")) :
							draw_type += 1

							// �I�u�W�F�N�g�̈ʒu
							elif(k == ord("s")) :
							r_y += 1.
							r_moved = 1
							elif(k == ord("x")) :
							r_y -= 1.
							r_moved = 1
							elif(k == ord("z")) :
							r_x -= 1.
							r_moved = 1
							elif(k == ord("c")) :
							r_x += 1.
							r_moved = 1
							elif(k == ord("w")) :
							r_x = 0.
							r_y = 0.
							r_moved = 1

							elif(k == ord("p")) :
							if (frame_pause == 0) :
								frame_pause = 1
								print("pause. (p)")
							else :
								frame_pause = 0
#endif
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

#if 0
int mouse_event(event, x, y, flags, param)
{
	// ���N���b�N�ŐԂ��~�`�𐶐�
	if event == cv2.EVENT_LBUTTONUP:
	cv2.circle(img, (x, y), 50, (0, 0, 255), -1)
		// �E�N���b�N + Shift�L�[�ŗΐF�̃e�L�X�g�𐶐�
		elif event == cv2.EVENT_RBUTTONUP and flags & cv2.EVENT_FLAG_SHIFTKEY :
		cv2.putText(img, "CLICK!!", (x, y), cv2.FONT_HERSHEY_SIMPLEX, 5, (0, 255, 0), 3, cv2.CV_AA)

		// �E�N���b�N�݂̂Ő��l�p�`�𐶐�
		elif event == cv2.EVENT_RBUTTONUP :
		cv2.rectangle(img, (x - 100, y - 100), (x + 100, y + 100), (255, 0, 0), -1)

		print("mouse event")

		if event == cv2.EVENT_LBUTTONUP:
	if (x > window_h / 2) :
		if (y > window_v / 2) :
			param1 += 1.
		else :
			param1 -= 1.
	else:
	if (y > window_v / 2) :
		param2 += 1.
	else :
		param2 -= 1.

		print(x, y, param1, param2)
		print("mouse event")
}


#endif

