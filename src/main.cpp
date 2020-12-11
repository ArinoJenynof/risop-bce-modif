#include <dirent.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <deque>

void update_least_cost(std::vector<unsigned> &flc, std::vector<unsigned> &slc, unsigned d);

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
		std::vector<long long> difference(d);
	}
	closedir(dp);

	return 0;
}
