#ifndef _BROADCAST_ORDER_H
#define _BROADCAST_ORDER_H

typedef struct _broadcast_order_t *broadcast_order_t;

broadcast_order_t _make_broadcast_order (ft_event_t,int,
					 void* value);

ft_event_t  _broadcast_order_event   (broadcast_order_t);
int         _broadcast_order_is_pure (broadcast_order_t);
void*       _broadcast_order_value   (broadcast_order_t);

#endif /* _BROADCAST_ORDER_H */
