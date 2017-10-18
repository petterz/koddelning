#include "common.h"
#include "tree.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct node node_t;

struct node {
  tree_key_t key;
  elem_t elem;
  node_t *left;
  node_t *right;
};


struct tree {
  node_t *node;
  element_copy_fun element_copy;
  key_free_fun key_free;
  element_free_fun element_free;
  element_comp_fun element_comp;
};

int tree_depth_aux(node_t *node){
  if(node->left == NULL && node->right == NULL){
    return 1;
  }else if(node->right == NULL){
    return 1 + tree_depth_aux(node->left);
  }else if(node->left == NULL){
    return 1 +tree_depth_aux(node->right);
  }else{
    if(tree_depth_aux(node->left) >= tree_depth_aux(node->right)){
      return 1 + tree_depth_aux(node->left);
    }else{
      return 1 + tree_depth_aux(node->right);
    }
  }
}

// A utility function to get maximum of two integers
int max(int a, int b)
{
    return (a > b)? a : b;
}
 
// A utility function to right rotate subtree rooted with y
// See the diagram given above.
struct node *rightRotate(struct node *y)
{
  struct node *x = y->left;
    struct node *T2 = x->right;
 
    // Perform rotation
    x->right = y;
    y->left = T2;
 
    // Return new root
    return x;
}
 
// A utility function to left rotate subtree rooted with x
// See the diagram given above.
struct node *leftRotate(struct node *x)
{
    struct node *y = x->right;
    struct node *T2 = y->left;
 
    // Perform rotation
    y->left = x;
    x->right = T2;
 
    // Return new root
    return y;
}

int get_balance(node_t *node){
  if(node==NULL){
    return 0;
  }
  return tree_depth_aux(node->left)-tree_depth_aux(node->right);
}

//Skapar ett nytt, tomt träd. Allokelar minne för storleken

tree_t *tree_new(element_copy_fun element_copy, key_free_fun key_free, element_free_fun elem_free, element_comp_fun compare){
  tree_t *tree = calloc(1, sizeof(tree_t));
  tree->element_copy=element_copy;
  tree->key_free=key_free;
  tree->element_free=elem_free;
  tree->element_comp=compare;
  return tree;
}

node_t *node_new(tree_key_t key, elem_t elem){
  node_t *node = calloc(1, sizeof(node_t));
  node->key = key;
  node->elem = elem;
  node->left = NULL;
  node->right = NULL;
  return node;
}


void tree_delete_aux(node_t *node, element_free_fun elem, element_free_fun key){
  if(node->left == NULL && node->right == NULL){
    if(key){
      key(node->key);
    }
    if(elem){
      elem(node->elem);
    }
    free(node); 
  }
  if(node->left != NULL){
    tree_delete_aux(node->left, elem, key);
  }
  else if(node->right != NULL){
    tree_delete_aux(node->right, elem,key);
  }
 
}


void tree_delete(tree_t *tree, bool delete_keys, bool delete_elements){
  element_free_fun elem=NULL;
  element_free_fun key=NULL;
  if (delete_elements==true){
   elem=tree->element_free;
  }
 if (delete_keys==true){
   key=tree->element_free;
  }
  if(tree_size(tree) > 0){
      tree_delete_aux(tree->node, elem, key);
  }
  if (tree_size(tree) == 0){
    free(tree);
  }
   
}

bool tree_insert_aux(node_t *node, tree_key_t key, elem_t elem, element_comp_fun compare){

  if (node == NULL){
    node=node_new(key, elem);
    return true;
  }
  
  if (0>compare(key,node->key)){
    tree_insert_aux(node->left, key, elem, compare);
  }
  
  else if (0<compare(key,node->key)){
    tree_insert_aux(node->right, key, elem, compare);
  }
  else {
    return false;
  }
  
  int balance = get_balance(node);
 
  // If this node becomes unbalanced, then
  // there are 4 cases
 
  // Left Left Case
  if (balance > 1 && (0>compare(key,node->left->key)))
    rightRotate(node);
 
  // Right Right Case
  if (balance < -1 && (0<compare(key,node->right->key)))
    leftRotate(node);
 
  // Left Right Case
  if (balance > 1 &&  (0<compare(key,node->left->key)))
    {
      node->left =  leftRotate(node->left);
      rightRotate(node);
    }
 
  // Right Left Case
  if (balance < -1 &&  (0>compare(key,node->right->key)))
    {
      node->right = rightRotate(node->right);
      leftRotate(node);
    }
  return true;
}

bool tree_insert(tree_t *tree, tree_key_t key, elem_t elem){
if(tree_has_key(tree, key)){ //retunerar false om nyckeln redan finns
      return false;
  }else{
  return tree_insert_aux(tree->node, key, elem, tree->element_comp);
  }
}



//Använder sig av rekursion for att gå igenom trädet och retunera det totala antalet noder.



int tree_size_aux(node_t *node){
  if (node->left == NULL && node->right == NULL){
    return 1;
  }else if(node->left == NULL){
    return 1 + tree_size_aux(node->right);
  }else if(node->right == NULL){
    return 1 + tree_size_aux(node->left);
  }else{
    return 1 + tree_size_aux(node->right) + tree_size_aux(node->left);
  }
}

int tree_size(tree_t *tree){
  if(tree->node == NULL){
    return 0;
  }else{
  return tree_size_aux(tree->node);
  }
}


// Använder sig av rekursion för att retunera det största djupet på trädet.

int tree_depth(tree_t *tree){
  if(tree->node == NULL){
    return 0;
  }else{
    return tree_depth_aux(tree->node);
  } 
}


//Använder rekursion för att traversera trädet och retunera jämförelsen av instoppade nyckeln och alla nycklar i trädet, för att se om nyckeln redan finns i trädet.

bool tree_has_key_aux(node_t *node, tree_key_t key, element_comp_fun compare){
  if(0==compare(node->key, key)){
    return true;
  }else if(node->left == NULL && node->right == NULL){
    return false;
  }else{
    return(tree_has_key_aux(node->left, key, compare) || tree_has_key_aux(node->right, key, compare));
  }
}

bool tree_has_key(tree_t *tree, tree_key_t key){
  if(tree->node == NULL){
    return false;
  }else{
    return tree_has_key_aux(tree->node, key, tree->element_comp);           
  }
}


//insertar en key och ett element in i ett träd. Om key redan finns i trädet retuneras false

//Kolla med GDB så att trädet verkligen blir balanserat!!!!!!!


//Utifrån antagandet att den inmatade nyckeln existerar i det givna trädet så kommer funktionen att retunera elementet kopplat till nyckeln


bool tree_get_aux(node_t *node, tree_key_t key, elem_t *result, element_comp_fun compare){
  if(0==compare(node->key, key)){
    return true;
  }
   else if(node->left != NULL){
     return tree_get_aux(node->left, key, result, compare);
  }
  else if(node->right != NULL){
    return tree_get_aux(node->right, key, result, compare);
  }
else
    return false; 
}

bool tree_get(tree_t *tree, tree_key_t key, elem_t *result){
  if(tree->node != NULL){
    return tree_get_aux(tree->node, key, result, tree->element_comp);
  }
  else{
    return NULL;
  }
}

node_t *min_value_node(node_t *node)
{
    node_t *current = node;
 
    /* loop down to find the leftmost leaf */
    while (current->left != NULL)
        current = current->left;
 
    return current;
}

bool node_remove(node_t *node, tree_key_t key, elem_t *result, element_comp_fun compare){
   
    // STEP 1: PERFORM STANDARD BST DELETE
 
    if (node == NULL)
        return false;
 
    // If the key to be deleted is smaller than the
    // root's key, then it lies in left subtree
    if (0>compare(key,node->key))
      node_remove(node->left, key, result, compare);
 
    // If the key to be deleted is greater than the
    // root's key, then it lies in right subtree
    else if(0<compare(key,node->key))
      node_remove(node->right, key, result, compare);
    // if key is same as root's key, then This is
    // the node to be deleted
    else
    {
        // node with only one child or no child
        if( (node->left == NULL) || (node->right == NULL) )
        {
            node_t *temp = node->left ? node->left :
                                             node->right;
            // No child case
            if (temp == NULL)
            {
                temp = node;
                node = NULL;
            }
            else // One child case
             *node = *temp; // Copy the contents of
                            // the non-empty child
            free(temp);
        }
        else
        {
            // node with two children: Get the inorder
            // successor (smallest in the right subtree)
            node_t *temp = min_value_node(node->right);
 
            // Copy the inorder successor's data to this node
            node->key = temp->key;
 
            // Delete the inorder successor
            node_remove(node->right, temp->key, result, compare);
        }
    }
 
    // If the tree had only one node then return
    if (node == NULL)
      return false;
    
    // STEP 3: GET THE BALANCE FACTOR OF THIS NODE (to
    // check whether this node became unbalanced)
    int balance = get_balance(node);
 
    // If this node becomes unbalanced, then there are 4 cases
 
    // Left Left Case
    if (balance > 1 && get_balance(node->left) >= 0)
        rightRotate(node);
 
    // Left Right Case
    if (balance > 1 && get_balance(node->left) < 0)
    {
        node->left =  leftRotate(node->left);
        rightRotate(node);
    }
 
    // Right Right Case
    if (balance < -1 && get_balance(node->right) <= 0)
        leftRotate(node);
 
    // Right Left Case
    if (balance < -1 && get_balance(node->right) > 0)
    {
        node->right = rightRotate(node->right);
       leftRotate(node);
    }
 
    return true;
}

bool tree_remove(tree_t *tree, tree_key_t key, elem_t *result){
  if (tree_has_key(tree, key)){
    return node_remove(tree->node, key, result, tree->element_comp);
  }
  else
    return false;
}

 void tree_keys_aux(node_t *node, tree_key_t *key_array, int *index){
  if(node->left == NULL){//Found the leftmost key of a given sub-tree
   key_array[*index] = node->key;
    *index = *index + 1;
  }else{ // Parent node, recurses into left node, then adds self
    tree_keys_aux(node->left, key_array, index);
    key_array[*index] = node->key;
    *index = *index + 1;
  }
  
  if(node->right != NULL){
    tree_keys_aux(node->right, key_array, index);
  }
}

tree_key_t *tree_keys(tree_t *tree){
  if(tree->node == NULL){
    return NULL;
  }else{
    int node_amount = tree_size(tree);
    tree_key_t *key_array = calloc(node_amount, sizeof(tree_key_t));
    int index = 0;
    tree_keys_aux(tree->node, key_array, &index);
    return key_array;
  }
}

void tree_elements_aux(node_t *node, elem_t *elem_array, int *index){
    if(node->left == NULL){//Found the leftmost key of a given sub-tree
    elem_array[*index] = node->elem;
    *index = *index + 1;
  }else{ // Parent node, recurses into left node, then adds self
    tree_elements_aux(node->left, elem_array, index);
    elem_array[*index] = node->elem;
    *index = *index + 1;
  }
  
  if(node->right != NULL){
    tree_elements_aux(node->right, elem_array, index);
  }
}

elem_t *tree_elements(tree_t *tree){
  if(tree->node == NULL){
    return NULL;
  }else{
    int node_amount = tree_size(tree);
    elem_t *elem_array = calloc(node_amount, sizeof(elem_t));
    int index = 0;
    tree_elements_aux(tree->node, elem_array, &index);
    return elem_array;
  }
}

 
bool tree_apply_aux(node_t *node, enum tree_order order, key_elem_apply_fun function, void *data){
  if(order){ // postorder
    if(node->left == NULL && node->right == NULL){
      function(node->key, node->elem, data);
    }else{
      if(node->right == NULL){
        tree_apply_aux(node->left, order, function, data);
        function(node->key, node->elem, data);
      }else if(node->left == NULL){
        tree_apply_aux(node->right, order, function, data);
        function(node->key, node->elem, data);
      }else{
        tree_apply_aux(node->left, order, function, data);
        tree_apply_aux(node->right, order, function, data);
        function(node->key, node->elem, data);
      }
    }
  }
  else if(order == 0){ // inorder
    if(node->left == NULL && node->right == NULL){
      function(node->key, node->elem, data);
    }else {
      if(node->left == NULL){
	function(node->key, node->elem, data);
        tree_apply_aux(node->right, order, function, data);
      }else if(node->right == NULL){
	tree_apply_aux(node->left, order, function, data);
	function(node->key, node->elem, data);
      }else{
        tree_apply_aux(node->left, order, function, data);
        function(node->key, node->elem, data);
        tree_apply_aux(node->right, order, function, data);
      }
    }
  }
  else{ // preorder
    if(node->left == NULL && node->right == NULL){
      function(node->key, node->elem, data);
    }else {
      if(node->right == NULL){
        function(node->key, node->elem, data);
        tree_apply_aux(node->left, order, function, data);
      }else if(node->left == NULL){
        function(node->key, node->elem, data);
        tree_apply_aux(node->right, order, function, data);
      }else{
        function(node->key, node->elem, data);
        tree_apply_aux(node->left, order, function, data);
        tree_apply_aux(node->right, order, function, data);
      }
    }
  }
      return true;
}


bool tree_apply(tree_t *tree, enum tree_order order, key_elem_apply_fun fun, void *data){
  if(tree->node == NULL){
    puts("Please input a non-empty tree");
    return false;
  }
  else{
    return tree_apply_aux(tree->node, order, fun, data);
  }
}







/*
int main(){
  tree_t *tree = tree_new();
  int x = 10;
  int y = 20;
  int z = 30;
  tree_insert(tree, "Aang", &x);
  tree_insert(tree, "Anka", &y);
  tree_insert(tree, "aaaa", &z);
  if(tree_has_key(tree, "aa")){
    puts("Found your key!");
  }
  int depth = tree_depth(tree);
  int size = tree_size(tree);
  printf("Depth of tree: %d\nSize of tree: %d\n", depth, size);
  T *elem_array = tree_elements(tree);
  for (int i = 0; i < tree_size(tree); ++i){
    printf("%d. %d\n", i, *((int *)elem_array[i]));
  }
}
*/  
