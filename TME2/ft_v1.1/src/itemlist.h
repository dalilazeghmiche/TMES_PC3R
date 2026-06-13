#ifndef _ITEMLIST_H
#define _ITEMLIST_H

#define OK         0 // like for pthread: 0 means OK
#define EBADMEM    1 // not enough memory

/***************************************************************/
typedef struct item_cell_t *item_cell_t;
typedef struct item_list_t *item_list_t;

/***************************************************************/
item_list_t    _make_item_list        (void);
void           _reset_item_list       (item_list_t);
void           _deallocate_item_list  (item_list_t);

int            _length_of_item_list   (item_list_t);
item_cell_t    _first_cell            (item_list_t);
item_cell_t    _last_cell             (item_list_t);

int            _append_item_list      (item_list_t,void*);
int            _add_item_list         (item_list_t list,void*);
int            _concat_item_list      (item_list_t result,item_list_t);

void*          _nth_of_item_list      (item_list_t list,int n);

void*          _content_of_cell       (item_cell_t cell);
item_cell_t    _next_cell             (item_cell_t);

#define KEEP   1
#define REMOVE 2

typedef int    (*map_item_t)          (void *item,void *arg);
void           _apply_on_items        (item_list_t,map_item_t,void *arg);

/***************************************************************/
/***************************************************************/
#define MASK   3

typedef int    (*map_cell_t)          (item_cell_t,void *arg);
void           _apply_on_cells        (item_list_t,map_cell_t,void *arg);
void           _reinsert_cell         (item_list_t,item_cell_t);

/***************************************************************/

#endif /* _ITEMLIST_H */

/* item_list_t defines lists of cells. Each cell contains an
   item which is a void*. lists provide direct access to the
   first and last cells, and to the number of elements (length).
   New items are always appended at the end of the list.
   Each cell contains a pointer to the next cell in the list.
   In the standard way, a list is first created (make_item_list),
   then items are appended to it. The items can be accessed directly
   by their index (nth_of_item_list) or can be cyclically accessed
   by a loop of the form:
   
      item_cell_t cell = list->first;
      while (cell) {
         void *item = cell->item;
         ...
	 cell = cell->next;
      }

   The list can be reset (reset_item_list) which frees
   all cells in it. It can be deallocated using deallocate_item_list.

   All items in a list can be processed in turn using special functions
   of type map_item_t. Such a function takes an item and an argument
   in parameter, and returns KEEP when the item considered should
   remain in the list or REMOVE when it should be removed from the list.
   The way to process all items in a list is typically:

      int fun (void *item,void *arg)
      {
	 ...
	 if (to_keep) return KEEP;
	 else return REMOVE;
      }
      ...
      apply_on_items (list,fun,param);

   
     Item lists can also be used in an other context, where some cells
     can be temporally removed from the list, and reintroduced later.
     For this usage, one must use special functions of type map_fun_t
     which takes a cell and an argument in parameter, and returns KEEP
     when the cell considered remains in the list, REMOVE when it is
     definitevely removed, and MASK when it is temporally removed and
     thus can be reinserted later. The way to process all cells in a
     list is typically:

      int fun (item_cell_t cell,void *arg)
      {
         void *item = cell->item;
	 ...
	 if (to_keep) return KEEP;
	 if (to_rem) return REMOVE;
         // save cell to be later reintroduced
	 return MASK;
      }
      ...
      apply_on_cells (list,fun,param);

   A masked cell can be reintroduced in a list, at the same place,
   using reinsert_cell.  The important point is that masked cells do
   *not* introduce any overhead when processing the list. In
   particular, access to the next unmasked cell is constant in time
   through the next pointer of the cell.
   
*/
