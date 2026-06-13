int              _thread_well_created       (ft_thread_t);

ft_scheduler_t   _get_thread_scheduler      (ft_thread_t);
void             _set_thread_scheduler      (ft_thread_t,ft_scheduler_t);
ft_scheduler_t   _creation_scheduler        (ft_thread_t);

int              _get_thread_is_linked      (ft_thread_t);
void             _set_thread_is_linked      (ft_thread_t,char);

ft_event_t       _get_thread_signal         (ft_thread_t);

int              _get_thread_status         (ft_thread_t);
void             _set_thread_status         (ft_thread_t,int status);

int              _get_thread_deadline       (ft_thread_t);
void             _set_thread_deadline       (ft_thread_t,int deadline);

int              _get_thread_autom_state    (ft_thread_t);
void             _set_thread_autom_state    (ft_thread_t,int state);

char             _get_thread_is_suspended   (ft_thread_t);
void             _set_thread_is_suspended   (ft_thread_t,char b);

void             _set_thread_args           (ft_thread_t,void *args);
void*            _get_thread_args           (ft_thread_t);        
 
ft_thread_t      _make_thread               (ft_executable_t cleanup,
					     void *args);

int              _start_phase               (ft_thread_t);
void             _stop_thread               (ft_thread_t);
void             _lock_thread               (ft_thread_t);
void             _unlock_thread             (ft_thread_t);

void             _link_thread_to            (ft_thread_t,ft_scheduler_t);

void             _release                   (int status);

void             _add_lock                  (ft_thread_t,pthread_mutex_t*);
void             _remove_lock               (ft_thread_t,pthread_mutex_t*);

void             _terminate                 (ft_thread_t);
void             _finalize                  (ft_thread_t);

void             _give_token_to             (ft_thread_t);
void             _transmit_token_to_thread  (ft_thread_t target);
 
int              _is_automaton              (ft_thread_t);

void*            _get_thread_local          (ft_thread_t);
void             _set_thread_local          (ft_thread_t,void *data);

void*            _get_thread_special        (ft_thread_t);
void             _set_thread_special        (ft_thread_t,void *data);

int              _get_thread_return_code    (ft_thread_t);
void             _set_thread_return_code    (ft_thread_t,int code);

void             _run_as_automaton          (ft_thread_t);

void             ft_thread_deallocate       (ft_thread_t);

/****************************************************************/
#define _POST_EVENT(event)\
 _register_in_event (event)

#define _POST_TIMER(self,sched,deadline)\
{\
   _set_thread_deadline (self,deadline);\
   _register_in_timer (sched);\
}

#define _CONTROL_TIMEOUT(deadline,sched) \
   if (deadline <= _get_scheduler_instant (sched)) return ETIMEOUT

/****************************************************************/
