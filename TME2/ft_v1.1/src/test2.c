#include "fthread.h"
#include "stdio.h"

/*  */
ft_scheduler_t sched;

/********************************/
void T1 (void *args)
{
   int i;
   for (i=0;i<10;i++) {
      fprintf (stderr,"T1(%d) ",i);
      ft_thread_cooperate ();
   }
}

/********************************/
DEFINE_AUTOMATON(A1)
{
   BEGIN_AUTOMATON
      STATE(0)
      {
	 SET_LOCAL(0);
      }
      STATE(1)
      {
         int i = (int)LOCAL;
	 if (i>=10) RETURN; 
         fprintf (stderr,"A1(%d) ",i);
         SET_LOCAL(i+1);
         GOTO(1);
      }
   END_AUTOMATON
}

/********************************/
void run_automaton (ft_automaton_t aut,void *arg)
{
   ft_thread_t t = _make_automaton (ft_thread_scheduler (),aut,NULL,arg);
   while (_get_thread_status(t)!=_TERMINATED){
      _run_as_automaton (t);
      ft_thread_cooperate ();
   }
}
void run_native (ft_executable_t ex,void *arg)
{
   ex (arg);
}

/********************************/
DEFINE_AUTOMATON(RUN_AUT)
{
   void **params = ARGS;
   ft_automaton_t aut = (ft_automaton_t)params[0];
   void *arg = params[1];

   BEGIN_AUTOMATON
      STATE(0)
      {
         SET_LOCAL (_make_automaton (ft_thread_scheduler (),aut,NULL,arg));
      }
      STATE(1)
      {
	 ft_thread_t t = (ft_thread_t)LOCAL;
	 if (_get_thread_status(t)!=_TERMINATED){
	    _run_as_automaton (t);
	    GOTO(1);
	 }
      }
   END_AUTOMATON
}

DEFINE_AUTOMATON(RUN_NATIVE)
{
   void **params = ARGS;
   ft_executable_t ex = (ft_executable_t)params[0];
   void *arg = params[1];

   BEGIN_AUTOMATON
      STATE(0)
      {
         SET_LOCAL (ft_thread_create (ft_thread_scheduler (),ex,NULL,arg));
      }
      STATE_JOIN(1,LOCAL)
      {
      }
   END_AUTOMATON
}

/********************************/

void T2 (void *args)
{
   int i;
   for (i=0;i<20;i++) {

      //run_native (T1,args); 
      //run_automaton (A1,args);
/*
      void **params = malloc (2*sizeof (void*));
      params[0] = A1;
      params[1] = NULL;
      run_automaton (RUN_AUT,params);
*/
      void **params = malloc (2*sizeof (void*));
      params[0] = T1;
      params[1] = NULL;
      run_automaton (RUN_NATIVE,params);
      
      ft_thread_cooperate ();
      fprintf (stderr,"\n");      
   }
   exit (0);
   
}

/********************************/
int main (void)
{
   sched = ft_scheduler_create ();
   ft_thread_create (sched,T2,NULL,NULL);
  
   ft_scheduler_start (sched);

   ft_exit ();
   return 0;
}

/* result

end result */
