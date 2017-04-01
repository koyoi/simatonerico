#include <string>    // useful for reading and writing
#include <fstream>   // ifstream, ofstream
#include <sstream>   // istringstream

#include "basic3dCalc.h"

enum file_read_state{
	EN_FRS_init,
	EN_FRS_verts,
	EN_FRS_group,
};


int read_data(std::string filename,
	int vert_buff_size, jhl_xyz* verts[],
	int pol_buff_size, pol_def* pols[] )
{
	std::ifstream reading_file;
	reading_file.open(filename, std::ios::in);
	if (reading_file.fail())
	{
		printf("error: file not found/ open error:%s\n", filename );
		return -1;
	}

	file_read_state file_read_state = EN_FRS_init;

	std::string reading_line_buffer;
	std::istringstream line_separater(reading_line_buffer);

	std::string separated_string_buffer;
	const char delimiter = ',';

	int cnt_vert = 0;
	int cnt_pol = 0;


	while (!reading_file.eof())
	{
		std::getline(reading_file, reading_line_buffer);
		if (reading_line_buffer[0] == ':')
		{
			if (reading_line_buffer == ":points")
			{
				file_read_state = EN_FRS_verts;
			}
			else if (reading_line_buffer == ":group")
			{
				file_read_state = EN_FRS_group;
			}
			else {
				printf("error: file format error. unknown tag \"%s\". abort.", reading_line_buffer);
				return -1;
			}
		}
		else
		{
			switch (file_read_state)
			{
			case EN_FRS_init:
				printf("error: file format error. data before known tag. abort.");
				return -1;
				break;

			case EN_FRS_verts:
				if (cnt_vert >= vert_buff_size) {
					printf("error: vert buff is not enough. abort.");
					return -1;
				}
				std::getline(line_separater, separated_string_buffer, delimiter);
				verts[cnt_vert]->x = std::stod(separated_string_buffer);
				std::getline(line_separater, separated_string_buffer, delimiter);
				verts[cnt_vert]->y = std::stod(separated_string_buffer);
				std::getline(line_separater, separated_string_buffer, delimiter);
				verts[cnt_vert]->z = std::stod(separated_string_buffer);
				cnt_vert++;
				break;

			case EN_FRS_group:
				if (cnt_pol >= pol_buff_size) {
					printf("error: pol buff is not enough. abort.");
					return -1;
				}
				std::getline(line_separater, separated_string_buffer, delimiter);
				pols[cnt_pol]->a = std::stoi(separated_string_buffer);
				std::getline(line_separater, separated_string_buffer, delimiter);
				pols[cnt_pol]->b = std::stoi(separated_string_buffer);
				std::getline(line_separater, separated_string_buffer, delimiter);
				pols[cnt_pol]->c = std::stoi(separated_string_buffer);
				cnt_pol++;
				break;
			default:
				break;
			}
		}
	}
}
