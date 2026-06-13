#include "fthread.h"
#include <stdio.h>
#include <stdlib.h>

/* completely deterministic behavior */

ft_scheduler_t sched;
ft_thread_t t1,t2;
ft_event_t  e1,e2;

void run1 (void *args)
{
   int i;
   ft_thread_await (e1);
   ft_scheduler_stop (t2);
   //ft_thread_cooperate ();
   for (i=0;i<1000;i++) {
      fprintf (stdout,"*");
      ft_thread_cooperate ();
   }
}

void run2 (void *args)
{
   int i;
   ft_thread_await (e2);
   ft_scheduler_stop (t1);
   //ft_thread_cooperate ();
   for (i=0;i<1000;i++) {
      fprintf (stdout,"#");
      ft_thread_cooperate ();
   }
}

void gen (void *args)
{
   ft_thread_cooperate_n (100);
   ft_thread_generate (e2);
   ft_thread_generate (e1);   
}

void traceInstants (void *args)
{
   int i = 0;
   while (1) {
      fprintf (stdout,"\n>>>>>>>>>>> instant %d ",i++);
      ft_thread_cooperate ();
   }
}

int main (void)
{
  int c, *cell = &c;
  
  sched = ft_scheduler_create ();
  t1 = ft_thread_create (sched,run1,NULL,NULL);
  t2 = ft_thread_create (sched,run2,NULL,NULL);  
  e1 = ft_event_create (sched);
  e2 = ft_event_create (sched);  


  //ft_thread_create (sched,traceInstants,NULL,NULL);
  
  ft_thread_create (sched,gen,NULL,NULL);  

  ft_scheduler_start (sched);

  pthread_join (ft_pthread (t1),(void**)cell);
  pthread_join (ft_pthread (t2),(void**)cell);
  
  fprintf (stdout,"exit\n");
  exit (0);

  return 0;
}

/* result
*#exit
end result */
