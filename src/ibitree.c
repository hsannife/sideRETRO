#include "config.h"

#include <assert.h>
#include "wrapper.h"
#include "ibitree.h"

#define ibitree_left(node)  ((IBiTreeNode *)bitree_data (bitree_left ((node))))
#define ibitree_right(node) ((IBiTreeNode *)bitree_data (bitree_right ((node))))
#define ibitree_data(node)  ((IBiTreeNode *)bitree_data (node))

enum _IBiTreePos
{
	ROOT = 0,
	LEFT,
	RIGHT
};

typedef enum _IBiTreePos IBiTreePos;

static void
ibitree_rem_node (IBiTree *tree, BiTreeNode *node, IBiTreePos pos)
{
	if (tree == NULL
			|| (tree != NULL && tree->size == 0))
		return;

	BiTreeNode **position = node == NULL
		? &bitree_root (tree)
		: pos == LEFT
			? &bitree_left (node)
			: &bitree_right (node);

	if (*position != NULL)
		{
			ibitree_rem_node (tree, *position, LEFT);
			ibitree_rem_node (tree, *position, RIGHT);

			if (tree->destroy_fun != NULL)
				tree->destroy_fun (ibitree_data (*position)->data);

			xfree (ibitree_data (*position));
			xfree (*position);
			tree->size--;
		}
}

void
ibitree_free (IBiTree *tree)
{
	ibitree_rem_node (tree, NULL, ROOT);
	xfree (tree);
}

IBiTree *
ibitree_new (DestroyNotify destroy_fun)
{
	BiTree *tree = bitree_new (destroy_fun);
	return tree;
}

static inline IBiTreeNode *
ibitree_node_new (long low, long high, const void *data)
{
	IBiTreeNode *idata = xcalloc (1, sizeof (IBiTreeNode));

	idata->data = (void *) data;
	idata->low = low;
	idata->high = high;
	idata->max = high;
	idata->height = 1;

	return idata;
}

static inline int
max (int a, int b)
{
	return a > b
		? a
		: b;
}

static inline int
height (const BiTreeNode *node)
{
	return bitree_is_eob (node)
		? 0
		: ibitree_data (node)->height;
}

static inline int
balance_factor (const BiTreeNode *node)
{
	if (node == NULL)
		return 0;
	return height (bitree_left (node)) - height (bitree_right (node));
}

static void
left_rotate (BiTreeNode **node)
{
	BiTreeNode *right = bitree_right (*node);

	// Perform rotation
	bitree_right (*node) = bitree_left (right);
	bitree_left (right) = *node;

	//  Update heights
	ibitree_data (*node)->height = 1 + max (height (bitree_left (*node)),
			height (bitree_right (*node)));
	ibitree_data (right)->height = 1 + max (height (bitree_left (right)),
			height (bitree_right (right)));

	// Update root
	*node = right;
}

static void
right_rotate (BiTreeNode **node)
{
	BiTreeNode *left = bitree_left (*node);

	// Perform rotation
	bitree_left (*node) = bitree_right (left);
	bitree_right (left) = *node;

	//  Update heights
	ibitree_data (*node)->height = 1 + max (height (bitree_left (*node)),
			height (bitree_right (*node)));
	ibitree_data (left)->height = 1 + max (height (bitree_left (left)),
			height (bitree_right (left)));

	// Update root
	*node = left;
}

static void
insert (IBiTree *tree, BiTreeNode **node, IBiTreeNode *idata)
{
	// Handle insertion into an empty tree
	if (bitree_is_eob (*node))
		{
			bitree_ins_left (tree, *node, idata);
			return;
		}

	// Handle insertion into a tree that is not empty
	if (idata->low < ibitree_data (*node)->low)
		{
			if (bitree_is_eob (bitree_left (*node)))
				bitree_ins_left (tree, *node, idata);
			else
				insert (tree, &bitree_left (*node), idata);
		}
	else
		{
			if (bitree_is_eob (bitree_right (*node)))
				bitree_ins_right (tree, *node, idata);
			else
				insert (tree, &bitree_right (*node), idata);
		}

	//  Update max of this ancestor node
	if (ibitree_data (*node)->max < idata->high)
		ibitree_data (*node)->max = idata->high;

	//  Update height of this ancestor node
	ibitree_data (*node)->height = 1 + max (height (bitree_left (*node)),
			height (bitree_right (*node)));

	/*
	* Get the balance factor of this ancestor
	* node to check whether this node became
	* unbalance
	*/
	int balance = balance_factor (*node);

	if ((balance > 1) && (idata->low < ibitree_left (*node)->low))
		{
			// Left left case
			right_rotate (node);
		}
	else if ((balance < -1) && (idata->low > ibitree_right (*node)->low))
		{
			// right right case
			left_rotate (node);
		}
	else if ((balance > 1) && (idata->low > ibitree_left (*node)->low))
		{
			// Left right case
			left_rotate (&bitree_left (*node));
			right_rotate (node);
		}
	else if ((balance < -1) && (idata->low < ibitree_right (*node)->low))
		{
			// Right left case
			right_rotate (&bitree_right (*node));
			left_rotate (node);
		}
}

void
ibitree_insert (IBiTree *tree, long low, long high, const void *data)
{
	assert (tree != NULL && (high >= low));
	IBiTreeNode *idata = ibitree_node_new (low, high, data);
	insert (tree, &bitree_root (tree), idata);
}

static inline int
do_overlap (BiTreeNode *node, long low, long high)
{
	return (ibitree_data (node)->low <= high)
		&& (low <= ibitree_data (node)->high)
		? 1
		: 0;
}

static int
lookup (BiTreeNode *node, long low, long high,
		Func func, void *user_data, int *acm)
{
	if (bitree_is_eob (node))
		return 0;

	/*
	* If left child of node is present and max of left child is
	* greater than or equal to given interval, then i may
	* overlap with an interval is left subtree
	*/
	if (!bitree_is_eob (bitree_left (node))
			&& ibitree_left (node)->max >= low)
		lookup (bitree_left (node), low, high, func, user_data, acm);

	// If given interval overlaps with node
	if (do_overlap (node, low, high))
		{
			func (ibitree_data (node)->data, user_data);
			(*acm)++;
		}

	// Else interval can only overlap with right subtree
	if (!bitree_is_eob (bitree_right (node)))
		lookup (bitree_right (node), low, high, func, user_data, acm);
}

int
ibitree_lookup (IBiTree *tree, long low, long high, Func func, void *user_data)
{
	assert (tree != NULL && (high >= low) && func != NULL);
	int acm = 0;
	lookup (bitree_root (tree), low, high, func, user_data, &acm);
	return acm;
}