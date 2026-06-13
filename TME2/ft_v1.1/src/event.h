int              _event_well_created        (ft_event_t);

ft_scheduler_t   _get_event_scheduler       (ft_event_t);

int              _event_is_generated        (ft_event_t);

int              _fill_mask                 (int length,
					     ft_event_t *events,
					     int *mask);

void             _event_generate            (ft_event_t);

int              _event_generate_value      (ft_event_t,
					     void *value);

int              _event_get_value           (ft_event_t,
					     int index,
					     void **result);

void             _register_in_event         (ft_event_t);

void             _ft_event_deallocate       (ft_event_t);
