#include "fthread_internal.h"

struct _order_t
{
   int         kind;
   ft_thread_t thread;
};

/**********************************************/
order_t _make_order (int kind,ft_thread_t thread)
{
   order_t new = malloc (sizeof (struct _order_t));
   if (new == NULL) return NULL;
   new->kind   = kind;
   new->thread = thread;
   return new;
}

int _order_kind (order_t order)
{
   return order->kind;
}

ft_thread_t _order_thread (order_t order)
{
   return order->thread;
}

/**********************************************/
