#include <stdio.h>
#include <stdlib.h>

pthread_t t1,t2;
pthread_mutex_t lock1,lock2;
pthread_cond_t  e1,e2;

void* run1 (void *args)
{
   int i;
   ft_thread_await (e1);
   ft_scheduler_stop (t2);

   for (i=0;i<1000;i++) {
      fprintf (stdout,"*");
      ft_thread_cooperate ();
   }
}

void* run2 (void *args)
{
   int i;
   ft_thread_await (e2);
   ft_scheduler_stop (t1);

   for (i=0;i<1000;i++) {
      fprintf (stdout,"#");
      ft_thread_cooperate ();
   }
}

void gen (void *args)
{
   int i;
   ft_thread_cooperate_n (100);
   ft_thread_generate (e2);
   //ft_thread_generate (e1);   
}


int main (void)
{
  int c, *cell = &c;
  
  pthread_create (&t1,run1,NULL,NULL);
  pthread_create (&t2run2,NULL,NULL);  
  e1 = ft_event_create (sched);
  e2 = ft_event_create (sched);  


  ft_thread_create (sched,gen,NULL,NULL);  

  ft_scheduler_start (sched);

  pthread_join (t1,(void**)cell);
  pthread_join (t2,(void**)cell);
  
  fprintf (stdout,"exit\n");
  exit (0);

  return 0;
}

/* result

end result */
