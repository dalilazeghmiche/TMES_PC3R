#ifndef _ORDER_H
#define _ORDER_H

#define STOP_ORDER       1
#define SUSPEND_ORDER    2
#define RESUME_ORDER     3
#define LINK_ORDER       4
#define UNLINK_ORDER     5

typedef struct _order_t *order_t;

order_t     _make_order   (int kind,ft_thread_t);
int         _order_kind   (order_t);
ft_thread_t _order_thread (order_t);

#endif /* _ORDER_H */
