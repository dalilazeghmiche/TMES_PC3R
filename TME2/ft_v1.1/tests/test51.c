#include "fthread.h"
#include <stdio.h>
#include <stdlib.h>

/* mutual stops */

#define MAX 10
ft_scheduler_t sched;
ft_thread_t *thread_table;
ft_event_t  *event_table;

void cleanup (void *args)
{
   int num = (int)args;
   ft_scheduler_stop (thread_table[(1+num)%MAX]);
   fprintf (stdout,"stopping thread %d\n",(1+num)%MAX);      
}


void run (void *args)
{
   int num = (int)args;
   ft_thread_await (event_table[num]);
   ft_scheduler_stop (thread_table[(1+num)%MAX]);
   while (1) ft_thread_cooperate ();
}

void gen (void *args)
{
   ft_thread_cooperate_n (100);
   ft_thread_generate (event_table[5]);
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
  int i, c, *cell = &c;
  
  sched        = ft_scheduler_create ();
  thread_table = malloc (MAX*sizeof (ft_thread_t));
  event_table  = malloc (MAX*sizeof (ft_event_t));
  
  for (i=0;i<MAX;i++){
     thread_table[i] = ft_thread_create (sched,run,cleanup,(void**)i);
     event_table[i]  = ft_event_create (sched);
  }

  //ft_thread_create (sched,traceInstants,NULL,NULL);
  
  ft_thread_create (sched,gen,NULL,NULL);  

  ft_scheduler_start (sched);

  for (i=0;i<MAX;i++){
     pthread_join (ft_pthread (thread_table[i]),(void**)cell);
  }
  fprintf (stdout,"exit\n");
  exit (0);

  return 0;
}

/* result
stopping thread 7
stopping thread 8
stopping thread 9
stopping thread 0
stopping thread 1
stopping thread 2
stopping thread 3
stopping thread 4
stopping thread 5
stopping thread 6
exit
end result */
