#include "stdafx.h"

#include "basic3dCalc.h"
#include "readData.h"

#if 0
int drawLines(draw_type, im, points, polygons)
{

	points_disp = trans_disp(points)  // ディスプレイ座標へ
	//        print("viewport points")
	//        print(points_disp)

		if (draw_type == 1) : // ワイヤフレーム（影面消去なし)
			for vecs in points_disp :
	t.append(tuple(map(int, vecs[0:2]))) // cv2.line が整数しか受け付けない
		for d in polygons :
	im = cv2.line(im, t[d[0]], t[d[1]], line_color, line_width)
		im = cv2.line(im, t[d[1]], t[d[2]], line_color, line_width)
		im = cv2.line(im, t[d[2]], t[d[0]], line_color, line_width)

		elif(draw_type == 2) : // ワイヤフレーム、影面消去あり
		pols_drawn = 0
		for vecs in points_disp :
	t.append(tuple(map(int, vecs[0:2]))) // cv2.line が整数しか受け付けない
		for d in polygons :
	if (check_side((points_disp[d[0]], points_disp[d[1]], points_disp[d[2]])) > 0) :
		//                print(line_color)
		//                print(t[d[0]], t[d[1]], t[d[2]])
		im = cv2.line(im, t[d[0]], t[d[1]], line_color, line_width)
		im = cv2.line(im, t[d[1]], t[d[2]], line_color, line_width)
		im = cv2.line(im, t[d[2]], t[d[0]], line_color, line_width)
		pols_drawn += 1
		print("polygons drawn: ", pols_drawn)

		elif(draw_type == 3) : // 単色ポリゴン（光源あり）
		z_min = 255
		z_max = 0
		z_calc = 0

		pols_drawn = 0
		light_color = []
		for d in polygons :
	pd0 = points_disp[d[0]] // 今注目しているポリゴンの各頂点   Points_disp[Dn]
		pd1 = points_disp[d[1]]
		pd2 = points_disp[d[2]]


		// 裏面は描画しない
		if (check_side((pd0, pd1, pd2)) > 0) :
			// (視線との角度が必要なら、そのとき... シェーダ案件？)
			light_color = (calc_lighting((points[d[0]], points[d[1]], points[d[2]]))) * 255
			light_color_int = (int(light_color[0]), int(light_color[1]), int(light_color[2]))
			//                light_color_int = map(int, light_color.flatten())
			//                print("light_color_int", light_color_int)
			// y ソート（ X方向　･･･VRAM アドレスインクリメントでアクセスしたいため
			//                print("points_disp")
			//                for ppp in d :
		//                    print ppp
		//                print pd0
		//                print pd1
		//                print pd2
		// todo sorted と言うメソッドがあるぞ
			if (pd0[1] <= pd1[1] and pd0[1] <= pd2[1]) :
				idx0 = d[0]
				if (pd1[1] <= pd2[1]) :
					idx1 = d[1]
					idx2 = d[2]
				else :
					idx1 = d[2]
					idx2 = d[1]
					elif(pd1[1] <= pd0[1] and pd1[1] <= pd2[1]) :
					idx0 = d[1]
					if (pd0[1] <= pd2[1]) :
						idx1 = d[0]
						idx2 = d[2]
					else :
						idx1 = d[2]
						idx2 = d[0]
			else:
	idx0 = d[2]
		if (pd0[1] <= pd1[1]) :
			idx1 = d[0]
			idx2 = d[1]
		else :
			idx1 = d[1]
			idx2 = d[0]
			//                print("sorted")
			//                print(idx0, idx1, idx2)
			//                print(points_disp[idx0], points_disp[idx1], print(points_disp[idx2])

				// ディスプレイ上で一ラインごとに舐める
				// todo 分母ゼロの対応

			pd0s = points_disp[idx0] // 塗りつぶしの都合で y ソートした    PointsDispSorted
			pd1s = points_disp[idx1]
			pd2s = points_disp[idx2]

			a1s = (pd1s[0] - pd0s[0]) / (pd1s[1] - pd0s[1] + 0.00001)   // todo toria ゼロ除算対策
			a2s = (pd2s[0] - pd0s[0]) / (pd2s[1] - pd0s[1] + 0.00001)
			a3s = (pd2s[0] - pd1s[0]) / (pd2s[1] - pd1s[1] + 0.00001)
			//                print("katamuki", a1s, a2s, a3s)

			// p0 - p1 までyを増やしながら、１ラインずつ線を引いて塗る ... zを考えなくていいからいろいろ省略
			for y_ in range(int(pd0s[1]) + 1, int(pd1s[1])) :
				x0s = pd0s[0] + a1s * (y_ - pd0s[1])   // pd0s[1] : 初期値
				x1s = pd0s[0] + a2s * (y_ - pd0s[1])
				im = cv2.line(im, (int(x0s), y_), (int(x1s), y_), light_color_int, line_width)
				//                print(",")

	//// ゴミ、はみ出し防止（int丸めのため)
				x0s = pd1s[0]
				x1s = pd0s[0] + a2s * (pd1s[1] - pd0s[1])
				y_ = int(pd1s[1])
				im = cv2.line(im, (int(x0s), y_), (int(x1s), y_), light_color_int, line_width)

				// p1 - p2 までyを増やしながら、１ラインずつ線を引いて塗る
				for y_ in range(int(pd1s[1]) + 1, int(pd2s[1])) :
					x0s = pd1s[0] + a3s * (y_ - pd1s[1])
					x1s = pd0s[0] + a2s * (y_ - pd0s[1])
					im = cv2.line(im, (int(x0s), y_), (int(x1s), y_), light_color_int, line_width)
					//                    img_z = cv2.line(img_z, (int(x0s), y_), (int(x1s), y_), light_color_int[0], line_width)

					// z map 作成 //
					p0s = points[idx0]
					p1s = points[idx1]
					p2s = points[idx2]
					//                print("triangle_orig")
					//                print(p0s, ",", ",", p1s, ",", ",", p2s)
					//                print("triangle_disp")
					//                print(pd0s, ",", pd1s, ",", pd2s)

					//                print("")
					//1                print("t0_wari,t0_x,y,z,,t1_wari,t1_x,y,z,, scr_y, scr_x1_t,x2_t")

					// idx0 ～ idx1 の、元座標での t : (1 - t) に分周する点
					t0s_all = pd1s[1] - pd0s[1]   // スクリーン上でyだけ、走査する（ｘはすぐしたのループ内）
					t1s_all = pd2s[1] - pd0s[1]   // 正規化空間の中での位置が知るため、スクリーン上でのその点による分割割合が知りたい
					for t0s in range(int(t0s_all)) :
						// クリーン上で水平線を引いてきて、xに平行に引いて、ｙをなめるとき、p0 - p1上である始点, p0 - p2上である終点の元座標
						pt0 = proj_norm(t0s / t0s_all, p0s, p1s)
						pt0.append(1.)
						pt1 = proj_norm(t0s / t1s_all, p0s, p2s)
						pt1.append(1.)
						//1                    print(t0s / t0s_all, pt0[0:3], "", t0s / t1s_all, pt1[0:3], "", pd0s[1] + t0s)
						// 直線 pt0 - pt1 をスクリーン上で１ピクセルずつ、元座標はなんなのかチェック
						// するために、分割点 t0s での、x幅を計算
						temp0 = (viewport_trans.dot((np.matrix(pt0).T)))[0] // x しかいらない
						temp1 = (viewport_trans.dot((np.matrix(pt1).T)))[0]

						x_start = int(min(temp0, temp1))
						x_end = int(max(temp0, temp1))

						x_all = x_end - x_start
						//2                    print("x scan (disp) :", x_start, " - ", x_end)
						//2                    print("wari_x, norm_x,y,z,disp_x,y,z")
						for x_work in range(x_all) : // スクリーン上で1pixずつ
							z_calc += 1
							pt_scan_wari = (x_work / x_all)
							pix_norm = proj_norm(pt_scan_wari, pt0, pt1)
							//2                        print(pt_scan_wari, pix_norm, trans_disp([pix_norm]))

							//                        print(int(z_buff_test[1]), int(z_buff_test[0]), ((z_buff_test[2] + 1) / 2) * 255)
							//                        print("pos: ", x_start + x_work, pd0s[1] + t0s)
							if (pix_norm[2] <= 1 and pix_norm[2] >= -1) :
								z_buff_test = trans_disp([pix_norm])[0]
								//                            z_temp = int(((pix_norm[2] + 1) / 2) * 255)
								z_temp = int((((pix_norm[2] + 1) / 2) * 255 - 251) * 50) // debug 範囲を変えてみる
								//                            print(z_temp)
								z_min = min(z_min, z_temp)
								z_max = max(z_max, z_temp)
								if (int(img_z[int(pd0s[1] + t0s)][int(x_start + x_work)]) < z_temp) :
									//                                print(int(pd0s[1] + t0s), int(x_start + x_work), ")", int(img_z[int(pd0s[1] + t0s), int(x_start + x_work)]), "<= ", z_temp)
									img_z[int(pd0s[1] + t0s)][int(x_start + x_work)] = z_temp
								else:
	print(int(pd0s[1] + t0s), int(x_start + x_work), ")", int(img_z[int(pd0s[1] + t0s), int(x_start + x_work)]), )


		// z map ここまで //





		pols_drawn += 1
		print("polygons drawn: ", pols_drawn)
		print("z_min,max", z_min, z_max)
		//        print("// z calc", z_calc)
							else:
	draw_type = 1
#if 0
		elif(draw_type == 0) :
		x = 0
		elif(draw_type == 2) :
		x = 0
#endif
}

#endif