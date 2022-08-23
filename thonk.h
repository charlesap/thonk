#define  SHKVAL 42
#define  UNPREPARED  -1
#define  PREPARED     0
#define  READ1      1
#define  READ2      2
#define  READ3      3
#define  MAXTHREADS 32
#define  THREAD_STACK_SIZE 16384

struct Mailbox {
	int  corepid;
	int  corefd;
	int  condition;
	char  data[64];
};

struct MailFrame {
	struct Mailbox * box;
	int id;
};
