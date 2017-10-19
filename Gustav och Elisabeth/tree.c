#include <assert.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "tree.h"


typedef struct node node_t;

node_t *tree_traverse(tree_t *tree, tree_key_t key);

//////////////////////////
// AVL-Header functions //
//////////////////////////

void balance_tree(tree_t *tree);
void balance_sub_nodes(element_comp_fun elem_cmp, node_t *node);
void balance_node(element_comp_fun elem_cmp, node_t *node);

int node_depth(node_t *node);
int node_balance(node_t *node);
void right_rotate(element_comp_fun elem_cmp, node_t *node);
void left_rotate(element_comp_fun elem_cmp, node_t *node);
void left_right_rotate(element_comp_fun elem_cmp, node_t *node);
void right_left_rotate(element_comp_fun elem_cmp, node_t *node);

//////////////////////
// Normal functions //
//////////////////////

//Declare struct of tree
struct tree
{	
	int count;

	node_t *root;

  // Function-pointers
	element_copy_fun elem_copy;
	key_free_fun key_free;
	element_free_fun elem_free;
	element_comp_fun elem_cmp;
};

struct node
{
	tree_key_t key;

	elem_t elem;

	node_t *nodeR;
	node_t *nodeL;
	node_t *parentNode;
};

//Return a new, empty tree
tree_t *tree_new(element_copy_fun element_copy,
	key_free_fun key_free,
	element_free_fun elem_free,
	element_comp_fun compare)
{
	// None of the functions are allowed to be NULL
	if (!element_copy) { return NULL; }
	if (!key_free) { return NULL; }
	if (!elem_free) { return NULL; }
	if (!compare) { return NULL; }

    // Dedicate memory on heap
	tree_t *new_tree = (tree_t *) calloc(1, sizeof(tree_t));

    // Set standard values
	new_tree->count = 0;
	new_tree->root = NULL;

    // Set functions
	new_tree->elem_copy = element_copy;
	new_tree->key_free = key_free;
	new_tree->elem_free = elem_free;
	new_tree->elem_cmp = compare;

	return new_tree;
}

node_t *node_new() 
{
	return (node_t *) calloc(1, sizeof(node_t));
}

//Frees and tares down the structure of the tree (not the keys, elements or functions)
void deleteTree(tree_t *tree)
{	
	if (!tree) { return; }

	node_t *nextNode = tree->root;

	if (!nextNode)
	{
		free(tree);

		return;
	}

	nextNode = nextNode->nodeL ? nextNode->nodeL : nextNode->nodeR;
	
	//While next node isn't the first node
	while (nextNode != tree->root)
	{
		node_t *tmp = NULL;

		while (nextNode)
		{
			//If left node isn't NULL
			if (nextNode->nodeL)
			{
				nextNode = nextNode->nodeL;
			}
			//If right node isn't NULL
			else if (nextNode->nodeR)
			{
				nextNode = nextNode->nodeR;
			}
			else 
			{
				break;
			}
		}

		tmp = nextNode;

		nextNode = nextNode->parentNode;

		node_t **tmpParentPosition = nextNode->nodeL == tmp ?
		&nextNode->nodeL : &nextNode->nodeR;

		*tmpParentPosition = NULL;

		free(tmp);
	}

	free(tree);
}


void free_func_on_nodes(node_t *node, element_free_fun free_func, bool freeKey)
{
	if (!node) { return; }
	if (!free_func) { return; }

	free_func_on_nodes(node->nodeL, free_func, freeKey);
	free_func_on_nodes(node->nodeR, free_func, freeKey);

	if (freeKey)
	{
		free_func(node->key);
	}
	else
	{
		free_func(node->elem);
	}
}

void free_func_on_tree(tree_t *tree, element_free_fun free_func, bool freeKey)
{
	if (!tree) { return; }

	free_func_on_nodes(tree->root, free_func, freeKey);
}

void free_all_keys(tree_t *tree)
{
	if (!tree) { return; }

	free_func_on_tree(tree, tree->elem_free, true);
}

void free_all_elems(tree_t *tree)
{
	if (!tree) { return; }

	free_func_on_tree(tree, tree->elem_free, false);
}

void tree_delete(tree_t *tree, bool delete_keys, bool delete_elements)
{
	if (!tree) { return; }

	if (delete_elements)
	{
		free_all_elems(tree);
	}

	if (delete_keys)
	{
		free_all_keys(tree);
	}

	deleteTree(tree);
}

//Returns the number of nodes (items) in tree
int tree_size(tree_t *tree)
{
	if (!tree) { return 0; }

	return tree->count;
}

//Returns the depth of the tree (H(tree)-1)
int tree_depth(tree_t *tree)
{
	if (!tree) { return 0; }

	// Optimal is = ln(n)/ln(2)+1
	return node_depth(tree->root);
}

//If a new value has replaced the old root
bool change_tree_root(tree_t *tree)
{
	if (!tree) { return false; }
	if (!tree->root) { return false; }

	if (tree->root->parentNode)
	{
		tree->root = tree->root->parentNode;

		return true;
	}

	return false;
}

//Insert element into tree. Return false if key is N/A
bool tree_insert(tree_t *tree, tree_key_t key, elem_t elem)
{
	if (!tree) { return false; }

	if (tree_has_key(tree, key))
	{
		return false;
	}

	node_t *new_node = node_new();
	new_node->key = key;
	new_node->elem = elem;

	if (!tree->root)
	{
		tree->root = new_node;
		tree->count = 1;

		return true;
	}

	node_t *nextNode = tree->root;


	//While nextNode != NULL
	while (nextNode)
	{
		//If key is smaller
		if (tree->elem_cmp(nextNode->key, key) > 0)
		{
			if (!nextNode->nodeL)
			{
				nextNode->nodeL = new_node;
				new_node->parentNode = nextNode;
				break;
			}
			else 
			{
				nextNode = nextNode->nodeL;
			}
		}
		//If key is larger
		else if (tree->elem_cmp(nextNode->key, key) < 0)
		{
			if (!nextNode->nodeR)
			{
				nextNode->nodeR = new_node;
				new_node->parentNode = nextNode;
				break;
			}
			else
			{
				nextNode = nextNode->nodeR;
			}
		}
	}

	balance_tree(tree);

	change_tree_root(tree);

	++tree->count;
	return true;
}

//Returns if the key is already in tree
bool tree_has_key(tree_t *tree, tree_key_t key)
{
	if (!tree) { return false; }

	node_t *tmpNode = tree_traverse(tree, key);

	return tmpNode ? true : false;
}

//Returns element of matching key
bool tree_get(tree_t *tree, elem_t key, elem_t *result)
{
	if (!tree) { return false; }

	node_t *node = tree_traverse(tree, key);
	*result = node->elem;

	return node ? true : false;
}

void replace_node(node_t *toRemove, node_t *replacement)
{
	if (!toRemove) { return; }
	if (!replacement) { return; }
	if (!replacement->parentNode)
	{
		puts("replace_node():: !replacement->parentNode");
		return;
	}
	if (replacement->nodeL)
	{
		if (!toRemove->nodeL && toRemove->parentNode)
		{
			node_t **toRemovesParentPosition = toRemove->parentNode->nodeR == toRemove ?
			&toRemove->parentNode->nodeR : &toRemove->parentNode->nodeL;

			*toRemovesParentPosition = replacement;

			return;
		}		

		assert(false);
		return;
	}

	node_t **replacementsParentPosition = replacement->parentNode->nodeL == replacement ?
	&replacement->parentNode->nodeL : &replacement->parentNode->nodeR;

	*replacementsParentPosition = replacement->nodeR;

	replacement->parentNode = toRemove->parentNode;
	replacement->nodeL = toRemove->nodeL;
	replacement->nodeR = toRemove->nodeR;

	if (toRemove->parentNode)
	{
		node_t **toRemovesParentPosition = toRemove->parentNode->nodeR == toRemove ?
		&toRemove->parentNode->nodeR : &toRemove->parentNode->nodeL;

		*toRemovesParentPosition = replacement;
	}

	if (toRemove->nodeL)
	{
		toRemove->nodeL->parentNode = replacement;
	}

	if (toRemove->nodeR)
	{
		toRemove->nodeR->parentNode = replacement;
	}
}

node_t *replace_with_smallest_bigger(node_t *toRemove)
{
	if (!toRemove) { return NULL; }

	// toRemove should have 2 children
	if (!toRemove->nodeL && !toRemove->nodeR) { return NULL; }

	node_t *rightChild = toRemove->nodeR;

	// If toRemove->nodeR doesn't have any children
	if (!rightChild->nodeL && !rightChild->nodeR)
	{
		replace_node(toRemove, rightChild);

		return NULL;
	}

	// If rightChild only have a right child
	if (!rightChild->nodeL && rightChild->nodeR)
	{
		replace_node(toRemove, rightChild);

		return NULL;
	}

	// This should have been sorted with the previous "if"s
	if (!rightChild->nodeL)
	{
		assert(false);
		return NULL;
	}

	node_t *nextNode = rightChild->nodeL;

	// Navigate to the left-most point
	while (nextNode->nodeL)
	{
		nextNode = nextNode->nodeL;
	}

	if (nextNode->nodeR)
	{
		if (nextNode->nodeR->nodeL)
		{
			replace_with_smallest_bigger(nextNode);
		}
		else
		{
			replace_node(nextNode, nextNode->nodeR);
		}
	}

	replace_node(toRemove, nextNode);

	return nextNode;
}

//Returns the removed element
bool tree_remove(tree_t *tree, tree_key_t key, elem_t *result)
{
	if (!tree) { return false; }

	node_t *tmpNode = tree_traverse(tree, key);

	bool returnBool = tmpNode ? true : false;

	if (!returnBool) { return returnBool; }

	*result = tmpNode->elem;

	bool treeRoot = tmpNode->parentNode == NULL;
	node_t *replacement_node = NULL;

    // If 2 children
	if (tmpNode->nodeL && tmpNode->nodeR) 
	{
		replacement_node = replace_with_smallest_bigger(tmpNode);

	} // If 1 child
	else if (tmpNode->nodeL || tmpNode->nodeR)
	{
		replacement_node = tmpNode->nodeL ? tmpNode->nodeL : tmpNode->nodeR;

		replace_node(tmpNode, replacement_node);

	}
	else
	{
		if (!treeRoot) 
		{
			node_t **removesParentPosition = tmpNode->parentNode->nodeL == tmpNode ?
			&tmpNode->parentNode->nodeL : &tmpNode->parentNode->nodeR;

			*removesParentPosition = NULL;
		}
	}

	if (treeRoot)
	{
		tree->root = replacement_node;
	}

	free(tmpNode);

	--tree->count;

	balance_tree(tree);

	return returnBool;
}

//Traverses the tree and returns the subtree with the same key as k Key.
node_t *tree_traverse(tree_t *tree, tree_key_t key)
{
	if (!tree) { return NULL; }
	if (!tree->root) { return NULL; }

	node_t *nextNode = tree->root;

	
          //Traverse the tree and set nextNode to nodeL/tree until we find the key or hit NULL.
	do 
	{
		if (tree->elem_cmp(nextNode->key, key) == 0) // Same
		{
			break;
		}
		else if (tree->elem_cmp(nextNode->key, key) > 0) //nextNode->key > key
		{
			nextNode = nextNode->nodeL;
		}
		else if (tree->elem_cmp(nextNode->key, key) < 0) //nextNode->key < key
		{
			nextNode = nextNode->nodeR;
		}

		if (!nextNode) { return NULL; }

	} while (nextNode->nodeL || nextNode->nodeR);

	//Return the right thing.
	return (tree->elem_cmp(nextNode->key, key) == 0) ? nextNode : NULL;
}



// Aux-function for tree_keys
void tree_keys_aux(node_t *node, tree_key_t **keyArrayAdress)
{
	if (!node) { return; }
	if (!*keyArrayAdress) { return; }

	//Go down the left side
	tree_keys_aux(node->nodeL, keyArrayAdress);

    //Add to Array
	**keyArrayAdress = node->key;
	++*keyArrayAdress;

	// Go down right side
	tree_keys_aux(node->nodeR, keyArrayAdress);
}

/// Returns an array holding all the keys in the tree
/// in ascending order.
tree_key_t *tree_keys(tree_t *tree)
{
	if (!tree) { return NULL; }

	int treeSize = tree_size(tree);

	if (treeSize == 0) { return NULL; }

	//Allocates memory on heap for the keys
	tree_key_t *keyArray = (tree_key_t *) calloc(treeSize, sizeof(tree_key_t));
	tree_key_t **keyArrayAdress = (tree_key_t **) calloc(1, sizeof(void *));
	*keyArrayAdress = keyArray;

	tree_keys_aux(tree->root, keyArrayAdress);

	return keyArray;
}

void tree_elements_aux(node_t *node, elem_t **elementsArrayAdress)
{
	if (!node) { return; }
	if (!*elementsArrayAdress) { return; }

  	//Go down the left side
	tree_elements_aux(node->nodeL, elementsArrayAdress);

	//Add key to elementArray
	**elementsArrayAdress = node->elem;
	++*elementsArrayAdress;

	// Go down right side
	tree_elements_aux(node->nodeR, elementsArrayAdress);
}

elem_t *tree_elements(tree_t *tree)
{
	if (!tree) { return NULL; }

	int treeSize = tree_size(tree);

	if (treeSize == 0) { return NULL; }

	elem_t *elementArray = (elem_t *) calloc(treeSize, sizeof(elem_t));
	elem_t **elementArrayAdress = (elem_t **) calloc(1, sizeof(void *));
	*elementArrayAdress = elementArray;

	tree_elements_aux(tree->root, elementArrayAdress);

	return elementArray;
}

bool tree_apply(tree_t *tree, enum tree_order order, key_elem_apply_fun fun, void *data)
{
	//TODO: This
	return false;
}

////////////////////////
// AVL-Tree functions //
////////////////////////

void balance_tree(tree_t *tree)
{
	if (!tree) { return; }

	balance_sub_nodes(tree->elem_cmp, tree->root);
}

void balance_sub_nodes(element_comp_fun elem_cmp, node_t *node)
{
	if (!node) { return; }
	if (!elem_cmp) { return; }

	balance_node(elem_cmp, node);

	if (node->nodeL)
	{
		balance_sub_nodes(elem_cmp, node->nodeL);
	}

	if (node->nodeR)
	{
		balance_sub_nodes(elem_cmp, node->nodeR);
	}
}

void balance_node(element_comp_fun elem_cmp, node_t *node)
{
	//Node should not be NULL
	if (!node) { return; }
	if (!elem_cmp) { return; }

	int nodeBalance = node_balance(node);

	if (nodeBalance > 1)
	{
		int leftChildBalance = node_balance(node->nodeL);

		if (leftChildBalance > 0)
		{
			right_rotate(elem_cmp , node);
		}
		else if (leftChildBalance < 0)
		{
			left_right_rotate(elem_cmp, node);
		}
	}
	else if (nodeBalance < -1)
	{
		int rightChildBalance = node_balance(node->nodeR);

		if (rightChildBalance < 0)
		{
			left_rotate(elem_cmp, node);
		}
		else if (rightChildBalance > 0)
		{
			right_left_rotate(elem_cmp, node);
		}
	}
}

int max_of_int(int one, int two)
{
	return (one > two) ? one : two;
}

int node_depth(node_t *node)
{
	if (!node) { return 0; }

	return max_of_int(node_depth(node->nodeL) + 1, node_depth(node->nodeR) + 1);
}

int node_balance(node_t *node)
{
	if (!node) { return 0; }

	int leftDepth = 0;
	int rightDepth = 0;

	if (node->nodeL)
	{
		leftDepth = 1 + node_depth(node->nodeL);
	}

	if (node->nodeR)
	{
		rightDepth = 1 + node_depth(node->nodeR);
	}

	return leftDepth - rightDepth;
}



// Self balancing (AVL)
void right_rotate(element_comp_fun elem_cmp, node_t *node)
{
	if (!node) { return; }
	if (!node->nodeL) { return; }
	if (!node->nodeL->nodeL) { return; }
	if (!elem_cmp) { return; }

	/// Letter references
	///   a  (parentNode)
	///   Z  (node)
	///  Y   (node->nodeL)
	/// X    (node->nodeL->nodeL)

	// Copies adress of Y-right
	node_t *nodeLeftRight = node->nodeL->nodeR;

	// Y-right = node
	node->nodeL->nodeR = node;

	node->nodeL->parentNode = node->parentNode;

	// If parentNode isn't NULL
	if (node->parentNode)
	{
		// If node is the left node of it's parent
    		// Make Y new left node of parent
    	// Else make Y new right node of parent
		node_t **parentsOtherNode = node->parentNode->nodeL == node ?
		&node->parentNode->nodeL : &node->parentNode->nodeR;

		*parentsOtherNode = node->nodeL;
	}

	node->parentNode = node->nodeL;
	node->nodeL = nodeLeftRight;
}

// Self balancing (AVL)
void left_rotate(element_comp_fun elem_cmp, node_t *node)
{
	if (!node) { return; }
	if (!node->nodeR) { return; }
	//if (!node->nodeR->nodeR) { return; }
	if (!elem_cmp) { return; }

	/// Letter references
	///  a    (parentNode)
	///  X    (node)
	///   Y   (node->nodeR)
	///    Z  (node->nodeR->nodeR)

	// Copies adress of Y-left
	node_t *nodeRightLeft = node->nodeR->nodeL;

	// Y-left = node
	node->nodeR->nodeL = node;


    // If parentNode isn't NULL
	if (node->parentNode)
	{
    	// If node is the left node of it's parent
	    	// Make Y new left node of parent
    	// Else make Y new right node of parent
		node_t **nodesParentPosition = node->parentNode->nodeL == node ?
		&node->parentNode->nodeL : &node->parentNode->nodeR;

		*nodesParentPosition = node->nodeR;
	}

	node->nodeR->parentNode = node->parentNode;

	node->parentNode = node->nodeR;
	node->nodeR = nodeRightLeft;

}

void left_right_rotate(element_comp_fun elem_cmp, node_t *node)
{
	/// Letter references
	///   a  (parentNode)
	///   Z  (node)
	///  Y   (node->nodeL)
	///   X  (node->nodeL->nodeL)

	left_rotate(elem_cmp, node->nodeL);
	right_rotate(elem_cmp, node);
}

void right_left_rotate(element_comp_fun elem_cmp, node_t *node)
{
	/// Letter references
	///  a    (parentNode)
	///  X    (node)
	///   Y   (node->nodeR)
	///  Z    (node->nodeR->nodeL)

	right_rotate(elem_cmp, node->nodeR);
	left_rotate(elem_cmp, node);

}

// FROM ELISABETH'S INLUPP1
/*
bool tree_for_each_aux(node_t *node, eachfunction forEachFunc, void *data)
{
	if (node == NULL)
	{
		return false;
	}
	else
	{
		tree_for_each_aux(node->nodeL, forEachFunc, data);
		forEachFunc(node->key, data);
		tree_for_each_aux(node->nodeR, forEachFunc, data);
	}
}

bool tree_for_each(tree_t *tree, eachfunction forEachFunc, void *data)
{
	return tree_for_each_aux(tree->root, forEachFunc);
}
*/
