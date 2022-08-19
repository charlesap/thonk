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

#include  "thonk.h"

void  main(int  argc, char *argv[])
{
     bool first = false;
     key_t          MailKey;
     int            MailID;
     struct Mailbox  *MailPtr;
     struct stat st = {0};
     const char *homedir;
     char thonkdir[256];

     struct passwd *p = getpwuid(getuid());  // Check for NULL!

     

     if (p != 0){
          printf("User Name: %s\n", p->pw_name);     
          if ((homedir = getenv("HOME")) == NULL) {
              homedir = p->pw_dir;
          }
          sprintf( thonkdir, "%s/.thonk", homedir);
          printf("Thonk Directory: %s\n", thonkdir);     

          if (stat(thonkdir, &st) == -1) {
	      mkdir(thonkdir, 0700);
          }

          if (argc != 2) {
               printf("Use: %s <\"message\"|number>\n", argv[0]);
               exit(1);
          }

          MailKey = ftok(thonkdir, SHKVAL);

          MailID = shmget(MailKey, sizeof(struct Mailbox), 0666);
          if (MailID < 0) {
	       first = true;	  
          
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
          
	  if (first) {

               MailPtr->condition  = UNPREPARED;
               strncpy(MailPtr->data, argv[1], 63);
               MailPtr->data[63] = '\0';
               MailPtr->condition = PREPARED;
               printf("Ready for connections.\n");
                           
               while (MailPtr->condition < READ3)
                    sleep(1);
                    
               shmdt((void *) MailPtr);
               shmctl(MailID, IPC_RMID, NULL);
               exit(0);
	  }else{
               while (MailPtr->condition < PREPARED)
                    ;
               printf("   Message is %s \n",  MailPtr->data);

               MailPtr->condition = atoi(argv[1]);
               shmdt((void *) MailPtr);
               exit(0);

	  }
     }
}
