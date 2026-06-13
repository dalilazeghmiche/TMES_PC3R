include ../config

####################################################
CC	= gcc -Wall -O3 -D_REENTRANT -I$(FT_INCLDIR) -L$(FT_LIBDIR)

LIBS	= -lfthread -lpthread
#LIBS	= -posix4 -lfthread -lpthread # for solaris

####################################################
all:
	aux/all "$(CC)" "$(LIBS)"

####################################################
clean:
	rm -f *~ *.o a.out

####################################################