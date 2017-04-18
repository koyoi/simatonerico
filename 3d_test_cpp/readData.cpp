#include "stdafx.h"

#include <string>    // useful for reading and writing
#include <sstream>   // istringstream
#include <iostream>
#include <fstream>   // ifstream, ofstream

#include "basic3dCalc.h"
#include "jhl3dLib.h"


int read_data(modelData& model, std::string filename)
{
	std::ifstream reading_file(filename);
	if (reading_file.fail())
	{
		std::cout << "error: file not found/ open error:" << filename << std::endl;
		return 0;
	}

	std::string reading_line_buffer;

	std::string separated_string_buffer[3];
	char delimiter = ',';

	int group_target = 0;

	grpAttrib *grp_tgt;

	// todo エラー時の既確保の解放
	while ( std::getline(reading_file, reading_line_buffer) )
	{
		if (reading_line_buffer[0] == '#')
		{
			// コメント行
		}
		else if (reading_line_buffer[0] == ':')
		{
			if (reading_line_buffer == ":vertex")
			{
				reading_file >> model.n_vert;
				model.vert = new jhl_xyz[model.n_vert];
				if (model.vert == NULL)
				{
					std::cout << "buff allocate failed" << std::endl;
					return 0;
				}

				for ( int cnt_vert = 0; cnt_vert< model.n_vert; cnt_vert++ )
				{
					reading_file >> model.vert[cnt_vert].x >> delimiter >> model.vert[cnt_vert].y >> delimiter >> model.vert[cnt_vert].z;
				}
			}

			else if (reading_line_buffer == ":polygon")
			{
				reading_file >> model.n_pol;
				model.poldef = new pol_def[model.n_pol];
				if (model.poldef == NULL)
				{
					std::cout << "buff allocate failed" << std::endl;
					return 0;
				}

				for ( int cnt_pol = 0; cnt_pol < model.n_pol; cnt_pol++)
				{
					reading_file >> model.poldef[cnt_pol].a >> delimiter >> model.poldef[cnt_pol].b >> delimiter >> model.poldef[cnt_pol].c;
				}
			}

			else if (reading_line_buffer == ":groups")
			{
				reading_file >> model.n_group;
				model.attr = new grpAttrib[model.n_group];
				if (model.attr == NULL)
				{
					std::cout << "attrib buff allocate failed" << std::endl;
					return 0;
				}
			}

			else if (reading_line_buffer == ":group")
			{
				reading_file >> group_target;
				group_target--;
				grp_tgt = &model.attr[group_target];
				grp_tgt->pTex = NULL;
				grp_tgt->color = jhl_rgb(0,0,0);
			}

			else if (reading_line_buffer == "::color")
			{
				reading_file >> model.attr[group_target].color.r >> delimiter >> model.attr[group_target].color.g >> delimiter >> model.attr[group_target].color.b;
			}

			else if (reading_line_buffer == "::member")
			{
				reading_file >> grp_tgt->n_member;
				grp_tgt->member = new int[grp_tgt->n_member];
				if (model.attr == NULL)
				{
					std::cout << "attrib buff allocate failed" << std::endl;
					return 0;
				}

				/* カンマ区切り対応大変！
				// todo
				std::string split_temp, split_temp2;
				int pos;
				while (cnt_member < field_size)
				{
					std::getline(reading_file, split_temp);
					do {
						pos = split_temp.find_first_of(',');
						model.attr[group_target].member[cnt_member] = atoi(split_temp.substr(0, pos - 1).c_str() );
						split_temp = split_temp.substr(pos + 1);
						cnt_member++;
					} while (split_temp.size() != 0);
				}
				*/
				for (int cnt_member = 0; cnt_member < grp_tgt->n_member; cnt_member++)
				{
					reading_file >> grp_tgt->member[cnt_member];
				}
			}

			// todo 以下コピペしただけ
#if 0
			else if (reading_line_buffer == "::texName")
			{
				reading_file >> model.attr[group_target].texName;
			}

			else if (reading_line_buffer == "::UV")	// todo
			{
				reading_file >> field_size;
				model.attr = new grpAttrib[field_size];
				if (model.attr == NULL)
				{
					std::cout << "attrib buff allocate failed" << std::endl;
					return 0;
				}
			}
#endif
			else
			{
				std::cout << "error: file format error. unknown tag " << reading_line_buffer << ". abort." << std::endl;
				return 0;
			}
		}
	}
	return 1;
}
