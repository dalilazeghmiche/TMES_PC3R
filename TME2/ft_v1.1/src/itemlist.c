#include <malloc.h>
#include "itemlist.h"

/********************************************************/
void internal_error    (char *format, ...);

#ifdef VERIF

static void control    (item_list_t,char*);
static void contained  (item_list_t,item_cell_t,char*);

#define CONTROL(list,msg) control (list,msg);
#define CONTAINED(list,cell,msg) contained (list,cell,msg);

#endif

/********************************************************/
struct item_cell_t
{
   void                *item;
   struct item_cell_t  *struct_previous; // should never change
   struct item_cell_t  *struct_next;     // should never change   
   struct item_cell_t  *previous;        // to previous unmasked cell   
   struct item_cell_t  *next;            // to next unmasked cell
   char                 masked;
};

struct item_list_t
{
   int                  length;         // number of unmasked cells
   struct item_cell_t  *first;
   struct item_cell_t  *last;
   struct item_cell_t  *end;
};

int _length_of_item_list (item_list_t list)
{
   return list->length;
}

item_cell_t _first_cell (item_list_t list)
{
   return list->first;
}

item_cell_t _last_cell (item_list_t list)
{
   return list->first;
}

void* _content_of_cell (item_cell_t cell)
{
   return cell->item;
}

item_cell_t _next_cell (item_cell_t cell)
{
   return cell->next;
}
/********************************************************/
// creation of cells
item_cell_t _make_item_cell (void *item)
{
   item_cell_t res = malloc (sizeof (struct item_cell_t));
   if (res == NULL) return NULL;
   
   res->item              = item;
   res->struct_previous   = NULL;
   res->struct_next       = NULL;   
   res->next              = NULL;
   res->previous          = NULL;   
   res->masked            = 0;
   return res;
}

// creation of lists
item_list_t _make_item_list (void)
{
   item_list_t res = malloc (sizeof (struct item_list_t));
   if (res == NULL) return NULL;
   
   res->length  = 0;
   res->first   = NULL;
   res->last    = NULL;
   res->end     = NULL;
   return res;
}

// reset a list and free all cells in it
void _reset_item_list (item_list_t list)
{
   item_cell_t cell = list->first;
   while (cell != NULL) {
      item_cell_t next = cell->next;
      free (cell);
      cell = next;
   }
   list->length  = 0;
   list->first   = NULL;
   list->last    = NULL;
   list->end     = NULL;   
}

// reset and free a list
void _deallocate_item_list (item_list_t list)
{
   _reset_item_list (list);
   free (list);
}

/**********************************************/
// content of the nth cell in a list
void* _nth_of_item_list (item_list_t list,int n)
{
   int i;
   item_cell_t cell;
   if (n >= list->length) return NULL;
   cell = list->first;
   for (i = 0; i < n; i++) {
      cell = cell->next;
      if (cell == NULL) return NULL;
   }
   return cell->item;
}

/***********************************************************/
// append a new item at the end of a list
int _append_item_list (item_list_t list,void *item)
{
   item_cell_t new = _make_item_cell (item);
   if (new == NULL) return EBADMEM;
   if (0 == list->length) {
      list->first = list->last = new;
   } else {
      list->last->next = new;
      new->previous = list->last;
      list->last = new;
   }
   // list end
   if (list->end) list->end->struct_next = new;
   new->struct_previous = list->end;
   list->end = new;
   // length
   list->length++;
   return OK;
}

// concat two lists and returns the first one
int _concat_item_list (item_list_t target,item_list_t list2)
{
   if (list2 != NULL && list2->length != 0) {
      item_cell_t cell = list2->first;
      while (cell != NULL) {
	 item_cell_t next = cell->next;
	 if (_append_item_list (target,cell->item) == EBADMEM) return EBADMEM;
	 cell = next;
      }
   }
   return OK;
}

// is an item contained in a cell in a list ?
static int _contains_item_list (item_list_t list,void *item)
{
   if (list->length != 0) {
      item_cell_t cell = list->first;
      while (cell != NULL) {
         if (cell->item == item) return 1;
	 cell = cell->next;
      }
   }
   return 0;
}

// add a new item if not already present in the list
int _add_item_list (item_list_t list,void *item)
{
   item_cell_t cell = list->first;
   while (cell != NULL) {
      if (cell->item == item) return OK;
      cell = cell->next;
   }
   return _append_item_list (list,item);
}

/***********************************************************/
/***********************************************************/
// remove a cell in a list
static void rem (item_list_t list,item_cell_t cell)
{
   if (list->length == 0) {
      internal_error ("rem on an empty itemlist\n");
   }
   list->length--;
   if (list->first == cell) { // first item
      list->first = cell->next;
      if (cell == list->last) {
	 list->last = NULL;
      } else cell->next->previous = NULL;
      return;
   }
   if (cell == list->last) { // last item
      cell->previous->next = NULL;
      list->last = cell->previous;
   } else {
      cell->previous->next = cell->next;
      cell->next->previous = cell->previous;
   }
}

// mask a cell in a list
static void mask (item_list_t list,item_cell_t cell)
{
#ifdef VERIF
   CONTAINED(list,cell,"mask attempt");
   CONTROL(list,"mask init");
#endif

   rem (list,cell);
   cell->masked = 1;
   
#ifdef VERIF
   CONTROL(list,"mask end");
#endif
}

// extract a cell from a list
static void extract (item_list_t list,item_cell_t cell)
{
#ifdef VERIF
   CONTAINED(list,cell,"extract attempt");
   CONTROL(list,"extract init");
#endif
   
   rem (list,cell);

   if (cell->struct_next)
      cell->struct_next->struct_previous = cell->struct_previous;
   if (cell->struct_previous)
      cell->struct_previous->struct_next = cell->struct_next;
   if (cell == list->end)
      list->end = list->end->struct_previous;

#ifdef VERIF
   CONTROL(list,"extract end");
#endif
}

/***********************************************************/
static item_cell_t previous_unmasked (item_cell_t cell)
{
   while (cell->struct_previous) {
      if (!cell->struct_previous->masked) return cell->struct_previous;
      cell = cell->struct_previous;
   }
   return NULL;
}

// apply a function f to all items
void _apply_on_items (item_list_t list,map_item_t f,void *arg)
{
   item_cell_t cell, previous = NULL;
   if (list->length == 0) return;
   cell = list->first;
   while (cell != NULL) {
      switch (f (cell->item,arg))
      {
	 case REMOVE:
	 {
            item_cell_t save = cell;
	    extract (list,cell);
            cell = cell->next;
            free (save);
            break;
	 }
	 case KEEP:
	    previous = cell;
            cell = cell->next;
	    break;
      }
   }
}

// apply a function f to all unmasked cells
void _apply_on_cells (item_list_t list,map_cell_t f,void *arg)
{
   item_cell_t cell, previous = NULL;
   if (list->length == 0) return;
   cell = list->first;
   while (cell != NULL) {
      switch (f (cell,arg))
      {
	 case REMOVE:
         {
	    item_cell_t save = cell;
	    extract (list,cell);
	    cell = cell->next;
            free (save);
            break;
	 }
	 case KEEP:
	    previous = cell;
	    cell = cell->next;
	    break;
	 case MASK:
	    mask (list,cell);
	    cell = cell->next;
            break;
      }
   }
}

// reinsert a masked cell (that was previously a list element)
// nothing done if the cell is already present in the list

void _reinsert_cell (item_list_t list,item_cell_t to_insert)
{
   item_cell_t prev;
#ifdef VERIF
   CONTROL(list,"reinsert init");
   CONTAINED(list,to_insert,"reinsert attempt");
#endif
   
   if (!to_insert->masked) return;
   to_insert->masked = 0;

   if (list->first == NULL) {// empty list
      list->first = list->last = to_insert;
      to_insert->next = NULL;
      to_insert->previous = NULL;
      list->length++;
#ifdef VERIF
      CONTROL(list,"reinsert end empty");
#endif
      return;
   }

   prev = previous_unmasked (to_insert);

   if (prev == NULL) {// reinsert at first place
      to_insert->next = list->first;
      list->first->previous = to_insert;
      list->first = to_insert;
      list->length++;
#ifdef VERIF
      CONTROL(list,"reinsert end first");
#endif
      return;
   }

   to_insert->next = prev->next;
   to_insert->previous = prev;
   if (prev->next) prev->next->previous = to_insert;
   prev->next = to_insert;
   if (prev == list->last) list->last = to_insert;
   
   list->length++;
#ifdef VERIF
   CONTROL(list,"reinsert end");
#endif
}

/**********************************************************/
#ifdef VERIF
static void control (item_list_t list,char *s)
{
   int i;
   item_cell_t cell;
   int n = list->length;

   if (n == 0) {
      if (list->first || list->last) {
	 internal_error ("itemlist: bad zero size - %s\n",s);
      }
      return;
   }
   cell = list->first;
   if (n == 1 && list->last != list->first) {
      internal_error ("itemlist: bad first/last - %s\n",s);
   }
   for (i = 0; i < n-1; i++) {
      if (cell->next == NULL) {
	 internal_error ("itemlist: bad next - %s\n",s);
      }
      cell = cell->next;
   }
   if (cell->next) {
      internal_error ("itemlist: bad end - %s\n",s);
   }

   cell = list->end;
   while (cell) {
      if (cell->struct_previous) {
	 if (cell->struct_previous->struct_next != cell) {
            internal_error ("itemlist: bad structure - %s\n",s);
	 }
      }
      cell = cell->struct_previous;
   }

   cell = list->first;
   while (cell) {
      if (cell->next) {
	 if (cell->next->previous != cell) {
            internal_error ("itemlist: bad linking - %s\n",s);
	 }
      }
      cell = cell->next;
   }
}

static void contained (item_list_t list,item_cell_t cell,char *s)
{
   item_cell_t considered;
   if (list->end == NULL) {
      internal_error ("itemlist: cell not in empty list - %s\n",s);
   }
   considered = list->end;
   while (considered) {
      if (considered == cell) return;
      considered = considered->struct_previous;
   }
   internal_error ("itemlist: cell not in list - %s\n",s);
}
#endif
