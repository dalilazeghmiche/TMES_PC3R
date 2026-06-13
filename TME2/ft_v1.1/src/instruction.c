#include "fthread_internal.h"

/************************************************/
/*
 */
/************************************************/
int ft_thread_cooperate ()
{
   _VERIFY_COOP;
   
   _release (_COOP);
   return OK;
} 

int ft_thread_cooperate_n (int delay)
{ 
   int deadline;
   ft_thread_t self = ft_thread_self ();
   ft_scheduler_t sched;

   _VERIFY_COOP;
   
   sched = _get_thread_scheduler (self);
   deadline = delay + _get_scheduler_instant (sched);
   
   while (1) {
      if (deadline <= _get_scheduler_instant (sched)) return OK;
      _POST_TIMER (self,sched,deadline);
      _release (_WAIT);      
   }
}

/************************************************/
/* VERIFY_EVENT_INST: The event is well-created. The executing thread
   is well created.  The current scheduler is the one of the event.
*/
/************************************************/
int ft_thread_await (ft_event_t event)
{
   _VERIFY_EVENT_INST (event);
   
   while (1) {
      if (_event_is_generated (event)) return OK; 
      _POST_EVENT (event);
      _release (_WAIT);
   } 
}

int ft_thread_await_n (ft_event_t event,int delay)
{ 
   int deadline;
   ft_thread_t self = ft_thread_self ();
   ft_scheduler_t sched;

   _VERIFY_EVENT_INST (event);
   
   sched = _get_thread_scheduler (self);
   deadline = delay + _get_scheduler_instant (sched);
  
   while(1) {
      _CONTROL_TIMEOUT (deadline,sched);
      if (_event_is_generated (event)) return OK;
      _POST_TIMER (self,sched,deadline);	 
      _POST_EVENT (event);
      _release (_WAIT);
   }
}

/************************************************/
/*
 */
/************************************************/
int ft_thread_join (ft_thread_t thread)
{
   _VERIFY_JOIN (thread);
   if (_get_thread_status (thread) == _TERM) return OK;
   return ft_thread_await (_get_thread_signal (thread));
}

int ft_thread_join_n (ft_thread_t thread,int delay)
{
   _VERIFY_JOIN (thread);
   if (_get_thread_status (thread) == _TERM) return OK;
   return ft_thread_await_n (_get_thread_signal (thread),delay);
}

/************************************************/
int ft_thread_get_value (ft_event_t event,int index,void **result)
{
   ft_scheduler_t sched;

   _VERIFY_EVENT_INST (event);
   
   sched = _get_event_scheduler (event);
   while(1) {
      if (_event_get_value (event,index,result)) return OK;
      if (_get_scheduler_eoi (sched) == 1) break;
      _release (_CONT);
   }
   _release (_COOP);
   result = NULL;
   return ENEXT;
}

/************************************************/
/*
 */
/************************************************/
int ft_thread_generate (ft_event_t event)
{
   _VERIFY_EVENT_WITH_SCHEDULER(event,ft_thread_scheduler ());
   _event_generate (event);
   return OK;
}

int ft_thread_generate_value (ft_event_t event,void* val)
{
   _VERIFY_EVENT_WITH_SCHEDULER(event,ft_thread_scheduler ());
   return _event_generate_value (event,val);
}

/************************************************/
/*
 */
/************************************************/
int ft_thread_mutex_lock (pthread_mutex_t *mutex)
{
   ft_thread_t    self  = ft_thread_self ();
   ft_scheduler_t sched = ft_thread_scheduler ();
   
   if (self == NULL || sched == NULL) return pthread_mutex_lock(mutex);

   _add_lock (self,mutex);
   
   while(1){
     if (_get_scheduler_eoi (sched) == 1) {
	_release (_COOP);
     }else if (pthread_mutex_trylock (mutex) == OK) {
	return OK;
     }else{
	_release (_CONT);
     }
   }
}

int ft_thread_mutex_unlock (pthread_mutex_t *mutex)
{
   ft_thread_t self = ft_thread_self ();
   
   int res = pthread_mutex_unlock (mutex);
   
   if (self != NULL) {
      ft_scheduler_t sched = _get_thread_scheduler (self);
      if (sched != NULL){
	 _set_scheduler_move (sched);
         _remove_lock (self,mutex);
      }
   }
   return res;
}

/************************************************/
/*
 */
/************************************************/
int ft_thread_select (int length,ft_event_t *events,int *mask)
{
   int i;
   ft_thread_t    self  = ft_thread_self ();
   ft_scheduler_t sched = ft_thread_scheduler ();

   _VERIFY_THREAD_LINKED (self);

   for (i=0;i<length;i++) {
      _VERIFY_EVENT_WITH_SCHEDULER (events[i],sched);      
   }

   while (1) {
      if (_fill_mask (length,events,mask)) return OK;
      else if (_get_scheduler_eoi (sched)) _release (_COOP);
      else {
	 for (i=0;i<length;i++) _POST_EVENT (events[i]);
	 _release (_WAIT);
      }
   } 
}

int ft_thread_select_n (int length,ft_event_t *events,int *mask,int delay)
{
   int deadline,i;
   ft_thread_t    self  = ft_thread_self ();
   ft_scheduler_t sched = ft_thread_scheduler ();

   _VERIFY_THREAD_LINKED (self);

   for (i=0;i<length;i++) {
      _VERIFY_EVENT_WITH_SCHEDULER (events[i],sched);
   }

   deadline = delay + _get_scheduler_instant (sched);
   
   while (1) {
      _CONTROL_TIMEOUT (deadline,sched);
      if (_fill_mask (length,events,mask)) return OK;
      else if (_get_scheduler_eoi (sched)) _release (_COOP);
      else {
	 for (i=0;i<length;i++) _POST_EVENT (events[i]);
	 _POST_TIMER (self,sched,deadline);	 
	 _release (_WAIT);
      }
   } 
}

/************************************************/
/*
 */
/************************************************/
int ft_thread_unlink (void)
{
   ft_thread_t self = ft_thread_self ();
   int res = _register_unlink_order (self); // verifications by it
   if (res != OK) return res;
   return ft_thread_cooperate ();
}

int ft_thread_link (ft_scheduler_t sched)
{
   ft_thread_t self  = ft_thread_self ();
   _VERIFY_THREAD_UNLINKED (self);
   _link_thread_to (self,sched);
   return _start_phase (self); // verifications by it
}

