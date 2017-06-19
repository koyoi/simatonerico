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
					return -1;
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
					return -2;
				}

				for ( int cnt_pol = 0; cnt_pol < model.n_pol; cnt_pol++)
				{
					reading_file >> model.poldef[cnt_pol][0] >> delimiter >> model.poldef[cnt_pol][1] >> delimiter >> model.poldef[cnt_pol][2];
				}
			}

			else if (reading_line_buffer == ":groups")	// グループがいくつあるか。
			{
				reading_file >> model.n_groups;
				model.group = new pol_group[model.n_groups];
				if (model.group == NULL )
				{
					std::cout << "attrib buff allocate failed" << std::endl;
					return -3;
				}
			}

			else if (reading_line_buffer == ":attrib_target")	// ターゲットのグループの色の塗り方
			{
				reading_file >> attr_target;
				reading_file >> temp_str_parse;

				if (attr_target >= model.n_groups)
				{
					std::cout << "data incorrect. too many groups are defined. ( file format error )" << std::endl;
					return -4;
				}

				if (temp_str_parse == "flat")
				{
					tgt_type = eATTR_FLAT;
					model.group[attr_target].attr_type = attr_flat;
					model.group[attr_target].attrib = new attrib_flat;
				}
				else if (temp_str_parse == "tex")
				{
					tgt_type = eATTR_TEX;
					model.group[attr_target].attr_type = attr_tex;
					model.group[attr_target].attrib = new attrib_tex;
				}
				else
				{
					std::cout << "unknown attribute type:\"" << temp_str_parse << "\"" << std::endl;
					return -5;
				}
			}
				
			else if (reading_line_buffer == "::member")	// 
			{
				int i;
				reading_file >> i;
				model.group[attr_target].n_member = i;	// ポリゴン番号;
				model.group[attr_target].member = new int[i];	// ポリゴン番号;
				if (model.group[attr_target].member == NULL)
				{
					std::cout << "attr_flat member buff allocate failed" << std::endl;
					return -6;
				}

				// 不定長のカンマ区切りをパースするのは大変そうなので改行区切り。
				// 移植を考えてvectorは使わない方がいいかもと
				for (int cnt_member = 0; cnt_member < i; cnt_member++)
				{
					reading_file >> model.group[attr_target].member[cnt_member];
				}
			}

			// FLAT のみ
			else if (reading_line_buffer == "::color")
			{
				if (tgt_type == eATTR_FLAT)
				{
					int	c[3];

					reading_file >> c[0] >> delimiter >> c[1] >> delimiter >> c[2];
					model.group[attr_target].color = c;
				}
				else
				{
					std::cout << "data format error. type tex does not have attrib \"color\"." << std::endl;
					return -7;
				}
			}

			else if (reading_line_buffer == "::texName")
			{
				if (tgt_type == eATTR_TEX)
				{
					attrib_tex *temp_attrib_tex = (attrib_tex*)model.group[attr_target].attrib;

					reading_file >> temp_attrib_tex->texName;	// とりあえず。
					std::string temp = basedir + "\\" + temp_attrib_tex->texName;
					const char* texname = temp.c_str();
					temp_attrib_tex->Tex = cvLoadImageM(texname);
					if(temp_attrib_tex->Tex == NULL )
					{
						std::cout << "tex file :" << texname <<"read failed"  << std::endl;
						// return -9;
					}
				}
				else
				{
					std::cout << "data format error. type flat does not have attrib \"texname\"." << std::endl;
					return -8;
				}
			}

			else if (reading_line_buffer == "::UVvtx")
			{
				// 要素数は既知
				attrib_tex *temp_attrib_tex = (attrib_tex*)model.group[attr_target].attrib;


				temp_attrib_tex->uv = new texUv[model.n_vert];
				if (temp_attrib_tex->uv == NULL)
				{
					std::cout << "uv vtx buff allocate failed" << std::endl;
					return -10;
				}
				for (int cnt = 0; cnt < model.n_vert; cnt++)
				{
					reading_file >> temp_attrib_tex->uv[cnt].u >> delimiter
						>> temp_attrib_tex->uv[cnt].v;
				}
			}
			else if (reading_line_buffer == "::UV")
			{
				attrib_tex *temp_attrib_tex = (attrib_tex*)model.group[attr_target].attrib;

				temp_attrib_tex->poldef = new pol_def[model.n_pol];
				if (temp_attrib_tex->poldef == NULL)
				{
					std::cout << "uv assign buff allocate failed" << std::endl;
					return -11;
				}
				for (int cnt = 0; cnt < model.n_pol; cnt++)
				{
					reading_file >> temp_attrib_tex->poldef[cnt][0] >> delimiter
						>> temp_attrib_tex->poldef[cnt][1] >> delimiter
						>> temp_attrib_tex->poldef[cnt][2];
				}
			}

			else
			{
				std::cout << "error: file format error. unknown tag " << reading_line_buffer << ". abort." << std::endl;
				return -12;
			}
		}
	}
	return 1;
}
