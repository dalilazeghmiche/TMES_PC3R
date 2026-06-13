int              _scheduler_well_created     (ft_scheduler_t);

ft_thread_t      _get_scheduler_self         (ft_scheduler_t);
item_cell_t      _get_scheduler_current_cell (ft_scheduler_t);
int              _get_scheduler_eoi          (ft_scheduler_t);
void             _set_scheduler_move         (ft_scheduler_t);
int              _get_scheduler_instant      (ft_scheduler_t);

int              _register_unlink_order      (ft_thread_t);
int              _register_as_runnable       (ft_thread_t);

void             _unmask                     (ft_scheduler_t,item_cell_t);
void             _register_in_timer          (ft_scheduler_t);

void             ft_scheduler_deallocate     (ft_scheduler_t);
