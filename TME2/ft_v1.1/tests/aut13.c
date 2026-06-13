#include "fthread.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/* preemption using two auxiliary automata */

/*************************************/
DEFINE_AUTOMATON(joiner)
{
   void **args = ARGS;
   ft_thread_t to_join = (ft_thread_t)(args[0]);
   ft_thread_t to_stop = (ft_thread_t)(args[1]);
   
  BEGIN_AUTOMATON
     
     STATE_JOIN(0,to_join)
     {
        ft_scheduler_stop (to_stop);        
        fprintf (stdout, "joiner terminates!\n");
     }
  
  END_AUTOMATON
}

void awaiter_cleanup (void *args)
{
   fprintf (stdout, "awaiter cleanup!\n");
}

DEFINE_AUTOMATON(awaiter)
{
   void **args = ARGS;
   ft_event_t  to_await = (ft_event_t) (args[0]);
   ft_thread_t to_stop  = (ft_thread_t)(args[1]);
   
  BEGIN_AUTOMATON
     
     STATE_AWAIT(0,to_await)
     {
        ft_scheduler_stop (to_stop);        
        fprintf (stdout, "awaiter terminates!\n");
     }
  
  END_AUTOMATON
}

DEFINE_AUTOMATON(until)
{
   void **args = ARGS;
   
   ft_scheduler_t    sched = (ft_scheduler_t)(args[0]);
   ft_event_t        event = (ft_event_t)    (args[1]);
   ft_executable_t   run   = (ft_executable_t)  (args[2]);
   
  BEGIN_AUTOMATON
     
     STATE(0)
     {
	ft_thread_t controlled,awaiter_aut;
        void **v1 = malloc (2*sizeof(void*)), **v2 = malloc (2*sizeof(void*));

        controlled = ft_thread_create (sched,run,NULL,NULL);

        v1[0] = event;
        v1[1] = controlled;
        awaiter_aut = ft_automaton_create (sched,awaiter,awaiter_cleanup,v1);

	v2[0] = controlled;
        v2[1] = awaiter_aut;
        ft_automaton_create (sched,joiner,NULL,v2);
     }
     
  END_AUTOMATON
}

/*************************************/
void printer (void *args)
{
   int i;
   for (i=0;i<5;i++) {
      fprintf (stdout, "*");
      ft_thread_cooperate ();
   }
   fprintf (stdout, "printer terminates!\n");   
}

/*************************************/
void generator (void *args)
{
   ft_thread_cooperate_n (11);
   ft_thread_generate ((ft_event_t)args);
   fprintf (stdout, "event generated!\n");
   ft_thread_cooperate_n (11);
   exit (0);

}

int main(void)
{
   void **v = malloc (3*sizeof(void*));
   ft_scheduler_t sched = ft_scheduler_create ();
   ft_event_t event = ft_event_create (sched);

   ft_thread_create (sched,generator,NULL,event); 

   v[0] = sched;
   v[1] = event;
   v[2] = printer;
   ft_automaton_create (sched,until,NULL,v);   

   ft_scheduler_start (sched);   
   
   ft_exit ();
   return 0;
}

/* result
*****printer terminates!
joiner terminates!
awaiter cleanup!
event generated!
end result */

