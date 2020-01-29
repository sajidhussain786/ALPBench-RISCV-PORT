/*
 * threads.h - code for spawning threads on various platforms.
 *
 *  $Id: threads.h,v 1.29 2009/04/22 15:43:14 johns Exp $
 */ 

#ifndef RT_THREADS_INC
#define RT_THREADS_INC 1

/* define which thread calls to use */
#if defined(USEPOSIXTHREADS) && defined(USEUITHREADS)
#error You may only define USEPOSIXTHREADS or USEUITHREADS, but not both
#endif

/* POSIX Threads */
#if defined(HPUX) || defined(__PARAGON__) || defined(Irix) || defined(Linux) ||     defined(_CRAY) || defined(__osf__) || defined(AIX) || defined(__APPLE__)
#if !defined(USEUITHREADS) && !defined(USEPOSIXTHREADS)
#define USEPOSIXTHREADS
#endif
#endif

/* Unix International Threads */
#if defined(SunOS)
#if !defined(USEPOSIXTHREADS) && !defined(USEUITHREADS)
#define USEUITHREADS
#endif
#endif


#ifdef THR
#ifdef USEPOSIXTHREADS
#include <pthread.h>

typedef pthread_t        rt_thread_t;
typedef pthread_mutex_t   rt_mutex_t;
typedef pthread_cond_t     rt_cond_t;

typedef struct rwlock_struct {
  pthread_mutex_t lock;         /* read/write monitor lock */
  int rwlock;                   /* >0 = #rdrs, <0 = wrtr, 0=none */
  pthread_cond_t  rdrs_ok;      /* start waiting readers */
  unsigned int waiting_writers; /* # of waiting writers  */
  pthread_cond_t  wrtr_ok;      /* start waiting writers */ 
} rt_rwlock_t;

#endif

#ifdef USEUITHREADS
#include <thread.h>

typedef thread_t  rt_thread_t;
typedef mutex_t   rt_mutex_t;
typedef cond_t    rt_cond_t;
typedef rwlock_t  rt_rwlock_t;
#endif



#ifdef _MSC_VER
#include <windows.h>
typedef HANDLE rt_thread_t;
typedef CRITICAL_SECTION rt_mutex_t;

#if 0 && (NTDDI_VERSION >= NTDDI_WS08 || _WIN32_WINNT > 0x0600)
/* Use native condition variables only with Windows Server 2008 and newer... */
#define RTUSEWIN2008CONDVARS 1
typedef CONDITION_VARIABLE rt_cond_t;
#else
/* Every version of Windows prior to Vista/WS2008 must emulate */
/* variables using manually resettable events or other schemes */

/* For higher performance, use interlocked memory operations   */
/* rather than locking/unlocking mutexes when manipulating     */
/* internal state.                                             */
#if 1
#define RTUSEINTERLOCKEDATOMICOPS 1
#endif
#define RT_COND_SIGNAL    0
#define RT_COND_BROADCAST 1
typedef struct {
  LONG waiters;     /* XXX this _MUST_ be 32-bit aligned for correct */
                    /* operation with the InterlockedXXX() APIs      */
  CRITICAL_SECTION waiters_lock;
  HANDLE events[2]; /* Signal and broadcast event HANDLEs. */
} rt_cond_t;
#endif

typedef HANDLE rt_rwlock_t;

#endif
#endif /* _MSC_VER */

#ifndef THR
typedef int rt_thread_t;
typedef int rt_mutex_t;
typedef int rt_cond_t;
typedef int rt_rwlock_t;
#endif

typedef struct barrier_struct {
  int padding1[8]; /* Padding bytes to avoid false sharing and cache aliasing */
  rt_mutex_t lock;        /* Mutex lock for the structure */
  int n_clients;          /* Number of threads to wait for at barrier */
  int n_waiting;          /* Number of currently waiting threads */
  int phase;              /* Flag to separate waiters from fast workers */
  int sum;                /* Sum of arguments passed to barrier_wait */
  int result;             /* Answer to be returned by barrier_wait */
  rt_cond_t wait_cv;      /* Clients wait on condition variable to proceed */
  int padding2[8]; /* Padding bytes to avoid false sharing and cache aliasing */
} rt_barrier_t;

typedef struct rt_run_barrier_struct {
  int padding1[8]; /* Padding bytes to avoid false sharing and cache aliasing */
  rt_mutex_t lock;       /* Mutex lock for the structure */
  int n_clients;          /* Number of threads to wait for at barrier */
  int n_waiting;          /* Number of currently waiting threads */
  int phase;              /* Flag to separate waiters from fast workers */
  void * (*fctn)(void *); /* Fctn ptr to call, or NULL if done */
  void * parms;           /* parms for fctn pointer */
  void * (*rslt)(void *); /* Fctn ptr to return to barrier wait callers */
  void * rsltparms;       /* parms to return to barrier wait callers */
  rt_cond_t wait_cv; /* Clients wait on condition variable to proceed */
  int padding2[8]; /* Padding bytes to avoid false sharing and cache aliasing */
} rt_run_barrier_t;

int rt_thread_numprocessors(void);
int rt_thread_setconcurrency(int);
int rt_thread_create(rt_thread_t *, void * routine(void *), void *);
int rt_thread_join(rt_thread_t, void **);

int rt_mutex_init(rt_mutex_t *);
int rt_mutex_lock(rt_mutex_t *);
int rt_mutex_unlock(rt_mutex_t *);
int rt_mutex_destroy(rt_mutex_t *);

int rt_cond_init(rt_cond_t *);
int rt_cond_destroy(rt_cond_t *);
int rt_cond_wait(rt_cond_t *, rt_mutex_t *);
int rt_cond_signal(rt_cond_t *);
int rt_cond_broadcast(rt_cond_t *);

int rt_rwlock_init(rt_rwlock_t *);
int rt_rwlock_readlock(rt_rwlock_t *);
int rt_rwlock_writelock(rt_rwlock_t *);
int rt_rwlock_unlock(rt_rwlock_t *);

/*
 * counting barrier
 */
rt_barrier_t * rt_thread_barrier_init(int n_clients);
void rt_thread_barrier_destroy(rt_barrier_t *barrier);
int rt_thread_barrier(rt_barrier_t *barrier, int increment);

/*
 * This is a symmetric barrier routine designed to be used
 * in implementing a sleepable thread pool.
 */
int rt_thread_run_barrier_init(rt_run_barrier_t *barrier, int n_clients);
void rt_thread_run_barrier_destroy(rt_run_barrier_t *barrier);
void * (*rt_thread_run_barrier(rt_run_barrier_t *barrier,
                                void * fctn(void*),
                                void * parms,
                                void **rsltparms))(void *);

/*
 * Thread pool.
 */
typedef struct rt_threadpool_workerdata_struct {
  int padding1[8]; /* Padding bytes to avoid false sharing and cache aliasing */
  int threadid;                          /* worker thread's id */
  int threadcount;                       /* total number of worker threads */
  void *parms;                           /* fctn parms for this worker */
  void *thrpool;                         /* void ptr to thread pool struct */
  int padding2[8]; /* Padding bytes to avoid false sharing and cache aliasing */
} rt_threadpool_workerdata_t;

typedef struct rt_threadpool_struct {
  int workercount;                        /* number of worker threads */
  rt_thread_t *threads;                 /* worker threads */
  rt_threadpool_workerdata_t *workerdata; /* per-worker data */
  rt_run_barrier_t runbar;              /* master/worker execution barrier */
} rt_threadpool_t;

/* create a thread pool with a specified number of worker threads */
rt_threadpool_t * rt_threadpool_create(int workercount);

/* launch threads onto a new function, with associated parms */
int rt_threadpool_launch(rt_threadpool_t *thrpool,
                         void *fctn(void *), void *parms, int blocking);

/* wait for all worker threads to complete their work */
int rt_threadpool_wait(rt_threadpool_t *thrpool);

/* join all worker threads and free resources */
int rt_threadpool_destroy(rt_threadpool_t *thrpool);

/* worker thread can call this to get its ID and number of peers */
int rt_threadpool_worker_getid(void *thrpool, int *threadid, int *threadcount);

/* worker thread can call this to get its client data pointer */
int rt_threadpool_worker_getdata(void *thrpool, void **clientdata);


/*
 * Shared iterators intended for trivial CPU/GPU load balancing with no
 * exception handling capability (all work units must complete with
 * no errors, or else the whole thing is canceled).
 */

#define ITERATOR_DONE     -1
#define ITERATOR_CONTINUE  0

typedef struct rt_shared_iterator_struct {
  rt_mutex_t mtx;     /* mutex lock */
  int start;           /* starting value */
  int end;             /* ending value */
  int current;         /* current value */
  int fatalerror;      /* cancel processing immediately for all threads */
} rt_shared_iterator_t;

/* initialize a shared iterator */
int rt_shared_iterator_init(rt_shared_iterator_t *it);

/* destroy a shared iterator */
int rt_shared_iterator_destroy(rt_shared_iterator_t *it);

/* Set shared iterator parameters: 'start' is inclusive, */
/* 'end' is exclusive.  This yields a half-open interval */
/* that corresponds to a typical 'for' loop.             */
int rt_shared_iterator_set(rt_shared_iterator_t *it, int start, int end);

/* iterate the shared iterator, returns -1 if no iterations left  */
/* or a fatal error has occured during processing, canceling all  */
/* worker threads                                                 */
int rt_shared_iterator_next(rt_shared_iterator_t *it, int *current);

/* iterate the shared iterator with a requested block size,       */
/* returns the resulting start/end for the block received, and    */
/* a return code of -1 if no iterations left or a fatal error     */
/* has occured during processing, canceling all worker threads    */
/* 'start' is inclusive, 'end' is exclusive. This yields a        */
/* half-open interval that corresponds to a typical 'for' loop.   */
int rt_shared_iterator_next_block(rt_shared_iterator_t *it,
                                  int reqsize, int *start, int *end);

/* worker thread calls this to indicate a fatal error */
int rt_shared_iterator_setfatalerror(rt_shared_iterator_t *it);

/* master thread calls this to query for fatal errors */
int rt_shared_iterator_getfatalerror(rt_shared_iterator_t *it);



#endif
