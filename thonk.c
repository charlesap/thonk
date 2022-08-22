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

struct Mailbox * MailPtr;
int MailID;

/* begin-section-description
## ---------- Utility Functions
  end-section-description */


  /* begin-procedure-description
---
**myThreadFun** makes a thread.
  end-procedure-description */
void *myThreadFun(void *vargp)
{
    sleep(1);
    printf("In Thread \n");
    return NULL;
}

/* begin-section-description
## ---------- Initialization
  end-section-description */


/* begin-procedure-description
---
**server** accepts client connections via shared memory and ipc
  end-procedure-description */
void  server(int  argc, char *argv[])
{
     pthread_t worker[MAXTHREADS];
     int workers = 0;
     int ERRVAL = 0;
     
     if (argc != 2) {
          printf("Use: %s \"message\"\n", argv[0]);
	  ERRVAL = 1;
     }else{

          MailPtr->condition  = UNPREPARED;
          MailPtr->corepid  = 0;
          MailPtr->corefd  = 0;
          strncpy(MailPtr->data, argv[1], 63);
          MailPtr->data[63] = '\0';
          MailPtr->condition = PREPARED;
          printf("Ready for connections.\n");
                      
          while (MailPtr->condition < READ3)
               sleep(1);
               
     
          workers = get_nprocs();
          if (workers > MAXTHREADS) workers = MAXTHREADS;
     
          printf("This system has %d processors configured and "
                 "%d processors available.\n",
                 get_nprocs_conf(), get_nprocs());
     
          printf("Before Threads\n");
          for (int i = 0; i < workers; i++ ){
                 pthread_create(&worker[i], NULL, myThreadFun, NULL);
     
          }
          for (int i = 0; i < workers; i++ ){
                 pthread_join(worker[i],NULL);
     
          }
          //pthread_t thread_id;
          //pthread_create(&thread_id, NULL, myThreadFun, NULL);
          //pthread_join(thread_id, NULL);
          printf("After Threads\n");
     
     }
     shmdt((void *) MailPtr);
     shmctl(MailID, IPC_RMID, NULL);
     exit(ERRVAL);
}


/* begin-procedure-description
---
**client** connects to the server via shared memory and ipc
  end-procedure-description */
void  client(int  argc, char *argv[])
{
     int ERRVAL = 0;

     if (argc != 2) {
          printf("Use: %s number\n", argv[0]);
          ERRVAL = 1;
     }else{

          printf("Establishing connection.\n");
          while (MailPtr->condition < PREPARED)
               ;
          printf("   Message is %s \n",  MailPtr->data);
     
          int PidFD = 0;
          int TargetFD = 0;
          int TheFD = 0;
     
          if (MailPtr->corepid != 0){
              if (MailPtr->corefd != 0){
                  TheFD = syscall(__NR_pidfd_getfd, PidFD, TargetFD, 0);
              }
          }
     }
     MailPtr->condition = atoi(argv[1]);
     shmdt((void *) MailPtr);
     
     exit(ERRVAL);
  
}

/* begin-procedure-description
---
**setup** prepares the mailbox
  end-procedure-description */
void setup(int  argc, char *argv[], struct passwd *p, bool * first)
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

     MailID = shmget(MailKey, sizeof(struct Mailbox), 0666);
     if (MailID < 0) {
          *first = true;	  
     
          MailID = shmget(MailKey, sizeof(struct Mailbox), IPC_CREAT | 0666);
          if (MailID < 0) {
               printf("ERROR: Unable to establish shared memory\n");
               exit(1);
          }
     }
     
     MailPtr = (struct Mailbox *) shmat(MailID, NULL, 0);
     if ((int64_t) MailPtr == -1) {
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
     bool first = false;

     struct passwd *p = getpwuid(getuid());
     if (p != 0){

          setup(argc,argv,p,&first);
	     
	  if (first) {
		server(argc,argv);

	  }else{
		client(argc,argv);
	  }
     }
}
