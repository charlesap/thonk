/* begin-module-short-description
implements a ...
end-module-short-description */

/* begin-module-use-description

Module util

end-module-use-description */
/* begin-module-develop-description

(C. Perkins 2022)

**util** implements...

The structure... 

end-module-develop-description */



//#include <fcntl.h>
//#include <sys/mman.h>
//#include <sys/wait.h>



#include  <stdio.h>
#include  <stdlib.h>
//#include  <sys/stat.h>
//#include  <unistd.h>
//#include  <string.h>
//#include  <stdbool.h>
//#include  <pwd.h>
//#include  <sys/types.h>
//#include  <sys/ipc.h>
//#include  <sys/shm.h>
//#include  <sys/syscall.h>
//#include  <sys/sysinfo.h>
#include  <pthread.h>
 
#include  "thonk.h"

//#ifndef __NR_pidfd_open
//#define __NR_pidfd_open 434   /* System call # on most architectures */
//#endif
//#ifndef __NR_pidfd_getfd
//#define __NR_pidfd_getfd 438   /* System call # on most architectures */
//#endif

//    int return syscall(__NR_pidfd_open, pid, flags);
//    int return syscall(__NR_pidfd_getfd, pidfd, getfd, flags);



/* begin-section-description
## ---------- Global Variables
  end-section-description */

//struct Mailbox * MailPtr;
//int MailID;

/* begin-section-description
## ---------- Utility Functions
  end-section-description */




	
	
void display_pthread_attr(pthread_attr_t *attr, char *prefix)
{
    int s, i;
    size_t v;
    void *stkaddr;
    struct sched_param sp;

   s = pthread_attr_getdetachstate(attr, &i);
    if (s != 0)
        handle_error_en(s, "pthread_attr_getdetachstate");
    printf("%sDetach state        = %s\n", prefix,
            (i == PTHREAD_CREATE_DETACHED) ? "PTHREAD_CREATE_DETACHED" :
            (i == PTHREAD_CREATE_JOINABLE) ? "PTHREAD_CREATE_JOINABLE" :
            "???");

   s = pthread_attr_getscope(attr, &i);
    if (s != 0)
        handle_error_en(s, "pthread_attr_getscope");
    printf("%sScope               = %s\n", prefix,
            (i == PTHREAD_SCOPE_SYSTEM)  ? "PTHREAD_SCOPE_SYSTEM" :
            (i == PTHREAD_SCOPE_PROCESS) ? "PTHREAD_SCOPE_PROCESS" :
            "???");

   s = pthread_attr_getinheritsched(attr, &i);
    if (s != 0)
        handle_error_en(s, "pthread_attr_getinheritsched");
    printf("%sInherit scheduler   = %s\n", prefix,
            (i == PTHREAD_INHERIT_SCHED)  ? "PTHREAD_INHERIT_SCHED" :
            (i == PTHREAD_EXPLICIT_SCHED) ? "PTHREAD_EXPLICIT_SCHED" :
            "???");

   s = pthread_attr_getschedpolicy(attr, &i);
    if (s != 0)
        handle_error_en(s, "pthread_attr_getschedpolicy");
    printf("%sScheduling policy   = %s\n", prefix,
            (i == SCHED_OTHER) ? "SCHED_OTHER" :
            (i == SCHED_FIFO)  ? "SCHED_FIFO" :
            (i == SCHED_RR)    ? "SCHED_RR" :
            "???");

   s = pthread_attr_getschedparam(attr, &sp);
    if (s != 0)
        handle_error_en(s, "pthread_attr_getschedparam");
    printf("%sScheduling priority = %d\n", prefix, sp.sched_priority);

   s = pthread_attr_getguardsize(attr, &v);
    if (s != 0)
        handle_error_en(s, "pthread_attr_getguardsize");
    printf("%sGuard size          = %ld bytes\n", prefix, v);

   s = pthread_attr_getstack(attr, &stkaddr, &v);
    if (s != 0)
        handle_error_en(s, "pthread_attr_getstack");
    printf("%sStack address       = %p\n", prefix, stkaddr);
    printf("%sStack size          = 0x%lx bytes\n", prefix, v);
}

