#define FT_MAGIC 123456789

/***********************************************/
#define _VERIFY_SCHEDULER(sched)\
   if (!_scheduler_well_created (sched)) return EBADCREATE

#define _VERIFY_EVENT(event)\
   if (!_event_well_created (event)) return EBADCREATE

#define _VERIFY_THREAD(thread)\
   if (!_thread_well_created (thread)) return EBADCREATE

/***********************************************/
#define _VERIFY_THREAD_LINKED(thread)\
{\
   _VERIFY_THREAD(thread);\
   if (!_get_thread_is_linked (thread)) return EBADLINK;\
}

#define _VERIFY_THREAD_UNLINKED(thread)\
{\
   _VERIFY_THREAD(thread);\
   if (_get_thread_is_linked (thread)) return EBADLINK;\
}

/***********************************************/
#define _VERIFY_COOP \
{\
   ft_thread_t self  = ft_thread_self ();\
   _VERIFY_THREAD_LINKED (self);\
   _VERIFY_SCHEDULER (_get_thread_scheduler (self));\
}

/***********************************************/
#define _VERIFY_EVENT_INST(event)\
{\
   ft_thread_t self = ft_thread_self ();\
   ft_scheduler_t sched;\
   _VERIFY_EVENT (event);\
   _VERIFY_THREAD_LINKED (self);\
   sched = _get_thread_scheduler (self);\
   _VERIFY_SCHEDULER (sched);\
   if (sched != _get_event_scheduler (event)) return EBADLINK;\
}

/***********************************************/
#define _VERIFY_JOIN(thread)\
{\
   ft_thread_t self = ft_thread_self ();\
   ft_scheduler_t sched;\
   _VERIFY_THREAD_LINKED (self);\
   sched = _get_thread_scheduler (self);\
   _VERIFY_SCHEDULER (sched);\
   _VERIFY_THREAD (thread);\
   if (_creation_scheduler (thread) != sched) return EBADLINK;\
}

/***********************************************/
#define _VERIFY_EVENT_WITH_SCHEDULER(event,sched)\
{\
   _VERIFY_EVENT(event);\
   _VERIFY_SCHEDULER(sched);\
   if (sched != _get_event_scheduler (event)) return EBADLINK;\
}
