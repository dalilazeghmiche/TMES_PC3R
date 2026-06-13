#include "fthread.h"
#include <stdio.h>
#include <stdlib.h>

/*  */

/********************************/
void T1 (void *args)
{
   int max = (int)args;
   int i;
   for (i=0;i<max;i++) {
      fprintf (stderr,"T1(%d)\n",i);
      ft_thread_cooperate ();
   }
}

DEFINE_AUTOMATON (A1)
{
   int max = (int)ARGS;

   BEGIN_AUTOMATON
   STATE(0)
      {
	 SET_LOCAL(0);
      }
   STATE(1)
      {
	 int i = (int)LOCAL;
	 if (i >= max) RETURN;
	 fprintf (stderr,"A1(%d)\n",i);
	 SET_LOCAL(i+1);
	 GOTO(1);
      }
   END_AUTOMATON
}



/********************************/
void T2 (void *arg)
{
   void **args = arg;
   
   ft_thread_run_automaton (A1,args[1]);
   ft_thread_run_automaton (A1,args[1]);   

   fprintf (stderr,"middle: %s!\n",(char*)args[0]);      

   ft_thread_run (T1,args[1]);
   ft_thread_run (T1,args[1]);   
      
   fprintf (stderr,"exit: %s!\n",(char*)args[0]);
}

/********************************/
void T3 (void *arg)
{
   ft_thread_run (T2,arg);
   fprintf (stderr,"end of T3!\n");         
   exit (0);
}

/********************************/
int main (void)
{
   ft_scheduler_t sched = ft_scheduler_create ();

   void **params = malloc (2*sizeof (void*));
   params[0] = "i am T2";
   params[1] = (void*)5;

   ft_thread_create (sched,T3,NULL,params);

   ft_scheduler_start (sched);

   ft_exit ();
   return 0;
}

/* result

end result */
