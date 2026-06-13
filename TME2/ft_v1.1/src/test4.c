#include "fthread.h"
#include <stdio.h>
#include <stdlib.h>

/*  */

/********************************/
void GEN (void *args)
{
   ft_event_t event = (ft_event_t)args;
   int i;
   for (i=0;i<10;i++) {
      ft_thread_cooperate ();
   }
   fprintf (stderr,"generate!\n");
   ft_thread_generate (event);
   for (i=0;i<10;i++) {
      ft_thread_cooperate ();
   }
   fprintf (stderr,"generate!\n");
   ft_thread_generate (event);   
}

DEFINE_AUTOMATON (WAIT)
{
   ft_event_t event = (ft_event_t)ARGS;

   BEGIN_AUTOMATON
   STATE(0){ fprintf (stderr,"waiting event\n"); }
   STATE_AWAIT(1,event)
      {
	 fprintf (stderr,"received!\n");
      }
   END_AUTOMATON
}


/********************************/
void T1 (void *arg)
{
   ft_thread_run (GEN,arg);
}

void T2 (void *arg)
{
   ft_thread_run_automaton (WAIT,arg);
   fprintf (stderr,"middle!\n");
   ft_thread_run_automaton (WAIT,arg);   
   fprintf (stderr,"exit!\n");
   exit (0);
}

/********************************/
void TraceInstants (void *args)
{
   int i = 0;
   while (1) {
      fprintf (stderr,"\ninstant %d: ",i++);
      ft_thread_cooperate ();
   }
}
/********************************/
int main (void)
{
   ft_scheduler_t sched = ft_scheduler_create ();
   ft_event_t event = ft_event_create (sched);

   //ft_thread_create (sched,TraceInstants,NULL,NULL);
   
   ft_thread_create (sched,T2,NULL,event);
   ft_thread_create (sched,T1,NULL,event);

   ft_scheduler_start (sched);

   ft_exit ();
   return 0;
}

/* result

end result */
