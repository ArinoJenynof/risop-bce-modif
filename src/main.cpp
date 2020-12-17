#include <dirent.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <limits>
#include <utility>

int main()
{
	struct dirent *entry = nullptr;
	DIR *dp = nullptr;

	std::string soaldir("E:\\Home\\Documents\\Kuliah\\Semester 7\\Riset Operasi\\37 data");
	dp = opendir(soaldir.c_str());
	while ((entry = readdir(dp)))
	{
		std::string name(entry->d_name);
		if (name == "." || name == "..")
			continue;
		std::ifstream inp(soaldir + '\\' + name);
		int s, d;
		inp >> s >> d;

		std::vector<std::vector<unsigned>> cost(s, std::vector<unsigned>(d));
		std::vector<unsigned> supply(s);
		std::vector<unsigned> demand(d);
		// 0 = Satisfied, 1 = Not satisfied, 2 = Excess row
		std::vector<unsigned> status(s, 1);

		// Isi cost dan supply dari input file
		for (auto &i : cost)
			for (auto &j : i)
				inp >> j;
		for (auto &i : supply)
			inp >> i;
		for (auto &i : demand)
			inp >> i;

		std::vector<unsigned> total_cost(s, 0);
		for (int i = 0; i < s; i++)
			for (int j = 0; j < d; j++)
				total_cost.at(i) += cost.at(i).at(j);

		// Cari index dari first least dan second least di setiap kolom
		std::vector<unsigned> indexof_flc(d);
		std::vector<unsigned> indexof_slc(d);
		for (int i = 0; i < d; i++)
		{
			std::deque<unsigned> possibles_flc;
			std::deque<unsigned> possibles_slc;
			int current_lowest = cost.at(0).at(i);
			possibles_flc.push_back(0);
			for (int j = 1; j < s; j++)
			{
				if (status.at(j) == 0)
					continue;
				if (cost.at(j).at(i) == current_lowest)
					possibles_flc.push_back(j);
				else if (cost.at(j).at(i) < current_lowest)
				{
					current_lowest = cost.at(j).at(i);
					possibles_slc = std::move(possibles_flc);
					possibles_flc.clear();
					possibles_flc.push_back(j);
				}
			}
			// Memilih first least cost untuk kolom/demand ke-i
			if (possibles_flc.size() > 1)
			{
				int current_highest = total_cost.at(possibles_flc.at(0));
				int selected = possibles_flc.at(0);
				for (size_t i = 1; i < possibles_flc.size(); i++)
				{
					if (total_cost.at(possibles_flc.at(i)) > current_highest)
					{
						current_highest = total_cost.at(possibles_flc.at(i));
						selected = possibles_flc.at(i);
					}
				}
				indexof_flc.at(i) = selected;
			}
			else
			{
				indexof_flc.at(i) = possibles_flc.at(0);
			}
			// Memilih second least cost untuk kolom/demand ke-i
			if (possibles_slc.size() > 1)
			{
				int current_highest = total_cost.at(possibles_slc.at(0));
				int selected = possibles_slc.at(0);
				for (size_t i = 1; i < possibles_slc.size(); i++)
				{
					if (total_cost.at(possibles_slc.at(i)) > current_highest)
					{
						current_highest = total_cost.at(possibles_slc.at(i));
						selected = possibles_slc.at(i);
					}
				}
				indexof_slc.at(i) = selected;
			}
			else
			{
				indexof_slc.at(i) = possibles_slc.at(0);
			}
		}

		// Alokasi di setiap least cost sebanyak demandnya
		std::vector<std::vector<unsigned>> allocation(s, std::vector<unsigned>(d));
		for (int i = 0; i < d; i++)
			allocation.at(indexof_flc.at(i)).at(i) = demand.at(i);
		for (int i = 0; i < s; i++)
		{
			// Tandai baris yang ER
			int sum = 0;
			for (const auto &j : allocation.at(i))
				sum += j;
			if (sum > supply.at(i))
				status.at(i) = 2;
		}

		// Hitung difference pada ER
		std::vector<std::vector<long long>> difference(s, std::vector<long long>(d));
		for (unsigned i = 0; i < s; i++)
		{
			if (status.at(i) != 2)
				continue;

			for (unsigned j = 0; j < d; j++)
			{
				if (allocation.at(i).at(j) != 0)
				{
					difference.at(i).at(j) = cost.at(indexof_slc.at(j)).at(j) - cost.at(indexof_flc.at(j)).at(j);
				}
			}
		}

		// Cari sel yang akan dipindah, i.e. diff terkecil
		std::deque<std::pair<unsigned, unsigned>> possibles_coord;
		unsigned selected_x, selected_y;
		for (unsigned i = 0; i < s; i++)
		{
			if (status.at(i) != 2)
				continue;
			
			long long smallest_diff = std::numeric_limits<long long>::max();
			for (unsigned j = 0; j < d; j++)
			{
				if (difference.at(i).at(j) != 0 && difference.at(i).at(j) <= smallest_diff)
				{
					smallest_diff = difference.at(i).at(j);
					while (difference.at(possibles_coord.front().first).at(possibles_coord.front().second) > smallest_diff)
						possibles_coord.pop_front();
					possibles_coord.push_back(std::make_pair(i, j));
				}
			}
		}
		if (possibles_coord.size() > 1)
		{
			unsigned smallest_demand = std::numeric_limits<unsigned>::max();
			for (auto &i : possibles_coord)
			{
				if (demand.at(i.second) < smallest_demand)
				{
					smallest_demand = demand.at(i.second);
					selected_x = i.first;
					selected_y = i.second;
				}
			}
		}
		else
		{
			selected_x = possibles_coord.front().first;
			selected_y = possibles_coord.front().second;
		}
		
	}
	closedir(dp);

	return 0;
}
