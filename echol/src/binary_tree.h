#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include <glib.h>

enum TreePos {
	LEFT,
	RIGHT
};
enum TraverseOrder {
	ROOT_FIRST,
	ROOT_MEDIUM,
	ROOT_LAST
};
typedef struct BinaryTree {
	struct BinaryTree* left;
	struct BinaryTree* right;
	gpointer data;
} BinaryTree;

#define tree_alloc() g_slice_new(BinaryTree)
#define tree_new_single(x) tree_new(NULL, NULL, x)

#define tree_insert_left(x, y, z) tree_insert(x, y, NULL, LEFT, z)
#define tree_insert_left_data(x, y, z) tree_insert(x, NULL, y, LEFT, z)
#define tree_insert_right(x, y, z) tree_insert(x, y, NULL, RIGHT, z)
#define tree_insert_right_data(x, y, z) tree_insert(x, NULL, y, RIGHT, z)

typedef void (*TraverseFunc)(BinaryTree*);

#define tree_traverse_root_first(x, func) tree_depth_traverse(x, ROOT_FIRST, func)
#define tree_traverse_root_medium(x, func) tree_depth_traverse(x, ROOT_MEDIUM, func)
#define tree_traverse_root_last(x, func) tree_depth_traverse(x, ROOT_LAST, func)

BinaryTree* tree_new(BinaryTree* left,
					 BinaryTree* right,
					 gpointer data);

void tree_insert(BinaryTree* parent,
				 BinaryTree* node,
				 gpointer data,
				 enum TreePos* pos,
				 GError** error);

void tree_depth_traverse(BinaryTree* node,
						 enum TraverseOrder order,
						 TraverseFunc func);
#endif