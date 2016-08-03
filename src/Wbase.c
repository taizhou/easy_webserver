#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#define ERR_EXIT(m)\
				do\
				{\
								perror(m);\
								exit(EXIT_FAILURE);\
				}while(0)


