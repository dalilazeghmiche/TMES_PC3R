#include "fthread.h"
#include <stdio.h>
#include <stdlib.h>

/*  */

#define STATE_AWAIT_COND(num,cond)          _STATE(num) _AWAIT_COND(cond)
#define STATE_AWAIT_COND_N(num,cond,delay)  _STATE(num) _AWAIT_COND_N(cond,delay)
#define _AWAIT_COND(cond)                   _STAY_OR_PASS(_automaton_await_cond(_self,cond))
#define _AWAIT_COND_N(cond,delay)           _STAY_OR_PASS(_automaton_await_cond_n(_self,cond,delay))

/********************************/
void T1 (void *args)
{
   int *cond = (int*)args;
   int i;
   for (i=0;i<10;i++) ft_thread_cooperate ();
   fprintf (stderr,"set!\n");
   (*cond) = 1;   
}

DEFINE_AUTOMATON (WAITCOND)
{
   int *cond = (int*)ARGS;

   BEGIN_AUTOMATON
   STATE(0){ fprintf (stderr,"waiting cond\n"); }
   STATE_AWAIT_COND_N(1,cond,5)
      {
	 if (RETURN_CODE==ETIMEOUT) fprintf (stderr,"timeout!\n");
	 else fprintf (stderr,"satisfied!\n");
      }
   END_AUTOMATON
}


/********************************/
void T2 (void *args)
{
   int *cond = (int*)args;
   ft_thread_await_cond (cond);
   fprintf (stderr,"satisfied!\n");

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
   int b = 0;
   ft_scheduler_t sched = ft_scheduler_create ();

   ft_thread_create (sched,TraceInstants,NULL,NULL);
   
   ft_thread_create (sched,T2,NULL,&b);
   ft_thread_create (sched,T1,NULL,&b);
   ft_automaton_create (sched,WAITCOND,NULL,&b);

   ft_scheduler_start (sched);

   ft_exit ();
   return 0;
}

/* result

end result */
