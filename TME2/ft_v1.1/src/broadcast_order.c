#include "fthread_internal.h"

struct _broadcast_order_t
{
  ft_event_t  event;
  int         pure;
  void       *value;
};

/************************************************/
broadcast_order_t _make_broadcast_order (ft_event_t event,
					 int pure,
					 void *value)
{
   broadcast_order_t new = malloc (sizeof (struct _broadcast_order_t));
   if (new == NULL) return NULL;
   new->event  = event;
   new->pure   = pure;
   new->value  = value;
   return new;
}

ft_event_t _broadcast_order_event (broadcast_order_t broadcast)
{
   return broadcast->event;
}

int _broadcast_order_is_pure (broadcast_order_t broadcast)
{
   return broadcast->pure;
}

void* _broadcast_order_value (broadcast_order_t broadcast)
{
   return broadcast->value;
}
