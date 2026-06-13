#include "fthread_internal.h"

static void* _standard_behavior  (void *param);
static void  _unlock_owned_locks (ft_thread_t thread);

/*****************************************************/
// the key for getting the fthread associated to a pthread
/*****************************************************/
static pthread_once_t _once_self_key = PTHREAD_ONCE_INIT;
static pthread_key_t _self_key;

static void _init_self_key (void)
{
   pthread_key_create (&_self_key,NULL);
}

/*****************************************************/
// the thread type
/*****************************************************/
struct ft_thread_t
{
   pthread_t                 pthread;      // underlying pthread
   int                       well_created; // equals FT_MAGIC if creation is OK

   pthread_mutex_t           lock;         // lock for protecting the thread
   pthread_cond_t            token;        // the token
   int                       has_token;    // does the thread owns the token ?

   ft_executable_t           cleanup;      // the function run when stopped   
   ft_executable_t           run;          // the function run by the thread
   void                     *args;         // parameters for run and cleanup

   ft_event_t                signal;       // termination signal

   char                      is_linked;    // is the thread linked ?
   ft_scheduler_t            scheduler;    // scheduler, when linked

   char                      status;       // status of the thread
   char                      is_suspended; // is the thread suspended ?

   item_list_t               locks;        // list of locks owned by the thread

   int                       deadline;     // limit time when waiting

   char                      is_automaton; // is it an automaton ?
   ft_automaton_t            automaton;    // automaton function
   int                       autom_state;  // automaton state
   void                     *local;        // local data
   void                     *special;      // special data, for implementation
   int                       return_code;  // return code for instructions run by the automaton
};

/*****************************************************/
// thread deallocation
/*****************************************************/
void ft_thread_deallocate (ft_thread_t thread)
{
   ft_event_deallocate   (thread->signal);
   _unlock_owned_locks   (thread);
   _deallocate_item_list (thread->locks);
   thread->well_created = 0; // the thread does not exist anymore
   free                  (thread);
}

/*****************************************************/
/* to determine if a thread has been corectly created by
   ft_thread_create or ft_automaton_create */
/*****************************************************/
int _thread_well_created (ft_thread_t thread)
{
   return  thread != NULL && thread->well_created == FT_MAGIC;
}

/*****************************************************/
// accessors
/*****************************************************/
int _get_thread_is_linked (ft_thread_t thread)
{
   return thread->is_linked;
}
void _set_thread_is_linked (ft_thread_t thread,char b)
{
   thread->is_linked = b;
}
/*********/
ft_scheduler_t _get_thread_scheduler (ft_thread_t thread)
{
   return thread->scheduler;
}

void _set_thread_scheduler (ft_thread_t thread,ft_scheduler_t sched)
{
   thread->scheduler = sched;
}
/*********/
ft_event_t _get_thread_signal (ft_thread_t thread)
{
   return thread->signal;
}

/*********/
int _get_thread_status (ft_thread_t thread)
{
   return thread->status;
}

void _set_thread_status (ft_thread_t thread,int status)
{
   thread->status = status;
}

/*********/
int _get_thread_deadline (ft_thread_t thread)
{
   return thread->deadline;
}

void _set_thread_deadline (ft_thread_t thread,int deadline)
{
   thread->deadline = deadline;
}
/*********/
int _is_automaton (ft_thread_t thread)
{
   return thread->is_automaton;
}
/*********/
int _get_thread_autom_state (ft_thread_t thread)
{
   return thread->autom_state;
}

void _set_thread_autom_state (ft_thread_t thread,int state)
{
   thread->autom_state = state;
}

/*********/
void _set_thread_args (ft_thread_t thread,void *args)
{
   thread->args = args;
}
void *_get_thread_args (ft_thread_t thread)
{
   return thread->args;
}
/*********/
void *_get_thread_local (ft_thread_t thread)
{
   return thread->local;
}
void _set_thread_local (ft_thread_t thread,void *data)
{
   thread->local = data;
}
/*********/
void* _get_thread_special (ft_thread_t thread)
{
   return thread->special;
}
void _set_thread_special (ft_thread_t thread,void *data)
{
   thread->special = data;
}
/*********/
int _get_thread_return_code (ft_thread_t thread)
{
   return thread->return_code;
}
void _set_thread_return_code (ft_thread_t thread,int code)
{
   thread->return_code = code;
}
/*********/
char _get_thread_is_suspended (ft_thread_t thread)
{
   return thread->is_suspended;
}
void _set_thread_is_suspended (ft_thread_t thread,char b)
{
   thread->is_suspended = b;
}

/************************************************/
// performs thread linking
/************************************************/
void _link_thread_to (ft_thread_t thread,ft_scheduler_t sched)
{
   thread->scheduler = sched;
   thread->is_linked = 1;
}

/************************************************/
// thread signaling (using the lock associated to the thread)
/************************************************/
void _lock_thread (ft_thread_t thread)
{
   _PTH_LOCK(thread->lock);
}

void _unlock_thread (ft_thread_t thread)
{
   _PTH_UNLOCK(thread->lock);
}

void _wait_thread (ft_thread_t thread)
{
   _PTH_WAIT(thread->token,thread->lock);
}

void _notify_thread (ft_thread_t thread)
{
   _PTH_NOTIFY(thread->token,thread->lock);
}

/*****************************************************/
// creation of threads
/*****************************************************/
ft_thread_t _make_thread (ft_executable_t cleanup,void *args)
{
   pthread_mutex_t lock;
   pthread_cond_t token;

   ft_thread_t thread = malloc (sizeof (struct ft_thread_t));
   if (thread == NULL) return NULL;

   pthread_mutex_init (&lock,NULL);
   pthread_cond_init  (&token,NULL);

   if (NULL == (thread->locks = _make_item_list ())) return NULL;

   thread->lock             = lock;
   thread->token            = token;
   
   thread->cleanup          = cleanup;
   thread->args             = args;
  
   thread->is_linked        = 0;

   thread->status           = _CONT;
   thread->has_token        = 0;

   thread->is_suspended     = 0;   
   thread->deadline         = -1;
   thread->is_automaton     = 0;
   
   return thread;
}

ft_thread_t ft_thread_create (ft_scheduler_t sched,
			      ft_executable_t runnable,
			      ft_executable_t cleanup,
			      void *args)
{
   ft_thread_t thread;
   if (!_scheduler_well_created (sched)) return NULL;
   pthread_once (&_once_self_key,_init_self_key);

   if (NULL == (thread = _make_thread (cleanup,args))) return NULL;
  
   _link_thread_to      (thread,sched);
   thread->signal       = ft_event_create (sched);

   thread->is_automaton = 0;
   thread->run          = runnable;
   // create the pthread and wait until it is running
   _lock_thread (thread);
   if (OK == pthread_create (&thread->pthread,NULL,_standard_behavior,thread))
       thread->well_created = FT_MAGIC;
   _wait_thread (thread);
   _unlock_thread (thread);
  
   return thread;
}

/*****************************************************/
// the scheduler in which the thread has been created (it never changes)
/*****************************************************/
ft_scheduler_t _creation_scheduler (ft_thread_t thread)
{
   return _get_event_scheduler (thread->signal);
}

/*****************************************************/
//  locks managed by ft_thread_mutex_lock and ft_thread_mutex_unlock
/*****************************************************/
// adds a lock to the list of locks owned by the thread
void _add_lock (ft_thread_t thread,pthread_mutex_t *mutex)
{
   _append_item_list (thread->locks,mutex);
}

static int _rem_lock (void *item,void *arg)
{
   return (item == arg) ? REMOVE : KEEP;
}

// remove a lock if owned by the thread
void _remove_lock (ft_thread_t thread,pthread_mutex_t *mutex)
{
   _apply_on_items (thread->locks,_rem_lock,mutex);
}

static int _unlock (void *item,void *arg)
{
   pthread_mutex_unlock ((pthread_mutex_t*)item);
   return REMOVE;
}

// unlock all locks owned by the thread
static void _unlock_owned_locks (ft_thread_t thread)
{
   _apply_on_items (thread->locks,_unlock,NULL);
}

/*****************************************************/
// token use
/*****************************************************/
// give the token to a thread, without expecting a return
void _give_token_to (ft_thread_t thread)
{
   _lock_thread (thread); 
   thread->has_token = 1;
   _notify_thread (thread);   
   _unlock_thread (thread);   
}

// the scheduler gives the token to a thread and waits the return of it
void _transmit_token_to_thread (ft_thread_t target)
{ 
   ft_scheduler_t sched = _get_thread_scheduler (target);
   ft_thread_t sched_thread = _get_scheduler_self (sched);
   _lock_thread (sched_thread);   
   _give_token_to (target);
   while (sched_thread->has_token == 0) _wait_thread (sched_thread);
   sched_thread->has_token = 0;
   _unlock_thread (sched_thread);   
}

/* the token is returned to the scheduler.*/
static void _return_token_to_scheduler (ft_thread_t me)
{ 
   _lock_thread (me);
   _give_token_to (_get_scheduler_self (me->scheduler));
   while (me->has_token == 0) _wait_thread (me);
   me->has_token = 0;
   _unlock_thread (me);   
}

/* the executing thread releases the token and sets its status  */
void _release (int status)
{
   ft_thread_t self = ft_thread_self ();
   _set_thread_status (self,status);
   _return_token_to_scheduler (self);
}

/*****************************************************/
// the executing fair thread and its scheduler
/*****************************************************/
ft_thread_t ft_thread_self (void)
{
   return pthread_getspecific (_self_key);
}

ft_scheduler_t ft_thread_scheduler (void)
{
   ft_thread_t self = pthread_getspecific (_self_key);
   if (self == NULL || !self->is_linked) return NULL;
   return self->scheduler;
}

/************************************************/
/* for join. thread termination can only be signaled in the initial
scheduler, in which the thread has been created.  this is actually the
scheduler of the signaling event. */
/************************************************/
static void _signal_term_if_possible (ft_thread_t thread)
{
   if (!thread->is_linked) return;
   if (thread->scheduler != _creation_scheduler (thread)) return;
   _event_generate (thread->signal);
}

/************************************************/
// when a thread is stopped
/************************************************/
void _finalize (ft_thread_t thread)
{
   _unlock_owned_locks (thread);
   _signal_term_if_possible (thread);
   if (thread->cleanup != NULL) thread->cleanup (thread->args);
   if (thread->is_automaton) return;
   pthread_cancel (ft_pthread (thread)); // can use pthread_join
}

/************************************************/
/* normal termination (ft_exit, end of the thread body or end of the
 * automaton)
 */
/************************************************/
void _terminate (ft_thread_t thread)
{
   thread->status = _TERM;
   _unlock_owned_locks (thread);
   _signal_term_if_possible (thread);
   if (thread->is_automaton) return;
   if (thread->is_linked) {
      _give_token_to (_get_scheduler_self (thread->scheduler));
   }
   pthread_exit (NULL); //never pass through
}

/*****************************************************/
/* the first thing to do: atomically register in the scheduler, notify
  the creation function, and wait the token from the scheduler */
/*****************************************************/
int _start_phase (ft_thread_t thread)
{
   int res;
   _lock_thread (thread);
   res = _register_as_runnable (thread);
   _notify_thread (thread);
   if (res == OK){
      while (thread->has_token == 0) _wait_thread (thread);
      thread->has_token = 0;
   }
   _unlock_thread (thread);
   return res;
}

/*****************************************************/
/* the standard behavior of a thread */
/*****************************************************/
static void* _standard_behavior (void *param)
{
   /*the current thread is the parameter */
   ft_thread_t me = param;
   pthread_setspecific (_self_key,me);
   if (_start_phase (me) != OK) return NULL; // cannot register => nothing done
   me->run (me->args);
   _terminate (me);         // normal termination
   return NULL;
}

/*****************************************************/
/* the pthread at the basis of a fair thread */
/*****************************************************/
pthread_t ft_pthread (ft_thread_t thread)
{
   _VERIFY_THREAD (thread);
   return thread->pthread;
}

/*****************************************************/
/* exit the executing pthread (unless if it is an automaton)  */
/*****************************************************/
void ft_exit (void)
{
   ft_thread_t self = ft_thread_self ();
   if (self != NULL) _terminate (self);
   else pthread_exit (NULL);
}

/*****************************************************/
// creation of automata
/*****************************************************/
ft_thread_t ft_automaton_create (ft_scheduler_t sched,
				 ft_automaton_t automaton,
				 ft_executable_t cleanup,
				 void *args)
{
   ft_thread_t thread;
   if (!_scheduler_well_created (sched)) return NULL;
   pthread_once (&_once_self_key,_init_self_key);

   if (NULL == (thread = _make_thread (cleanup,args))) return NULL;

   _link_thread_to      (thread,sched);
   thread->signal       = ft_event_create (sched);
   
   thread->is_automaton = 1;
   thread->automaton    = automaton;
   thread->autom_state  = 0;
   thread->local        = NULL;
   thread->special      = NULL;      
   thread->return_code  = OK;

   thread->well_created = FT_MAGIC; // before registrating...
   
   if (OK != _register_as_runnable (thread)) return NULL;
   
   return thread;
}

/*****************************************************/
// running an automaton
/*****************************************************/
void _run_as_automaton (ft_thread_t thread)
{
      ft_thread_t caller = ft_thread_self ();
      pthread_setspecific (_self_key,thread); 
      thread->automaton (thread);
      pthread_setspecific (_self_key,caller);
}

