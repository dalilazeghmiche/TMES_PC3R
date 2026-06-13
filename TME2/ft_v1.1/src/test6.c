#include "fthread.h"
#include <stdio.h>
#include <stdlib.h>

/*  */
ft_scheduler_t sched;

DEFINE_AUTOMATON (MOVE)
{
   BEGIN_AUTOMATON
   STATE(0){
      fprintf (stderr,"in state0 ");
      GOTO_NEXT;
   }
   STATE(1){
      fprintf (stderr,"in state1 ");
      GOTO_NEXT;
   }
   STATE(2){
      fprintf (stderr,"the end ");
      GOTO_NEXT;
   }
   STATE(3){
      exit(0);
   }   
   END_AUTOMATON
}


/********************************/
void TraceInstants (void *args)
{
   int i = 0;
   while (i<1000) {
      fprintf (stderr,"\ninstant %d: ",i++);
      ft_thread_cooperate ();
   }
   //exit(0);
}
/********************************/
int main (void)
{

   sched = ft_scheduler_create ();   

   ft_thread_create (sched,TraceInstants,NULL,NULL);
   
   ft_automaton_create (sched,MOVE,NULL,NULL);

   ft_scheduler_start (sched);

   ft_exit ();
   return 0;
}

/* result

end result */
