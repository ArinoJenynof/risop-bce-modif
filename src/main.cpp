#include <iostream>
#include <fstream>
#include <vector>
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
	std::string jawabdir(".\\jawab");
	dp = opendir(soaldir.c_str());
	enum class row_status { SATISFIED, NOT_SATISFIED, EXCESS_ROW };
	while ((entry = readdir(dp)) != nullptr)
	{
		std::string name(entry->d_name);
		if (name == "." || name == "..")
			continue;
		std::ifstream inp(soaldir + '\\' + name);
		std::ofstream jwb(jawabdir + '\\' + name);
		std::string buffer;
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

		// Mulai
		const auto start = std::chrono::steady_clock::now();
		
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
				long long total_alloc = std::accumulate(allocation.at(i).begin(), allocation.at(i).end(), 0);
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
					int slc_row = least.at(rank.at(i).at(j) + 1).at(j);
					for (int k = 2; k < s - rank.at(i).at(j) && (status.at(slc_row) == row_status::SATISFIED || status.at(slc_row) == row_status::EXCESS_ROW); k++)
						slc_row = least.at(rank.at(i).at(j) + k).at(j);
					int slc = cost.at(slc_row).at(j);
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
			for (int i = 2; i < s - rank.at(selected_row).at(selected_column) && (status.at(slc_row) == row_status::SATISFIED || status.at(slc_row) == row_status::EXCESS_ROW); i++)
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

			// Cek status FLC
			int current_alloc = std::accumulate(allocation.at(selected_row).begin(), allocation.at(selected_row).end(), 0);
			if (current_alloc == supply.at(selected_row))
				status.at(selected_row) = row_status::SATISFIED;
			else if (current_alloc > supply.at(selected_row))
				status.at(selected_row) = row_status::EXCESS_ROW;
			else
				status.at(selected_row) = row_status::NOT_SATISFIED;

			// Cek status SLC
			current_alloc = std::accumulate(allocation.at(slc_row).begin(), allocation.at(slc_row).end(), 0);
			if (current_alloc == supply.at(slc_row))
				status.at(slc_row) = row_status::SATISFIED;
			else if (current_alloc > supply.at(slc_row))
				status.at(slc_row) = row_status::EXCESS_ROW;
			else
				status.at(slc_row) = row_status::NOT_SATISFIED;

			// Jika tidak ada ER, selesai
			finished = true;
			for (int i = 0; i < s; i++)
			{
				long long total_alloc = std::accumulate(allocation.at(i).begin(), allocation.at(i).end(), 0);
				if (total_alloc > supply.at(i))
				{
					finished = false;
					break;
				}
			}

			// Append ke buffer jawaban
			for (int i = 0; i < s; i++)
			{
				for (int j = 0; j < d; j++)
				{
					buffer += std::to_string(cost.at(i).at(j));
					if (allocation.at(i).at(j) != 0)
						buffer += " (" + std::to_string(allocation.at(i).at(j)) + ")";
					buffer += "\t\t";
				}
				buffer += std::to_string(supply.at(i)) + "\n";
			}
			for (const auto &i : demand)
				buffer += std::to_string(i) + "\t\t";
			buffer += "\n\n";
		}
		const auto end = std::chrono::steady_clock::now();
		const std::chrono::duration<double, std::milli> elapsed = end - start;
		long long obj_val = 0;
		for (int i = 0; i < s; i++)
			for (int j = 0; j < d; j++)
				obj_val += allocation.at(i).at(j) * cost.at(i).at(j);
		buffer += "Obj Val: " + std::to_string(obj_val) + "\n";
		buffer += "Runtime: " + std::to_string(elapsed.count()) + "ms\n";
		jwb << buffer;
	}

	return 0;
}
