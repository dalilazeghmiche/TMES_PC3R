#include "fthread_internal.h"

#define VALUES_CHUNK 10

/*******************************************/
// the event type
/*******************************************/
struct ft_event_t
{
  int                instant;
  ft_scheduler_t     scheduler;
   
  void             **values;
  int                value_max;
  int                value_count;
   
  int                well_created; // == FT_MAGIC if creation is OK

  item_list_t        waiting;
};

/*******************************************/
// event deallocation
/*******************************************/
void ft_event_deallocate (ft_event_t event)
{
   free                  (event->values);
   _deallocate_item_list (event->waiting);
   event->well_created = 0; // the event does not exist anymore
   free                  (event);
}

/*******************************************/
// is the event well created by ft_event_create
/*******************************************/
int _event_well_created (ft_event_t event)
{
   return event != NULL && event->well_created == FT_MAGIC;
}

/*******************************************/
// the scheduler of the event
/*******************************************/
ft_scheduler_t _get_event_scheduler (ft_event_t event)
{
   return event->scheduler;
}

/*******************************************/
/* Attempt to place the nth value of an event in a result variable
 * (for get_value). Returns 1 if it succeeds and 0 otherwise. */
/*******************************************/
int _event_get_value (ft_event_t event,int index,void **result)
{
    if (_event_is_generated (event) && event->value_count > index) {
      (*result) = (event->values)[index];
      return 1;
    }
    return 0;
}

/*******************************************/
// allocation and reallocation for event values
/*******************************************/
// allocation of the array of values associated to an event
static void* _values_malloc (ft_event_t evt)
{
   evt->value_max = VALUES_CHUNK;
   evt->values = malloc (evt->value_max * sizeof (void*));
   return evt->values;
}

static void* _values_realloc (ft_event_t evt)
{
   evt->value_max += VALUES_CHUNK;
   evt->values = realloc (evt->values,evt->value_max * sizeof (void*));
   return evt->values;
}

/*******************************************/
// event creation
/*******************************************/
ft_event_t ft_event_create (ft_scheduler_t sched)
{
   ft_event_t new_event = malloc (sizeof (struct ft_event_t));
   if (new_event == NULL || !_scheduler_well_created (sched)) return NULL;  

   new_event->instant     = -1;
   new_event->scheduler   = sched;
   new_event->value_count = 0;

   if (NULL == _values_malloc (new_event)) return NULL;
   if (NULL == (new_event->waiting = _make_item_list ())) return NULL;

   new_event->well_created = FT_MAGIC;
   return new_event;
}

/*******************************************/
/* register in an event the thread contained in the current cell of
 * the event scheduler */
/*******************************************/
void _register_in_event (ft_event_t event)
{
   item_cell_t cell = _get_scheduler_current_cell (event->scheduler);
   _append_item_list (event->waiting,cell);
}

/********************************************/
// awake the waiting threads
/********************************************/
static int _awake_one (void *item,void *arg)
{
   item_cell_t cell = item;
   ft_scheduler_t sched = arg;
   ft_thread_t thread = _content_of_cell (cell);
   if (_get_thread_status (thread) == _WAIT) _unmask (sched,cell);
   return REMOVE;
}

static void _awake (ft_event_t event)
{
   _apply_on_items (event->waiting,
		    _awake_one,
		    _get_event_scheduler (event));
}

/*******************************************/
// tests if an event is present
/*******************************************/
int _event_is_generated (ft_event_t event)
{
   return event->instant == _get_scheduler_instant (event->scheduler);
}

/*******************************************/
/* for select: return presences of an array of events.  sets the mask
   and returns 1 if there is a generated event, 0 otherwise */
/*******************************************/
int _fill_mask (int length,ft_event_t *events,int *mask)
{
   int res = 0,i,pres;
   for (i = 0; i < length; i++) {
      if ((pres = _event_is_generated (events[i]))) res = 1;
      if (mask != NULL) mask[i] = pres;
   }
   return res;
}

/*******************************************/
/* event generation. precondition: the scheduler of the event and the
   one of the executing thread are the same */
/*******************************************/
void _event_generate (ft_event_t event)
{
   if (!_event_is_generated (event)) {
      ft_scheduler_t sched = event->scheduler;
      event->value_count = 0;  // reset event values;
      event->instant = _get_scheduler_instant (sched);
      _set_scheduler_move (sched);
      _awake (event);
   }
}

int _event_generate_value (ft_event_t event,void *val)
{
   _event_generate (event);
   // may be somebody waits for using the new value...
   _set_scheduler_move (event->scheduler);   
   if (event->value_count == event->value_max) {
      if (_values_realloc (event) == NULL) return EBADMEM;
   }
   event->values[event->value_count++] = val;
   return OK;
}

/*******************************************/
