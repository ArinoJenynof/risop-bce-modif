#include <memory>
#include <vector>
#ifndef BINARY_TREE_H_INCLUDED
#define BINARY_TREE_H_INCLUDED

struct binary_tree_node
{
	int cost;
	int index;
	std::vector<std::unique_ptr<binary_tree_node>> link;
	binary_tree_node *parent;
};

class binary_tree
{
public:
	std::unique_ptr<binary_tree_node> root;
	void insert(const int cost, const int index, std::vector<int> const &total_cost)
	{
		if (!root)
		{
			root.reset(new binary_tree_node);
			root->cost = cost;
			root->index = index;
			root->link = std::vector<std::unique_ptr<binary_tree_node>>(2);
			root->parent = nullptr;
		}
		else
		{
			binary_tree_node *parent = nullptr;
			binary_tree_node *it = root.get();
			int dir;
			while (it != nullptr)
			{
				dir = (cost > it->cost) || (cost == it->cost && total_cost.at(index) <= total_cost.at(it->index));
				// if (cost > it->cost)
				// 	dir = 1;
				// else if (cost == it->cost && total_cost.at(index) <= total_cost.at(it->index))
				// 	dir = 1;
				// else
				// 	dir = 0;
				parent = it;
				it = (it->link[dir]).get();
			}
			parent->link[dir].reset(new binary_tree_node);
			parent->link[dir]->cost = cost;
			parent->link[dir]->index = index;
			parent->link[dir]->link = std::vector<std::unique_ptr<binary_tree_node>>(2);
			parent->link[dir]->parent = parent;
		}
	}
};

#endif // BINARY_TREE_H_INCLUDED
