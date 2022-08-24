/* begin-package-description

**Package Overview**

Thonk...


**Package Usage**

To...


end-package-description */

/* begin-module-short-description
implements a ...
end-module-short-description */

/* begin-module-use-description

Module OXP reads the source code of an Oberon program and produces an executable binary module.

end-module-use-description */
/* begin-module-develop-description

(C. Perkins 2022)

**thonk** implements...

The structure... 

end-module-develop-description */



#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>



#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/stat.h>
#include  <unistd.h>
#include  <string.h>
#include  <stdbool.h>
#include  <pwd.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include  <sys/syscall.h>
#include  <sys/sysinfo.h>
#include  <pthread.h>
 
#include  "thonk.h"

#ifndef __NR_pidfd_open
#define __NR_pidfd_open 434   /* System call # on most architectures */
#endif
#ifndef __NR_pidfd_getfd
#define __NR_pidfd_getfd 438   /* System call # on most architectures */
#endif

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



  /* begin-procedure-description
---
**sigsegv_handler** handles segmentation faults from reading the protected page at the base of the thread's stack.
  end-procedure-description */
void sigsegv_handler(int signum, siginfo_t *info, void *data) {
//    void *addr = info->si_addr;
//    int info = coro_t::in_coro_from_cpu(addr);
//    if (cpu == -1) {
//        crash("Segmentation fault from reading the address %p.", addr);
//    } else {
//        crash("Callstack overflow from a coroutine initialized on CPU %d at address %p.", cpu, addr);
//    }
    printf("Segmentation Fault Not Really Handled.\n");
    exit(1);
}


struct thread_args {
    char* name;
    void* stack;
    pthread_attr_t attr;
};



  /* begin-procedure-description
---
**myThreadFun** makes a thread.
  end-procedure-description */
void *myThreadFun(void *vargp)
{
    struct thread_args * tha = (struct thread_args*)vargp;
    int psz = getpagesize();
    size_t ssz; 
    void * stack;
    struct sigaction action;

    pthread_attr_t * pttr = &tha->attr;

//   printf("In Thread attributes:\n");
//    display_pthread_attr(pttr, "\t");

    pthread_attr_getstack(pttr,&stack,&ssz);

    mprotect(stack, psz, PROT_NONE);

    bzero(&action, sizeof(action));
    action.sa_flags = SA_SIGINFO|SA_STACK;
    action.sa_sigaction = &sigsegv_handler;
    sigaction(SIGSEGV, &action, NULL);
 
    stack_t segv_stack;
    segv_stack.ss_sp = valloc(SEGV_STACK_SIZE);
    segv_stack.ss_flags = 0;
    segv_stack.ss_size = SEGV_STACK_SIZE;
    sigaltstack(&segv_stack, NULL);


    sleep(.1);
    printf("In Thread %s stack at %p \n", tha->name, stack);
 //   char * foo = stack;
 //   foo[0]=0;   /* cause a fault */
    
    mprotect(stack, psz, PROT_READ|PROT_WRITE);
//    free(stack);
    return NULL;
}

/* begin-section-description
## ---------- Initialization
  end-section-description */


/* begin-procedure-description
---
**server** accepts client connections via shared memory and ipc
  end-procedure-description */
void  server(int  argc, char *argv[], struct MailFrame * frame)
{
     pthread_t worker[MAXTHREADS];
     struct thread_args wargs[MAXTHREADS];

     char * astring = "placeholder";

     int workers = 0;
     int ERRVAL = 0;
     
     if (argc != 2) {
          printf("Use: %s \"message\"\n", argv[0]);
	  ERRVAL = 1;

     }else{

          frame->box->condition  = UNPREPARED;
          frame->box->corepid  = 0;
          frame->box->corefd  = 0;
          strncpy(frame->box->data, argv[1], 63);
          frame->box->data[63] = '\0';
          frame->box->condition = PREPARED;
          printf("Ready for connections.\n");
                      
          while (frame->box->condition < READ3)
               sleep(1);
               
     
          workers = get_nprocs();
          if (workers > MAXTHREADS) workers = MAXTHREADS;
     
          printf("This system has %d processors configured and "
                 "%d processors available.\n",
                 get_nprocs_conf(), get_nprocs());

          printf("Before Threads\n");
          for (int i = 0; i < workers; i++ ){
		 struct thread_args * targs = &wargs[i];
		 targs->name = astring;
                 pthread_attr_t * pttr = &targs->attr;
	  	 int s = pthread_attr_init(pttr);

		 void * sp;
                 if (s != 0)
          	     handle_error_en(s, "pthread_attr_init");

	         s = posix_memalign(&sp, sysconf(_SC_PAGESIZE), STK_SIZE);
	         if (s != 0)
                         handle_error_en(s, "posix_memalign");
  	          	 
	         s = pthread_attr_setstack(pttr, sp, STK_SIZE);
	         if (s != 0)
                     handle_error_en(s, "pthread_attr_setstack");

    		 printf("Preparing Thread %s stack at %p \n", wargs[i].name, sp);
		 pthread_create(&worker[i], pttr, myThreadFun, (void *)&wargs[i]);
     
          }
          for (int i = 0; i < workers; i++ ){
                 pthread_join(worker[i],NULL);
     
          }
          printf("After Threads\n");
     
     }

     shmdt((void *) frame->box);
     shmctl(frame->id, IPC_RMID, NULL);

     exit(ERRVAL);
}


/* begin-procedure-description
---
**client** connects to the server via shared memory and ipc
  end-procedure-description */
void  client(int  argc, char *argv[], struct MailFrame * frame)
{
     int ERRVAL = 0;

     if (argc != 2) {
          printf("Use: %s number\n", argv[0]);
          ERRVAL = 1;
     }else{

          printf("Establishing connection.\n");
          while (frame->box->condition < PREPARED)
               ;
          printf("   Message is %s \n",  frame->box->data);
     
//          int PidFD = 0;
//          int TargetFD = 0;
//          int TheFD = 0;
     
//          if (frame->box->corepid != 0){
//              if (frame->box->corefd != 0){
//                  TheFD = syscall(__NR_pidfd_getfd, PidFD, TargetFD, 0);
//              }
//          }
     }
     frame->box->condition = atoi(argv[1]);
     shmdt((void *) frame->box);
     
     exit(ERRVAL);
  
}

/* begin-procedure-description
---
**setup** prepares the mailbox
  end-procedure-description */
void setup(int  argc, char *argv[], struct passwd *p, bool * first, struct MailFrame *frame)
{
     struct stat st = {0};
     const char *homedir;
     char thonkdir[256];
     key_t MailKey;
     
     
     
     printf("User Name: %s\n", p->pw_name);     
     if ((homedir = getenv("HOME")) == NULL) {
         homedir = p->pw_dir;
     }

     sprintf( thonkdir, "%s/.thonk", homedir);
     printf("Thonk Directory: %s\n", thonkdir);     

     if (stat(thonkdir, &st) == -1) {
         mkdir(thonkdir, 0700);
     }

     MailKey = ftok(thonkdir, SHKVAL);

     frame->id = shmget(MailKey, sizeof(struct Mailbox), 0666);
     if (frame->id < 0) {
          printf("first\n");
          *first = true;	  
     
          frame->id = shmget(MailKey, sizeof(struct Mailbox), IPC_CREAT | 0666);
          if (frame->id < 0) {
               printf("ERROR: Unable to establish shared memory\n");
               exit(1);
          }
     }else{
          printf("not first\n");
     }
     
     frame->box = (struct Mailbox *) shmat(frame->id, NULL, 0);
     if ((int64_t) frame->box == -1) {
          printf("ERROR: Unable to establish mailbox\n");
          exit(1);
     }

}

  /* begin-procedure-description
---
**main** begins...
  end-procedure-description */
int  main(int  argc, char *argv[])
{
     struct MailFrame frame;
     bool first = false;
     

     struct passwd *p = getpwuid(getuid());
     if (p != 0){

          setup(argc,argv,p,&first,&frame);
	     
	  if (first) {
		server(argc,argv,&frame);

	  }else{
		client(argc,argv,&frame);
	  }
     }
}
