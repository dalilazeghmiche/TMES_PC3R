#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_t       t1,t2;
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER,
                lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  e1    = PTHREAD_COND_INITIALIZER,
                e2    = PTHREAD_COND_INITIALIZER;

void susp (int n)
{
   sched_yield ();
   usleep(n);
}

void* run1 (void *args)
{
   int i;
   pthread_mutex_lock (&lock1);
   pthread_cond_wait (&e1,&lock1);
   pthread_mutex_unlock (&lock1);
   fprintf (stderr,"t1 starts\n");

   fprintf (stderr,"cancel t2\n");   
   pthread_cancel (t2);
   
   for (i=0;i<100;i++) {
      fprintf (stderr,"*");
      susp (1);
   }
   fprintf (stderr,"t1 terminated\n");
   return NULL;
}


void* run2 (void *args)
{
   int i;
   pthread_mutex_lock (&lock2);
   pthread_cond_wait (&e2,&lock2);
   pthread_mutex_unlock (&lock2);
   fprintf (stderr,"t2 starts\n");
   
   fprintf (stderr,"cancel t1\n");   
   pthread_cancel (t1);

   for (i=0;i<100;i++) {
      fprintf (stderr,"#");
      susp (1);
   }
   return NULL;
}

void* gen (void *args)
{
   usleep (1);

   pthread_mutex_lock (&lock1);
   pthread_mutex_lock (&lock2);

   pthread_cond_broadcast (&e1);
   fprintf (stdout,"broadcast e1\n");  
   pthread_cond_broadcast (&e2);
   fprintf (stdout,"broadcast e2\n");
   
   pthread_mutex_unlock (&lock1);
   pthread_mutex_unlock (&lock2);
   
   return NULL;
}


int main (void)
{
  int c, *cell = &c;
  pthread_t g;

  //e2 = e1;
  
  pthread_create (&t1,NULL,run1,NULL);
  pthread_create (&t2,NULL,run2,NULL);  

  pthread_create (&g,NULL,gen,NULL);  

  pthread_join (t1,(void**)cell);
  pthread_join (t2,(void**)cell);
  
  fprintf (stdout,"exit\n");
  exit (0);

  return 0;
}

/* result

end result */
