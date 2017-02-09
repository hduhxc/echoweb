#include <glib.h>
#include "binary_tree.h"

BinaryTree* tree_new(BinaryTree* left,
					 BinaryTree* right,
				     gpointer data)
{
	BinaryTree* binary_tree = tree_alloc();
	binary_tree->left = left;
	binary_tree->right = right;
	binary_tree->data = data;

	return binary_tree;
}

void tree_insert(BinaryTree* parent,
				 BinaryTree* node,
				 gpointer data,
				 enum TreePos* pos,
				 GError** error)
{
	if (node == NULL) {
		g_assert(data != NULL);
		node = tree_new_single(data);
	}
	if (pos == LEFT) {
		g_assert(parent->left == NULL);
		parent->left = node;
	} else {
		g_assert(parent->right == NULL);
		parent->right = node;
	}
}

void tree_depth_traverse(BinaryTree* node,
						 enum TraverseOrder order,
						 TraverseFunc func)
{
	g_return_if_fail(func != NULL);

	if (order == ROOT_FIRST) {
		func(node);
	}
	if (node->left != NULL) {
		tree_depth_traverse(node->left, order, func);
	}
	if (order == ROOT_MEDIUM) {
		func(node);
	}
	if (node->right != NULL) {
		tree_depth_traverse(node->right, order, func);
	}
	if (order == ROOT_LAST) {
		func(node);
	}
}