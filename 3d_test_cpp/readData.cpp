#include "stdafx.h"

#include <string>    // useful for reading and writing
#include <sstream>   // istringstream
#include <iostream>
#include <fstream>   // ifstream, ofstream

#include "basic3dCalc.h"

#if 0
enum file_read_state{
	EN_FRS_init,
	EN_FRS_verts,
	EN_FRS_poldef,
	EN_FRS_group,	// １行しかないはず
	
	EN_FRS_color,	// 1行しかないはず
	EN_FRS_member,
	EN_FRS_uv,
};
#endif

int read_data(modelData& pol_model, std::string filename)
{
	std::ifstream reading_file(filename);
	if (reading_file.fail())
	{
		std::cout << "error: file not found/ open error:" << filename << std::endl;
		return 0;
	}

	modelData model;

	std::string reading_line_buffer;

	std::string separated_string_buffer[3];
	char delimiter = ',';

	int cnt_vert = 0;
	int cnt_pol = 0;
	int cnt_member = 0;
	int field_size = 0;
	int group_target = 0;

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

				while (cnt_vert < model.n_vert)
				{
					reading_file >> model.vert[cnt_vert].x >> delimiter >> model.vert[cnt_vert].y >> delimiter >> model.vert[cnt_vert].z;
					cnt_vert++;
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

				while (cnt_pol < model.n_pol)
				{
					reading_file >> model.poldef[cnt_pol].a >> delimiter >> model.poldef[cnt_pol].b >> delimiter >> model.poldef[cnt_pol].c;
					cnt_pol++;
					break;
				}
			}

			else if (reading_line_buffer == ":groups")
			{
				reading_file >> field_size;
				model.attr = new polAttrib[field_size];
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
			}

			else if (reading_line_buffer == "::color")
			{
				reading_file >> model.attr[group_target].color.r >> delimiter >> model.attr[group_target].color.g >> delimiter >> model.attr[group_target].color.b;
			}

			else if (reading_line_buffer == "::member")
			{
				reading_file >> field_size;
				model.attr[group_target].member = new int[field_size];
				if (model.attr == NULL)
				{
					std::cout << "attrib buff allocate failed" << std::endl;
					return 0;
				}
				cnt_member = 0;
				while (cnt_member < field_size)
				{
					reading_file >> model.attr[group_target].member[cnt_member];
					cnt_member++;
					// todo 複数
				}
			}

			else if (reading_line_buffer == "::texName")	// todo
			{
				reading_file >> model.attr[group_target].texName;
			}

			else if (reading_line_buffer == "::UV")	// todo
			{
				reading_file >> field_size;
				model.attr = new polAttrib[field_size];
				if (model.attr == NULL)
				{
					std::cout << "attrib buff allocate failed" << std::endl;
					return 0;
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
