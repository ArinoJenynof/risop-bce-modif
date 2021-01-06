#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <limits>
#include <utility>
#include <chrono>
#include <stack>
#include <numeric>
#include <dirent.h>
#include "binary_tree.hpp"

int main()
{
	dirent *entry = nullptr;
	DIR *dp = nullptr;
	std::string soaldir(".\\soal");
	dp = opendir(soaldir.c_str());
	enum class row_status { SATISFIED, NOT_SATISFIED, EXCESS_ROW };
	std::ofstream out(".\\res.txt");
	while ((entry = readdir(dp)) != nullptr)
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
		std::vector<row_status> status(s, row_status::NOT_SATISFIED);

		// Isi cost, supply, dan demand dari input file
		for (auto &i : cost)
			for (auto &j : i)
				inp >> j;
		for (auto &i : supply)
			inp >> i;
		for (auto &i : demand)
			inp >> i;

		// Cek apakah supply == demand
		long long total_supply = std::accumulate(supply.begin(), supply.end(), 0);
		long long total_demand = std::accumulate(demand.begin(), demand.end(), 0);
		if (total_supply > total_demand)
		{
			for (auto &i : cost)
				i.push_back(0);
			demand.push_back(total_supply - total_demand);
			d++;
		}
		else if (total_supply < total_demand)
		{
			cost.push_back(std::vector<int>(d));
			supply.push_back(total_demand - total_supply);
			s++;
		}

		// Hitung total cost setiap baris
		std::vector<int> total_cost(s, 0);
		for (int i = 0; i < s; i++)
			for (int j = 0; j < d; j++)
				total_cost.at(i) += cost.at(i).at(j);

		// Prekalkulasi posisi masing-masing cost dalam kolom
		std::vector<std::vector<int>> rank(s, std::vector<int>(d));
		std::vector<std::vector<int>> least(s, std::vector<int>(d));
		for (int i = 0; i < d; i++)
		{
			binary_tree tree;
			for (int j = 0; j < s; j++)
				tree.insert(cost.at(j).at(i), j, total_cost);
			std::vector<std::pair<int, int>> sorted;

			// Traverse untuk mendapatkan sorted cost
			std::stack<binary_tree_node*> iter_stack;
			binary_tree_node *it = tree.root.get();
			while (!iter_stack.empty() || it != nullptr)
			{
				if (it != nullptr)
				{
					iter_stack.push(it);
					it = (it->link[0]).get();
				}
				else
				{
					it = iter_stack.top();
					iter_stack.pop();
					sorted.push_back(std::make_pair(it->cost, it->index));
					it = (it->link[1]).get();
				}
			}
			for (size_t j = 0; j < sorted.size(); j++)
			{
				rank.at(sorted[j].second).at(i) = j;
				least.at(j).at(i) = sorted[j].second;
			}
		}

		// Alokasi demand pada cost terkecil setiap kolom
		std::vector<std::vector<int>> allocation(s, std::vector<int>(d));
		for (int i = 0; i < d; i++)
		{
			allocation.at(least.at(0).at(i)).at(i) = demand.at(i);
		}

		bool finished = false;
		while (!finished)
		{
			// Tandai supply yang ER
			for (int i = 0; i < s; i++)
			{
				long long total_alloc = 0;
				for (int j = 0; j < d; j++)
					total_alloc += allocation.at(i).at(j);
				if (total_alloc > supply.at(i))
					status.at(i) = row_status::EXCESS_ROW;
			}

			// Untuk setiap baris ER, hitung diff pada sel yang dialokasikan
			int selected_row = 0, selected_column = 0;
			long long smallest_diff = std::numeric_limits<long long>::max();
			for (int i = 0; i < s; i++)
			{
				if (status.at(i) != row_status::EXCESS_ROW)
					continue;

				for (int j = 0; j < d; j++)
				{
					if (allocation.at(i).at(j) == 0)
						continue;

					int flc = cost.at(i).at(j);
					int slc = cost.at(least.at(rank.at(i).at(j) + 1).at(j)).at(j);
					long long diff = slc - flc;
					if (diff < smallest_diff)
					{
						smallest_diff = diff;
						selected_row = i;
						selected_column = j;
					}
					else if (diff == smallest_diff && demand.at(j) < demand.at(selected_column))
					{
						smallest_diff = diff;
						selected_row = i;
						selected_column = j;
					}
				}
			}

			// Cari nilai12 dari flc dan slc
			int slc_row = least.at(rank.at(selected_row).at(selected_column) + 1).at(selected_column);
			for (int i = 2; i < d && status.at(slc_row) == row_status::SATISFIED; i++)
				slc_row = least.at(rank.at(selected_row).at(selected_column) + i).at(selected_column);
			long long nilai_flc = (total_cost.at(selected_row) - d * cost.at(selected_row).at(selected_column)) * cost.at(selected_row).at(selected_column);
			long long nilai_slc = (total_cost.at(slc_row) - d * cost.at(slc_row).at(selected_column)) * cost.at(slc_row).at(selected_column);

			// Tentukan mana yang harus disatisfy dulu
			if (nilai_flc >= nilai_slc)
			{
				int dipindah = std::accumulate(allocation.at(selected_row).begin(), allocation.at(selected_row).end(), 0) - supply.at(selected_row);
				dipindah = std::min(dipindah, allocation.at(selected_row).at(selected_column));
				allocation.at(selected_row).at(selected_column) -= dipindah;
				allocation.at(slc_row).at(selected_column) += dipindah;
			}
			else if (nilai_slc > nilai_flc)
			{
				int dipindah = supply.at(slc_row) - std::accumulate(allocation.at(slc_row).begin(), allocation.at(slc_row).end(), 0);
				dipindah = std::min(dipindah, allocation.at(selected_row).at(selected_column));
				allocation.at(selected_row).at(selected_column) -= dipindah;
				allocation.at(slc_row).at(selected_column) += dipindah;
			}

			// Update status supply
			finished = true;
			for (int i = 0; i < s; i++)
			{
				long long total_alloc = std::accumulate(allocation.at(i).begin(), allocation.at(i).end(), 0);
				if (total_alloc > supply.at(i))
				{
					status.at(i) = row_status::EXCESS_ROW;
					finished = false;
				}
				else if (total_alloc == supply.at(i))
					status.at(i) = row_status::SATISFIED;
			}
		}
		std::cout << name << '\n';
		for (int i = 0; i < s; i++)
		{
			for (int j = 0; j < d; j++)
			{
				std::cout << cost.at(i).at(j);
				if (allocation.at(i).at(j) != 0)
					std:: cout << " (" << allocation.at(i).at(j) << ')';
				std::cout << '\t';
			}
			std::cout << supply.at(i) << '\n';
		}
		std::cout << '\t';
		for (const auto &i : demand)
			std::cout << i << '\t';
		long long obj_val = 0;
		for (int i = 0; i < s; i++)
			for (int j = 0; j < d; j++)
				obj_val += allocation.at(i).at(j) * cost.at(i).at(j);
		std::cout << "\nTotal: " << obj_val << "\n\n";
		out << obj_val << '\n';
	}

	return 0;
}
