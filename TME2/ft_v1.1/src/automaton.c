#include "fthread_internal.h"

/*********************************************************/
int _automaton_await (ft_thread_t self,ft_event_t event)
{
   _VERIFY_EVENT_INST (event);
   if (_event_is_generated (event)) return OK;
   _POST_EVENT (event);
   _set_thread_status (self,_WAIT);
   return _STAY;
}

int _automaton_await_n (ft_thread_t self,ft_event_t event,int delay)
{
   int current,deadline;
   ft_scheduler_t sched;

   _VERIFY_EVENT_INST (event);
   
   sched = _get_thread_scheduler (self);
   deadline = _get_thread_deadline (self);
   current = _get_scheduler_instant (sched);	   

   if (deadline == -1) {
      _set_thread_deadline (self,(deadline = delay + current));
   }

   if (deadline <= current) {
      _set_thread_deadline (self,-1);
      return ETIMEOUT;
   }
   if (_event_is_generated (event)) {
      _set_thread_deadline (self,-1);
      return OK;
   }
   _POST_TIMER (self,sched,deadline);
   _POST_EVENT (event);
   _set_thread_status (self,_WAIT);
   return _STAY;
}

/*********************************************************/
int _automaton_get_value (ft_thread_t self,
			  ft_event_t event,
			  int index,
			  void **result)
{
   ft_scheduler_t sched;

   _VERIFY_EVENT_INST (event);
   
   sched = _get_thread_scheduler (self);
   if (_get_thread_deadline (self) == _get_scheduler_instant (sched)) {
      _set_thread_deadline (self,-1);
      result = NULL;
      return ENEXT;      
   }
   if (_event_get_value (event,index,result)) return OK;
   if (_get_scheduler_eoi (sched) == 1) {
      _set_thread_deadline (self,_get_scheduler_instant (sched)+1);
      _set_thread_status (self,_COOP);
      return _STAY;
   }
   _set_thread_status (self,_CONT);
   return _STAY;
}

/*********************************************************/
int _automaton_stay (ft_thread_t self,int delay)
{
   int current,deadline;
   ft_scheduler_t sched;

   _VERIFY_THREAD_LINKED (self);
   
   sched = _get_thread_scheduler (self);
   deadline = _get_thread_deadline (self);
   current = _get_scheduler_instant (sched);	   

   if (deadline == -1) {
      _set_thread_deadline (self,(deadline = delay + current));
   }

   if (deadline > current) {
      _POST_TIMER (self,sched,deadline);
      _set_thread_status (self,_WAIT);
      return _STAY;      
   }
   _set_thread_deadline (self,-1);
   return OK;
}

/*********************************************************/
int _automaton_join (ft_thread_t self,ft_thread_t thread)
{
   _VERIFY_JOIN (thread);
   if (_get_thread_status (thread) == _TERM) return OK;
   return _automaton_await (self,_get_thread_signal (thread));
}

int _automaton_join_n (ft_thread_t self,ft_thread_t thread,int delay)
{
   _VERIFY_JOIN (thread);
   if (_get_thread_status (thread) == _TERM) return OK;
   return _automaton_await_n (self,_get_thread_signal (thread),delay);
}

/*********************************************************/
int _automaton_select (ft_thread_t self,
		       int length,
		       ft_event_t *events,
		       int *mask)
{
   int i;
   ft_scheduler_t sched;

   _VERIFY_THREAD_LINKED (self);

   sched = _get_thread_scheduler (self);
  
   for (i=0;i<length;i++) {
      _VERIFY_EVENT_WITH_SCHEDULER (events[i],sched);      
   }

   if (_fill_mask (length,events,mask)) {
      return OK;
   } else if (_get_scheduler_eoi (sched)) {
      _set_thread_status (self,_COOP);
      return _STAY;
   } else {
      for (i=0;i<length;i++) _POST_EVENT (events[i]);
      _set_thread_status (self,_WAIT);
      return _STAY;
   }
}

int _automaton_select_n (ft_thread_t self,
			 int length,
			 ft_event_t *events,
			 int *mask,
			 int delay)
{
   int current,deadline,i;
   ft_scheduler_t sched;

   _VERIFY_THREAD_LINKED (self);
   
   sched = _get_thread_scheduler (self);
   for (i=0;i<length;i++) {
      _VERIFY_EVENT_WITH_SCHEDULER (events[i],sched);
   }
   deadline = _get_thread_deadline (self);
   current = _get_scheduler_instant (sched);	   

   if (deadline == -1) {
      _set_thread_deadline (self,(deadline = delay + current));
   }
   _CONTROL_TIMEOUT (deadline,sched);
   if (_fill_mask (length,events,mask)) {
      _set_thread_deadline (self,-1);
      return OK;
   } else if (_get_scheduler_eoi (sched)) {
      _set_thread_status (self,_COOP);
      return _STAY;
   } else {
      for (i=0;i<length;i++) _POST_EVENT (events[i]);
      _POST_TIMER (self,sched,deadline);	 
      _set_thread_status (self,_WAIT);
      return _STAY;
   }
} 

