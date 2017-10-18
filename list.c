#include "list.h"
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef struct link link_t;

struct list{
  link_t *first;
  link_t *last;
  element_comp_fun compare;
  element_free_fun free;
  element_copy_fun copy;
};

struct link{
  elem_t elem;
  link_t *next;
};


//Creates a new empty list and returns it
list_t *list_new(element_copy_fun copy, element_free_fun free, element_comp_fun compare){
  list_t *list = calloc(1, sizeof(list_t));
  list->compare=compare;
  list->free=free;
  list->copy=copy;
  return list;
}


//creates a new link with the elem input
link_t *link_new(elem_t elem, link_t *next){
  link_t *new_link = calloc(1, sizeof(link_t));
  new_link->elem = elem;
  new_link->next = next;
  return new_link;
}


void list_append (list_t *list, elem_t elem){
  link_t *link = link_new(elem, NULL);
  if(list->last != NULL){
    list->last->next = link;
    list->last = link;
  }else{ //makes the appended item the first and the last adress in list if it's the only one
    list->first = link;
    list->last = link;
  }
}

//prepends elem to the list, which creates a new list entry at the end of that list that points to itself.
void list_prepend (list_t *list, elem_t elem){
  link_t *link = link_new(elem, NULL);
  link_t *tmp = list->first;
  list->first = link;
  link->next = tmp;
  if(list->last == NULL){ //makes the prepended item the first and the last adress in list if it's the only one
    list->last = link;
  }
}


//returns the length of the list
int list_length(list_t *list){
  link_t *cursor = list->first;
  int i = 0;
  while(cursor != NULL){
    cursor = cursor->next;
    ++i;
  }
  return i;
}


void list_insert_aux(list_t *list, int index, elem_t elem){
  link_t *cursor = list->first;
  
  int i = 0;
  while (i < (index - 1)){ //manouvers to the link which the new link shall be appended to
    cursor = cursor->next;
    ++i;
    printf("I am looping, value of i = %d\n", i);
    }
  link_t *new_next = cursor->next;
  link_t *new_link = link_new(elem, new_next);
  cursor->next = new_link;
}


//returns true if the new element was added, false if it wasn't
void list_insert(list_t *list, int index, elem_t elem){
  int check = abs(index);
  if (check < list_length(list))
    {  
      if(index == 0 || (list_length(list) + index == 0)){
        list_prepend(list, elem);
      }
      else if(list_length(list) == index || index == -1){
        list_append(list, elem);
      }
      else if (index < 0){
        index = list_length(list) + index;
        list_insert_aux(list, index, elem);
      }
      else{
        list_insert_aux(list, index, elem);
      }
    }
}



void list_remove_aux(list_t *list, int index){
  if(list->first == list->last){
    free(list->first);
    list->first = NULL;
    list->last = NULL;
  }
  else{
    link_t *cursor = list->first;
    if(index == 0){
      link_t *temp = cursor->next;
      free(cursor);
      list->first = temp;
    }
    
    int i = 0;
    while (i < (index - 1)){// sets cursor to the node before the selected node 
      cursor = cursor->next;
      ++i;
    }
    link_t *tmp = cursor->next;
    if(tmp->next == NULL){// if the removed node is the last
      list->last = cursor;
    }
    cursor->next = tmp->next;
  }
}

void list_remove(list_t *list, int index, bool delete){
  int check = abs(index);
  delete = false;
  if(list_length(list) > check)
    {
      delete = true;
      if (index < 0){ //fixes negative index
        index = list_length(list) + index;
        list_remove_aux(list, index);

      }
      else
        {
          list_remove_aux(list, index);
        }
    }
}

elem_t list_get_aux(list_t *list, int index){
  if (index == 0){
    return list->first->elem;
  }
  link_t *cursor = list->first;
  int i = 0;
  while (i <= (index - 1)){
    cursor = cursor->next;
    ++i;
  }
  return cursor->elem;
}

bool list_get(list_t *list, int index, elem_t *result){
  int check = abs(index);
  if(list_length(list) <= index || list_length(list) < check){
    printf("The given index doesn't exist\n");
    return false;
  }else if (index < 0){
    index = list_length(list) + index;
    *result = list_get_aux(list, index);
    return true;
  }else{
    *result = list_get_aux(list, index);
    return true;
  }
}

void list_delete(list_t *list, bool delete){
  if(delete){
    list_iterate(list, list->free);
  }
  free(list);
}

bool list_apply(list_t *list, elem_apply_fun fun, void *data){
  if(list_length(list) > 0){
    link_t *cursor = list->first;
    while(cursor->next != NULL){//doesn't catch last link
      fun(cursor->elem, data);
      cursor = cursor->next;
    }
    fun(cursor->elem, data); // catches last link
    return true;
  }
  return false;
}



void list_iterate(list_t *l, iter_fun_t *f)
{
  for (link_t *c = l->first; c; c = c->next)
    {
        f(c->elem);
    }
}


int main(){
  list_t *list=list_new(NULL, NULL ,NULL);
  elem_t elem = {.p = NULL};
  list_append(list, elem);
  list_length(list);
  elem_t result;
  bool x=(list_get(list, 0, &result));
  printf("%d",x);
  return 0;
}
