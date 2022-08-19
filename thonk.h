#define  SHKVAL 42
#define  UNPREPARED  -1
#define  PREPARED     0
#define  READ1      1
#define  READ2      2
#define  READ3      3


struct Mailbox {
	int  condition;
	char  data[64];
};
