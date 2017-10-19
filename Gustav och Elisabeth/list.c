/*
  skriv shelf funktionerna
  stöd för sparande till fil
  andra grejer
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "list.h"
#include "common.h"

typedef struct link link_t;
typedef struct iterator iterator_t;

struct iterator
{
  link_t *link_iter;
};

struct link
{
  link_t *previous;
  link_t *next;
  elem_t elem;
};

//Declare struct of list
struct list
{
  link_t *first;
  link_t *last;
  int length;	
  element_copy_fun copy;
  element_comp_fun comp;
  element_free_fun free;
};

link_t *list_index_adress(list_t *list, int index);

list_t *list_new(element_copy_fun copy, element_free_fun free, element_comp_fun comp)
{
  list_t *newlist = calloc(1, sizeof(elem_t));
  newlist->length = 0;
  newlist->copy = copy;
  newlist->free = free;
  newlist->comp = comp;
  return newlist;
}

void list_append(list_t *list, elem_t elem)
{
  link_t *last_link = list->last;

  link_t *new_link = calloc(1, sizeof(link_t));

  new_link->previous = last_link;
  new_link->next = NULL;
  new_link->elem = elem;

  list->last = new_link;

  ++list->length;

  if (!(list->first))
    {
      list->first = new_link;
    }
}

//Inserts a new element at the beginning of the list
void list_prepend(list_t *list, elem_t elem)
{
  link_t *first_link = list->first;

  link_t *new_link = calloc(1, sizeof(link_t));

  new_link->previous = NULL;
  new_link->next = first_link;
  new_link->elem = elem;
  list->first = new_link;
  ++list->length;
}

void list_insert(list_t *list, int index, elem_t elem)
{ //vad ligger i elem som vi skickar in??
  link_t *tmp = list_index_adress(list, index);

  link_t *new_link = calloc(1, sizeof(link_t));
  new_link->elem = elem;

  new_link->next = tmp->next;
  tmp->next = new_link;
  new_link->previous = tmp;

  ++list->length;
}


link_t *list_index_adress(list_t *list, int index)
{
  int listLength = list_length(list);

  if ((listLength - 1) < abs(index))
    {
      return NULL;
    }

  link_t *tmp;

  //Less the half the length and positiv
  if (index > -1 && index < (listLength - 1) / 2)
    {
      tmp = list->first;

      for (int i = 0; i < index; ++i)
        {
          tmp = tmp->next;
        }
    }
  else 
    {
      tmp = list->last;

      for (int i = listLength - 1; i > index; --i)
        {
          tmp = tmp->previous;
        }
    }

  return tmp;
}


//Removes an element from a list
//Returns true if success, else false
void list_remove(list_t *list, int index, bool delete)
{
  link_t *link_index = list_index_adress(list, index);

  if (!link_index)
    {
      ;
    }

  link_t *link_next = link_index->next;
  link_t *link_previous = link_index->previous;

  if (link_next)
    {
      link_next->previous = link_index->previous;
    }

  if (link_previous)
    {
      link_previous->next = link_index->next;
    }

  //*elem = link_index->elem;
  if(delete == true)//om dlete är true så freea också annars låt va kvar
    {
      free(link_index);
    }
  --list->length;
}


//Returns pointer to element at index
bool list_get(list_t *list, int index, elem_t *result)
{
  //elem_t *elem = calloc(1, sizeof(elem_t)); //callocat!
  if(list_index_adress(list, index) == NULL)
    {
      false;
    }
  else
    {
      // result->p;
      link_t *link = list_index_adress(list, index); //link_t * ->elem
      *result = link->elem;
      //result->p = &((list_index_adress(list, index))->elem);
      return true;
    }
  return false;
}


//Returns the first element of list
bool list_first(list_t *list, elem_t *result)
{
  if(list->first==NULL)
    {
      return false;
    }
  *result = list->first->elem;
  return true;
}

//Returns the last element of list
bool list_last(list_t *list, elem_t *result)
{
  if(list->last==NULL)
    {
      return false;
    }
  *result = list->last->elem;
  return true;
}


//Returns the length of lsit
int list_length(list_t *list)
{
  return list->length;
}

/*bool list_apply(list_t *list, elem_apply_fun fun, void *data)
  {
  link_t link = list->first;
  }
*/

//applies an elem_apply_fun to every element, with void *data as the other argument, data can be NULL
bool list_apply(list_t *list, elem_apply_fun fun, void *data)
{
  bool result =true;
  for (link_t *cursor = list->first; cursor != NULL; cursor = cursor->next)
    {
      bool result2 = fun(cursor->elem, data);
      if(result2 == false)
        {
          result = false;
        }
    }
  return result;
}

//deletes the list completely if delete == true otherwise only puts cursors to NULL
void list_delete(list_t *list, bool delete)
{
  if(delete == true)
    {
      for (link_t *cursor = list->first; cursor != NULL; cursor = cursor->next)
        {
          element_free_fun freefun = list->free;
          freefun(cursor->elem);
        }
    }
  else
    {
      list->first = NULL;
      list->last = NULL; //Detta kan ju inte vara rätt, känns ju helt meningslöst??
    }
}  

int list_contains(list_t *list, elem_t elem)
{
  int index = 0;
  element_comp_fun compfun = list->comp;
  /* if(compfun != NULL)
    {
      for (link_t *cursor = list->first; cursor != NULL; cursor = cursor->next)
        {
          index++;
          if(cursor->elem == elem) //fattar inte vad som ska göras här??
            {return index;}
        }
    }
  else
  {*/
      for (link_t *cursor = list->first; cursor != NULL; cursor = cursor->next)
        {
          index++;
          if(compfun(cursor->elem, elem)==0)
            {return index;}
        }
      // }
  return -1;
}
