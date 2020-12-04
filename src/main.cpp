#include <dirent.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <deque>

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

		std::vector<std::vector<int>> cost(s, std::vector<int>(d));
		std::vector<int> supply(s);
		std::vector<int> demand(d);
		std::vector<int> status(s, 1);

		for (auto &i : cost)
			for (auto &j : i)
				inp >> j;
		for (auto &i : supply)
			inp >> i;
		for (auto &i : demand)
			inp >> i;

		std::vector<int> total_cost(s);
		for (int i = 0; i < s; i++)
			for (int j = 0; j < d; j++)
				total_cost.at(i) += cost.at(i).at(j);

		std::vector<int> indexof_lc(d);
		for (int i = 0; i < d; i++)
		{
			std::deque<int> possibles;
			int current_lowest = cost.at(0).at(i);
			possibles.push_back(0);
			for (int j = 1; j < s; j++)
			{
				if (status.at(j) == 0)
					continue;
				if (cost.at(j).at(i) <= current_lowest)
				{
					current_lowest = cost.at(j).at(i);
					possibles.push_back(j);
					while (cost.at(possibles.front()).at(i) > current_lowest)
						possibles.pop_front();
				}
			}
			if (possibles.size() > 1)
			{
				int current_highest = total_cost.at(possibles.at(0));
				int selected = possibles.at(0);
				for (size_t i = 1; i < possibles.size(); i++)
				{
					if (total_cost.at(possibles.at(i)) > current_highest)
					{
						current_highest = total_cost.at(possibles.at(i));
						selected = possibles.at(i);
					}
				}
				indexof_lc.at(i) = selected;
			}
			else
			{
				indexof_lc.at(i) = possibles.at(0);
			}
		}

		std::vector<std::vector<int>> allocation(s, std::vector<int>(d));
		for (int i = 0; i < d; i++)
			allocation.at(indexof_lc.at(i)).at(i) = demand.at(i);
		for (int i = 0; i < s; i++)
		{
			int sum = 0;
			for (const auto &j : allocation.at(i))
				sum += j;
			if (sum > supply.at(i))
				status.at(i) = 2;
		}
	}
	closedir(dp);

	return 0;
}