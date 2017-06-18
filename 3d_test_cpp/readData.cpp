#include "stdafx.h"

/*
	実質、windows + opencv 専用

	xml にするか...
*/

#include <string>    // useful for reading and writing
#include <sstream>   // istringstream
#include <iostream>
#include <fstream>   // ifstream, ofstream

#include "basic3dCalc.h"
#include "jhl3dLib.h"


int read_and_perse_data(modelData& model, std::string basedir, std::string filename)
{
	std::string temp_file_full_path = basedir + "\\" + filename;
	std::ifstream reading_file(temp_file_full_path);
	if (reading_file.fail( ))
	{
		char temp;
		std::cout << "error: file not found/ open error:" << temp_file_full_path << std::endl;
		std::cin >> temp;
		return 0;
	}

	std::string reading_line_buffer;

	char delimiter = ',';

	int attr_target = 0;
	std::string temp_str_parse;
	attr_type tgt_type = eATTR_FLAT;

	// todo エラー時の既確保の解放
	while ( std::getline(reading_file, reading_line_buffer) )
	{
		if (reading_line_buffer[0] == '#')		// コメント
		{
			// コメント行
		}
		else if (reading_line_buffer[0] == ':')
		{
			if (reading_line_buffer == ":version")	// ver
			{
				int i;
				reading_file >> i;
				if (i != 3)
				{
					std::cout << "データファイルの形式が対応していないバージョンっぽいです。" << std::endl;
					return 0;
				}
			}
			else if (reading_line_buffer == ":name")	// モデル名（デバッグとかに便利かもと
			{
				reading_file >> model.name;
			}
			else if (reading_line_buffer == ":vertex")	// 頂点座標
			{
				reading_file >> model.n_vert;
				model.verts = new jhl_xyz[model.n_vert];
				if (model.verts == NULL)
				{
					std::cout << "vert buff allocate failed" << std::endl;
					return 0;
				}

				for ( int cnt_vert = 0; cnt_vert< model.n_vert; cnt_vert++ )
				{
					reading_file >> model.verts[cnt_vert].x >> delimiter >> model.verts[cnt_vert].y >> delimiter >> model.verts[cnt_vert].z;
				}
			}

			else if (reading_line_buffer == ":polygon")	// ポリゴン定義(頂点のインデックス)
			{
				reading_file >> model.n_pol;
				model.poldef = new pol_def[model.n_pol];
				if (model.poldef == NULL)
				{
					std::cout << "pol buff allocate failed" << std::endl;
					return 0;
				}

				for ( int cnt_pol = 0; cnt_pol < model.n_pol; cnt_pol++)
				{
					reading_file >> model.poldef[cnt_pol].a >> delimiter >> model.poldef[cnt_pol].b >> delimiter >> model.poldef[cnt_pol].c;
				}
			}

			else if (reading_line_buffer == ":attributes")	// グループがいくつあるか。flat, tex
			{
				reading_file >> model.n_attr_flat >> delimiter >> model.n_attr_tex;

				model.attr_flat = new attrib_flat[model.n_attr_flat];
				model.attr_tex = new attrib_tex[model.n_attr_tex];
				if (model.attr_flat == NULL || model.attr_tex == NULL )
				{
					std::cout << "attrib buff allocate failed" << std::endl;
					return 0;
				}
			}

			else if (reading_line_buffer == ":attrib_target")	// ターゲットのグループの色の塗り方
			{
				reading_file >> temp_str_parse;
				reading_file >> attr_target;

				if (temp_str_parse == "flat")
				{
					tgt_type = eATTR_FLAT;
					if (attr_target >= model.n_attr_flat)
					{
						std::cout << "data incorrect. # of attrib \"flat\" exceeds decrared num " << model.n_attr_flat << std::endl;
						return 0;
					}
				}
				else if (temp_str_parse == "tex")
				{
					tgt_type = eATTR_TEX;
					if (attr_target >= model.n_attr_tex)
					{
						std::cout << "data incorrect. # of attrib \"tex\" exceeds decrared num " << model.n_attr_tex << std::endl;
						return 0;
					}
				}
				else
				{
					std::cout << "unknown attribute type:\"" << temp_str_parse << "\"" << std::endl;
					return 0;
				}
			}
				
			// FLAT のみ
			else if (reading_line_buffer == "::color")
			{
				if (tgt_type == eATTR_FLAT)
				{
					int	c[3];

					reading_file >> c[0] >> delimiter >> c[1] >> delimiter >> c[2];
					model.attr_flat[attr_target].color = c;
				}
			}

			else if (reading_line_buffer == "::member")	// 
			{
				if (tgt_type == eATTR_FLAT)
				{
					int i;
					reading_file >> i;
					model.attr_flat[attr_target].n_member = i;
					model.attr_flat[attr_target].member = new int[i];	// ポリゴン番号
					if (model.attr_flat[attr_target].member == NULL)
					{
						std::cout << "attr_flat member buff allocate failed" << std::endl;
						return 0;
					}

					// 不定長のカンマ区切りをパースするのは大変そう
					for (int cnt_member = 0; cnt_member < i; cnt_member++)
					{
						reading_file >> model.attr_flat[attr_target].member[cnt_member];
					}
				}
				else
				{
					std::cout << "parse error" << __FILE__ << " " << __LINE__ << std::endl;
					return 0;
				}
			}

			else if (reading_line_buffer == "::texName")
			{
				if (tgt_type == eATTR_TEX)
				{
					reading_file >> model.attr_tex[attr_target].texName;	// とりあえず。
					std::string temp = basedir + "\\" + model.attr_tex[attr_target].texName;
					const char* texname = temp.c_str();
					model.attr_tex[attr_target].Tex = cvLoadImageM(texname);
					if(model.attr_tex[attr_target].Tex == NULL )
					{
						std::cout << "tex file :" << texname <<"read failed"  << std::endl;
						// return 0;
					}
				}
			}

			else if (reading_line_buffer == "::UVvtx")
			{
				// 要素数は既知
				model.attr_tex[attr_target].uv = new texUv[model.n_vert];
				if (model.attr_tex[attr_target].uv == NULL)
				{
					std::cout << "uv vtx buff allocate failed" << std::endl;
					return 0;
				}
				for (int cnt = 0; cnt < model.n_vert; cnt++)
				{
					reading_file >> model.attr_tex[attr_target].uv[cnt].u >> delimiter
						>> model.attr_tex[attr_target].uv[cnt].v;
					printf("%d,%d\n", model.attr_tex[attr_target].uv[cnt].u, model.attr_tex[attr_target].uv[cnt].v);
				}
			}
			else if (reading_line_buffer == "::UV")
			{
				model.attr_tex[attr_target].poldef = new pol_def[model.n_pol];
				if (model.attr_tex[attr_target].poldef == NULL)
				{
					std::cout << "uv assign buff allocate failed" << std::endl;
					return 0;
				}
				for (int cnt = 0; cnt < model.n_pol; cnt++)
				{
					reading_file >> model.attr_tex[attr_target].poldef[cnt].a >> delimiter
						>> model.attr_tex[attr_target].poldef[cnt].b >> delimiter
						>> model.attr_tex[attr_target].poldef[cnt].c;
				}
			}

			else
			{
				std::cout << "error: file format error. unknown tag " << reading_line_buffer << ". abort." << std::endl;
				return 0;
			}
		}
	}
	return 1;
}
