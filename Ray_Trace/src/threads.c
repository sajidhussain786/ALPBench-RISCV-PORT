/*
 * threads.c - code for spawning threads on various platforms.
 *
 *  $Id: threads.c,v 1.42 2009/04/23 15:01:01 johns Exp $
 */ 

#include "machine.h"
#include "threads.h"
#include "ui.h"

#ifdef _MSC_VER
#include <windows.h> /* main Win32 APIs and types */
#include <winbase.h> /* system services headers */
#endif

#if defined(SunOS) || defined(Irix) || defined(Linux) || defined(_CRAY) || defined(__osf__) || defined(AIX)
#include<unistd.h>  /* sysconf() headers, used by most systems */
#endif

#if defined(__APPLE__) && defined(THR)
#include <Carbon/Carbon.h> /* Carbon APIs for Multiprocessing */
#endif

#if defined(HPUX)
#include <sys/mpctl.h> /* HP-UX Multiprocessing headers */
#endif


int rt_thread_numprocessors(void) {
  int a=1;

#ifdef THR
#if defined(__APPLE__)
  a = MPProcessorsScheduled(); /* Number of active/running CPUs */
#endif

#ifdef _MSC_VER
  struct _SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  a = sysinfo.dwNumberOfProcessors; /* total number of CPUs */
#endif /* _MSC_VER */

#if defined(__PARAGON__) 
  a=2; /* Threads-capable Paragons have 2 CPUs for computation */
#endif /* __PARAGON__ */ 

#if defined(_CRAY)
  a = sysconf(_SC_CRAY_NCPU); /* total number of CPUs */
#endif

#if defined(SunOS) || defined(Linux) || defined(__osf__) || defined(AIX)
  a = sysconf(_SC_NPROCESSORS_ONLN); /* Number of active/running CPUs */
#endif /* SunOS */

#ifdef Irix
  a = sysconf(_SC_NPROC_ONLN); /* Number of active/running CPUs */
#endif /* IRIX */

#ifdef HPUX
  a = mpctl(MPC_GETNUMSPUS, 0, 0); /* total number of CPUs */
#endif /* HPUX */
#endif /* THR */

  return a;
}


int rt_thread_setconcurrency(int nthr) {
  int status=0;

#ifdef THR
#ifdef SunOS
  status = thr_setconcurrency(nthr);
#endif /* SunOS */

#if defined(Irix) || defined(AIX)
  status = pthread_setconcurrency(nthr);
#endif
#endif /* THR */

  return status;
}

int rt_thread_create(rt_thread_t * thr, void * fctn(void *), void * arg) {
  int status=0;

#ifdef THR
#ifdef _MSC_VER
  DWORD tid; /* thread id, msvc only */
  *thr = CreateThread(NULL, 8192, (LPTHREAD_START_ROUTINE) fctn, arg, 0, &tid);
  if (*thr == NULL) {
    status = -1;
  }
#endif /* _MSC_VER */

#ifdef USEPOSIXTHREADS 
#if defined(AIX)
  /* AIX schedule threads in system scope by default, have to ask explicitly */
  {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); 
    status = pthread_create(thr, &attr, fctn, arg);
    pthread_attr_destroy(&attr);
  } 
#elif defined(__PARAGON__)
  status = pthread_create(thr, pthread_attr_default, fctn, arg);
#else   
  status = pthread_create(thr, NULL, fctn, arg);
#endif 
#endif /* USEPOSIXTHREADS */

#ifdef USEUITHREADS 
  status = thr_create(NULL, 0, fctn, arg, 0, thr); 
#endif /* USEUITHREADS */
#endif /* THR */
 
  return status;
}


int rt_thread_join(rt_thread_t thr, void ** stat) {
  int status=0;  

#ifdef THR
#ifdef _MSC_VER
  DWORD wstatus = 0;
 
  wstatus = WAIT_TIMEOUT;
 
  while (wstatus != WAIT_OBJECT_0) {
    wstatus = WaitForSingleObject(thr, INFINITE);
  }
#endif /* _MSC_VER */

#ifdef USEPOSIXTHREADS
  status = pthread_join(thr, stat);
#endif /* USEPOSIXTHREADS */

#ifdef USEUITHREADS
  status = thr_join(thr, NULL, stat);
#endif /* USEPOSIXTHREADS */
#endif /* THR */

  return status;
}  


int rt_mutex_init(rt_mutex_t * mp) {
  int status=0;

#ifdef THR
#ifdef _MSC_VER
  InitializeCriticalSection(mp);
#endif /* _MSC_VER */

#ifdef USEPOSIXTHREADS
  status = pthread_mutex_init(mp, 0);
#endif /* USEPOSIXTHREADS */

#ifdef USEUITHREADS 
  status = mutex_init(mp, USYNC_THREAD, NULL);
#endif /* USEUITHREADS */
#endif /* THR */

  return status;
}


int rt_mutex_lock(rt_mutex_t * mp) {
  int status=0;

#ifdef THR
#ifdef _MSC_VER
  EnterCriticalSection(mp);
#endif /* _MSC_VER */

#ifdef USEPOSIXTHREADS
  status = pthread_mutex_lock(mp);
#endif /* USEPOSIXTHREADS */

#ifdef USEUITHREADS
  status = mutex_lock(mp);
#endif /* USEUITHREADS */
#endif /* THR */

  return status;
}


int rt_mutex_unlock(rt_mutex_t * mp) {
  int status=0;

#ifdef THR  
#ifdef _MSC_VER
  LeaveCriticalSection(mp);
#endif /* _MSC_VER */

#ifdef USEPOSIXTHREADS
  status = pthread_mutex_unlock(mp);
#endif /* USEPOSIXTHREADS */

#ifdef USEUITHREADS
  status = mutex_unlock(mp);
#endif /* USEUITHREADS */
#endif /* THR */

  return status;
}


int rt_mutex_destroy(rt_mutex_t * mp) {
  int status=0;

#ifdef THR
#ifdef _MSC_VER
  DeleteCriticalSection(mp);
#endif /* _MSC_VER */

#ifdef USEPOSIXTHREADS
  status = pthread_mutex_destroy(mp);
#endif /* USEPOSIXTHREADS */

#ifdef USEUITHREADS
  status = mutex_destroy(mp);
#endif /* USEUITHREADS */
#endif /* THR */

  return status;
}

/*
 * Condition variables
 */
int rt_cond_init(rt_cond_t * cvp) {
  int status=0;

#ifdef THR
#ifdef _MSC_VER
#if defined(RTUSEWIN2008CONDVARS)
  InitializeConditionVariable(cvp);
#else
  /* XXX not implemented */
  cvp->waiters = 0;

  // Create an auto-reset event.
  cvp->events[RT_COND_SIGNAL] = CreateEvent(NULL,  // no security
                                             FALSE, // auto-reset event
                                             FALSE, // non-signaled initially
                                             NULL); // unnamed

  // Create a manual-reset event.
  cvp->events[RT_COND_BROADCAST] = CreateEvent(NULL,  // no security
                                                TRUE,  // manual-reset
                                                FALSE, // non-signaled initially
                                                NULL); // unnamed
#endif
#endif /* _MSC_VER */

#ifdef USEPOSIXTHREADS
  status = pthread_cond_init(cvp, NULL);
#endif /* USEPOSIXTHREADS */
#ifdef USEUITHREADS
  status = cond_init(cvp, USYNC_THREAD, NULL);
#endif
#endif /* THR */

  return status;
}

int rt_cond_destroy(rt_cond_t * cvp) {
  int status=0;

#ifdef THR
#ifdef _MSC_VER
#if defined(RTUSEWIN2008CONDVARS)
  /* XXX not implemented */
#else
  CloseHandle(cvp->events[RT_COND_SIGNAL]);
  CloseHandle(cvp->events[RT_COND_BROADCAST]);
#endif
#endif /* _MSC_VER */

#ifdef USEPOSIXTHREADS
  status = pthread_cond_destroy(cvp);
#endif /* USEPOSIXTHREADS */
#ifdef USEUITHREADS
  status = cond_destroy(cvp);
#endif
#endif /* THR */

  return status;
}

int rt_cond_wait(rt_cond_t * cvp, rt_mutex_t * mp) {
  int status=0;
#if defined(THR) && defined(_MSC_VER)
  int result=0;
  LONG last_waiter;
  LONG my_waiter;
#endif

#ifdef THR
#ifdef _MSC_VER
#if defined(RTUSEWIN2008CONDVARS)
  SleepConditionVariableCS(cvp, mp, INFINITE)
#else
#if !defined(RTUSEINTERLOCKEDATOMICOPS)
  EnterCriticalSection(&cvp->waiters_lock);
  cvp->waiters++;
  LeaveCriticalSection(&cvp->waiters_lock);
#else
  InterlockedIncrement(&cvp->waiters);
#endif

  LeaveCriticalSection(mp); /* SetEvent() keeps state, avoiding lost wakeup */

  /* Wait either a single or broadcast even to become signalled */
  result = WaitForMultipleObjects(2, cvp->events, FALSE, INFINITE);

#if !defined(RTUSEINTERLOCKEDATOMICOPS)
  EnterCriticalSection (&cvp->waiters_lock);
  cvp->waiters--;
  last_waiter =
    ((result == (WAIT_OBJECT_0 + RT_COND_BROADCAST)) && cvp->waiters == 0);
  LeaveCriticalSection (&cvp->waiters_lock);
#else
  my_waiter = InterlockedDecrement(&cvp->waiters);
  last_waiter =
    ((result == (WAIT_OBJECT_0 + RT_COND_BROADCAST)) && my_waiter == 0);
#endif

  // Some thread called cond_broadcast()
  if (last_waiter)
    // We're the last waiter to be notified or to stop waiting, so
    // reset the manual event.
    ResetEvent(cvp->events[RT_COND_BROADCAST]);

  EnterCriticalSection(mp);
#endif
#endif /* _MSC_VER */

#ifdef USEPOSIXTHREADS
  status = pthread_cond_wait(cvp, mp);
#endif /* USEPOSIXTHREADS */
#ifdef USEUITHREADS
  status = cond_wait(cvp, mp);
#endif
#endif /* THR */

  return status;
}

int rt_cond_signal(rt_cond_t * cvp) {
  int status=0;

#ifdef THR
#ifdef _MSC_VER
#if defined(RTUSEWIN2008CONDVARS)
  WakeConditionVariable(cvp);
#else
#if !defined(RTUSEINTERLOCKEDATOMICOPS)
  EnterCriticalSection(&cvp->waiters_lock);
  int have_waiters = cvp->waiters > 0;
  LeaveCriticalSection(&cvp->waiters_lock);
  if (have_waiters)
    SetEvent (cvp->events[RT_COND_SIGNAL]);
#else
  if (InterlockedExchangeAdd(&cvp->waiters, 0) > 0)
    SetEvent(cvp->events[RT_COND_SIGNAL]);
#endif
#endif
#endif /* _MSC_VER */

#ifdef USEPOSIXTHREADS
  status = pthread_cond_signal(cvp);
#endif /* USEPOSIXTHREADS */
#ifdef USEUITHREADS
  status = cond_signal(cvp);
#endif
#endif /* THR */

  return status;
}

int rt_cond_broadcast(rt_cond_t * cvp) {
  int status=0;

#ifdef THR
#ifdef _MSC_VER
#if defined(RTUSEWIN2008CONDVARS)
  WakeAllConditionVariable(cvp);
#else
#if !defined(RTUSEINTERLOCKEDATOMICOPS)
  EnterCriticalSection(&cvp->waiters_lock);
  int have_waiters = cvp->waiters > 0;
  LeaveCriticalSection(&cvp->waiters_lock);
  if (have_waiters)
    SetEvent(cvp->events[RT_COND_BROADCAST]);
#else
  if (InterlockedExchangeAdd(&cvp->waiters, 0) > 0)
    SetEvent(cvp->events[RT_COND_BROADCAST]);
#endif

#endif
#endif /* _MSC_VER */

#ifdef USEPOSIXTHREADS
  status = pthread_cond_broadcast(cvp);
#endif /* USEPOSIXTHREADS */
#ifdef USEUITHREADS
  status = cond_broadcast(cvp);
#endif
#endif /* THR */

  return status;
}


/*
 * Reader/Writer locks -- slower than mutexes but good for some purposes
 */
int rt_rwlock_init(rt_rwlock_t * rwp) {
  int status=0;

#ifdef THR  
#ifdef USEPOSIXTHREADS
  pthread_mutex_init(&rwp->lock, NULL);
  pthread_cond_init(&rwp->rdrs_ok, NULL);
  pthread_cond_init(&rwp->wrtr_ok, NULL);
  rwp->rwlock = 0;
  rwp->waiting_writers = 0;
#endif /* USEPOSIXTHREADS */

#ifdef USEUITHREADS
  status = rwlock_init(rwp, USYNC_THREAD, NULL);
#endif /* USEUITHREADS */
#endif /* THR */

  return status;
}

int rt_rwlock_readlock(rt_rwlock_t * rwp) {
  int status=0;

#ifdef THR  
#ifdef USEPOSIXTHREADS
  pthread_mutex_lock(&rwp->lock);
  while (rwp->rwlock < 0 || rwp->waiting_writers) 
    pthread_cond_wait(&rwp->rdrs_ok, &rwp->lock);   
  rwp->rwlock++;   /* increment number of readers holding the lock */
  pthread_mutex_unlock(&rwp->lock);
#endif /* USEPOSIXTHREADS */

#ifdef USEUITHREADS
  status = rw_rdlock(rwp);
#endif /* USEUITHREADS */
#endif /* THR */

  return status;
}

int rt_rwlock_writelock(rt_rwlock_t * rwp) {
  int status=0;

#ifdef THR  
#ifdef USEPOSIXTHREADS
  pthread_mutex_lock(&rwp->lock);
  while (rwp->rwlock != 0) {
    rwp->waiting_writers++;
    pthread_cond_wait(&rwp->wrtr_ok, &rwp->lock);
    rwp->waiting_writers--;
  }
  rwp->rwlock=-1;
  pthread_mutex_unlock(&rwp->lock);
#endif /* USEPOSIXTHREADS */

#ifdef USEUITHREADS
  status = rw_wrlock(rwp);
#endif /* USEUITHREADS */
#endif /* THR */

  return status;
}

int rt_rwlock_unlock(rt_rwlock_t * rwp) {
  int status=0;

#ifdef THR  
#ifdef USEPOSIXTHREADS
  int ww, wr;
  pthread_mutex_lock(&rwp->lock);
  if (rwp->rwlock > 0) {
    rwp->rwlock--;
  } else {
    rwp->rwlock = 0;
  } 
  ww = (rwp->waiting_writers && rwp->rwlock == 0);
  wr = (rwp->waiting_writers == 0);
  pthread_mutex_unlock(&rwp->lock);
  if (ww) 
    pthread_cond_signal(&rwp->wrtr_ok);
  else if (wr)
    pthread_cond_signal(&rwp->rdrs_ok);
#endif /* USEPOSIXTHREADS */

#ifdef USEUITHREADS
  status = rw_unlock(rwp);
#endif /* USEUITHREADS */
#endif /* THR */

  return status;
}


rt_barrier_t * rt_thread_barrier_init(int n_clients) {
  rt_barrier_t *barrier = (rt_barrier_t *) malloc(sizeof(rt_barrier_t));

#ifdef THR
  if (barrier != NULL) {
    barrier->n_clients = n_clients;
    barrier->n_waiting = 0;
    barrier->phase = 0;
    barrier->sum = 0;
    rt_mutex_init(&barrier->lock);
    rt_cond_init(&barrier->wait_cv);
  }
#endif

  return barrier;
}

void rt_thread_barrier_destroy(rt_barrier_t *barrier) {
#ifdef THR
  rt_mutex_destroy(&barrier->lock);
  rt_cond_destroy(&barrier->wait_cv);
#endif
  free(barrier);
}

int rt_thread_barrier(rt_barrier_t *barrier, int increment) {
#ifdef THR
  int my_phase;
  int my_result;

  rt_mutex_lock(&barrier->lock);
  my_phase = barrier->phase;
  barrier->sum += increment;
  barrier->n_waiting++;

  if (barrier->n_waiting == barrier->n_clients) {
    barrier->result = barrier->sum;
    barrier->sum = 0;
    barrier->n_waiting = 0;
    barrier->phase = 1 - my_phase;
    rt_cond_broadcast(&barrier->wait_cv);
  }

  while (barrier->phase == my_phase) {
    rt_cond_wait(&barrier->wait_cv, &barrier->lock);
  }

  my_result = barrier->result;

  rt_mutex_unlock(&barrier->lock);

  return my_result; 
#else 
  return 0;
#endif
}



/* symmetric run barrier for use within a single process */
int rt_thread_run_barrier_init(rt_run_barrier_t *barrier, int n_clients) {
#ifdef THR
  if (barrier != NULL) {
    barrier->n_clients = n_clients;
    barrier->n_waiting = 0;
    barrier->phase = 0;
    barrier->fctn = NULL;

    rt_mutex_init(&barrier->lock);
    rt_cond_init(&barrier->wait_cv);
  }
#endif

  return 0;
}

void rt_thread_run_barrier_destroy(rt_run_barrier_t *barrier) {
#ifdef THR
  rt_mutex_destroy(&barrier->lock);
  rt_cond_destroy(&barrier->wait_cv);
#endif
}

/*
 * Wait until all threads reach barrier, and return the function
 * pointer passed in by the master thread.
 */
void * (*rt_thread_run_barrier(rt_run_barrier_t *barrier,
                               void * fctn(void*),
                               void * parms,
                               void **rsltparms))(void *) {
#if defined(THR)
  int my_phase;
  void * (*my_result)(void*);

  rt_mutex_lock(&barrier->lock);
  my_phase = barrier->phase;
  if (fctn != NULL)
    barrier->fctn = fctn;
  if (parms != NULL)
    barrier->parms = parms;
  barrier->n_waiting++;

  if (barrier->n_waiting == barrier->n_clients) {
    barrier->rslt = barrier->fctn;
    barrier->rsltparms = barrier->parms;
    barrier->fctn = NULL;
    barrier->parms = NULL;
    barrier->n_waiting = 0;
    barrier->phase = 1 - my_phase;
    rt_cond_broadcast(&barrier->wait_cv);
  }

  while (barrier->phase == my_phase) {
    rt_cond_wait(&barrier->wait_cv, &barrier->lock);
  }

  my_result = barrier->rslt;
  if (rsltparms != NULL)
    *rsltparms = barrier->rsltparms;

  rt_mutex_unlock(&barrier->lock);
#else
  void * (*my_result)(void*) = fctn;
  if (rsltparms != NULL)
    *rsltparms = parms;
#endif

  return my_result;
}



/*
 * Thread pool.
 */

#if defined(THR)
static void * rt_threadpool_workerproc(void *voidparms) {
  void *(*fctn)(void*);
  rt_threadpool_workerdata_t *workerdata = (rt_threadpool_workerdata_t *) voidparms;
  rt_threadpool_t *thrpool = (rt_threadpool_t *) workerdata->thrpool;

  while ((fctn = rt_thread_run_barrier(&thrpool->runbar, NULL, NULL, &workerdata->parms)) != NULL) {
    (*fctn)(workerdata);
  }

  return NULL;
}


static void * rt_threadpool_workersync(void *voidparms) {
  return NULL;
}
#endif

rt_threadpool_t * rt_threadpool_create(int workercount) {
  int i;
  rt_threadpool_t *thrpool = NULL;
  thrpool = (rt_threadpool_t *) malloc(sizeof(rt_threadpool_t));
  if (thrpool == NULL)
    return NULL;

  memset(thrpool, 0, sizeof(rt_threadpool_t));

#if !defined(THR)
  workercount=1;
#endif

  /* create a run barrier with N+1 threads: N workers, 1 master */
  thrpool->workercount = workercount;
  rt_thread_run_barrier_init(&thrpool->runbar, workercount+1);

  /* allocate and initialize thread pool */
  thrpool->threads = (rt_thread_t *) malloc(sizeof(rt_thread_t) * workercount);
  thrpool->workerdata = (rt_threadpool_workerdata_t *) malloc(sizeof(rt_threadpool_workerdata_t) * workercount);
  memset(thrpool->workerdata, 0, sizeof(rt_threadpool_workerdata_t) * workercount);

  /* setup per-worker data */
  for (i=0; i<workercount; i++) {
    thrpool->workerdata[i].threadid=i;
    thrpool->workerdata[i].threadcount=workercount;
    thrpool->workerdata[i].thrpool=thrpool;
  }

#if defined(THR)
  /* launch thread pool */
  for (i=0; i<workercount; i++) {
    rt_thread_create(&thrpool->threads[i], rt_threadpool_workerproc, &thrpool->workerdata[i]);
  }
#endif

  return thrpool;
}
int rt_threadpool_launch(rt_threadpool_t *thrpool,
                          void *fctn(void *), void *parms, int blocking) {
#if defined(THR)
  /* wake sleeping threads to run fctn(parms) */
  rt_thread_run_barrier(&thrpool->runbar, fctn, parms, NULL);
  if (blocking)
    rt_thread_run_barrier(&thrpool->runbar, rt_threadpool_workersync, NULL, NULL);
#else
  thrpool->workerdata[0].parms = parms;
  (*fctn)(&thrpool->workerdata[0]);
#endif
  return 0;
}

int rt_threadpool_wait(rt_threadpool_t *thrpool) {
#if defined(THR)
  rt_thread_run_barrier(&thrpool->runbar, rt_threadpool_workersync, NULL, NULL);
#endif
  return 0;
}

int rt_threadpool_destroy(rt_threadpool_t *thrpool) {
#if defined(THR)
  int i;
#endif

  /* wake threads and tell them to shutdown */
  rt_thread_run_barrier(&thrpool->runbar, NULL, NULL, NULL);

#if defined(THR)
  /* join the pool of worker threads */
  for (i=0; i<thrpool->workercount; i++) {
    rt_thread_join(thrpool->threads[i], NULL);
  }
#endif

  /* destroy the thread barrier */
  rt_thread_run_barrier_destroy(&thrpool->runbar);

  free(thrpool->threads);
  free(thrpool->workerdata);
  free(thrpool);

  return 0;
}

/* worker thread can call this to get its ID and number of peers */
int rt_threadpool_worker_getid(void *voiddata, int *threadid, int *threadcount) {
  rt_threadpool_workerdata_t *worker = (rt_threadpool_workerdata_t *) voiddata;
  if (threadid != NULL)
    *threadid = worker->threadid;

  if (threadcount != NULL)
    *threadcount = worker->threadcount;

  return 0;
}


/* worker thread can call this to get its client data pointer */
int rt_threadpool_worker_getdata(void *voiddata, void **clientdata) {
  rt_threadpool_workerdata_t *worker = (rt_threadpool_workerdata_t *) voiddata;
  if (clientdata != NULL)
    *clientdata = worker->parms;

  return 0;
}


/* initialize a shared iterator */
int rt_shared_iterator_init(rt_shared_iterator_t *it) {
  memset(it, 0, sizeof(rt_shared_iterator_t));
#if defined(THR)
  rt_mutex_init(&it->mtx);
#endif
  return 0;
}


/* destroy a shared iterator */
int rt_shared_iterator_destroy(rt_shared_iterator_t *it) {
#if defined(THR)
  rt_mutex_destroy(&it->mtx);
#endif
  return 0;
}


/* set shared iterator parameters */
int rt_shared_iterator_set(rt_shared_iterator_t *it, int start, int end) {
#if defined(THR)
  rt_mutex_lock(&it->mtx);
#endif
  it->start = start;
  it->current = start;
  it->end = end;
  it->fatalerror = 0;
#if defined(THR)
  rt_mutex_unlock(&it->mtx);
#endif
  return 0;
}


/* iterate the shared iterator */
int rt_shared_iterator_next(rt_shared_iterator_t *it, int *current) {
  int rc=ITERATOR_CONTINUE;

#if defined(THR)
  rt_mutex_lock(&it->mtx);
#endif
  if (!it->fatalerror) {
    *current = it->current;
    it->current++; /* this API only takes a single iteration, or work unit */
    if (*current >= it->end) {
      *current=0;
      rc = ITERATOR_DONE;
    }
  } else {
    rc = ITERATOR_DONE;
  }
#if defined(THR)
  rt_mutex_unlock(&it->mtx);
#endif

  return rc;
}


/* iterate the shared iterator, for a requested block size */
/* 'start' is inclusive, 'end' is exclusive                */
int rt_shared_iterator_next_block(rt_shared_iterator_t *it,
                                  int reqsize, int *start, int *end) {
  int rc=ITERATOR_CONTINUE;

#if defined(THR)
  rt_mutex_lock(&it->mtx);
#endif
  if (!it->fatalerror) {
    *start=it->current;   /* set start to the current work unit    */
    it->current+=reqsize; /* increment by the requested block size */
    *end=it->current;     /* set the (exclusive) endpoint          */

    /* if start is beyond the last work unit, we're done */
    if (*start >= it->end) {
      *start=0;
      *end=0;
      rc = ITERATOR_DONE;
    }

    /* if the endpoint (exclusive) for the requested block size */
    /* is beyond the last work unit, roll it back as needed     */
    if (*end > it->end) {
      *end = it->end;
    }
  } else {
    rc = ITERATOR_DONE;
  }
#if defined(THR)
  rt_mutex_unlock(&it->mtx);
#endif

  return rc;
}


/* worker thread calls this to indicate a fatal error */
int rt_shared_iterator_setfatalerror(rt_shared_iterator_t *it) {
#if defined(THR)
  rt_mutex_lock(&it->mtx);
#endif
  it->fatalerror=1;
#if defined(THR)
  rt_mutex_unlock(&it->mtx);
#endif
  return 0;
}


/* master thread calls this to query for fatal errors */
int rt_shared_iterator_getfatalerror(rt_shared_iterator_t *it) {
  int rc=0;
#if defined(THR)
  rt_mutex_lock(&it->mtx);
#endif
  if (it->fatalerror)
    rc = -1;
#if defined(THR)
  rt_mutex_unlock(&it->mtx);
#endif
  return rc;
}


