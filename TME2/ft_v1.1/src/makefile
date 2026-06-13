include ../config

##############################################################
LIBS		= -lfthread -lpthread
#LIBS		= -lposix4 -lfthread -lpthread # for solaris

##############################################################
CFLAGS = -Wall -O3 -D_REENTRANT -fcommon


############ Basic libraries #################################
LIB_NAME	= fthread
LIBFT         	= lib$(LIB_NAME).a

############ C code to executable code ########################
.c.o :
	$(C_COMPILER) $(CFLAGS) -c $<

############ The h files ######################################
HFILES	      = fthread.h fthread_internal.h 	\
                event.h thread.h scheduler.h	\
                broadcast_order.h order.h	\
                trace.h verify.h itemlist.h

############ The C files ######################################
CFILES	      = event.c thread.c scheduler.c 	\
                broadcast_order.c order.c	\
                automaton.c instruction.c 	\
                itemlist.c error.c

############ The object files ##################################
OBJS	      = event.o	thread.o scheduler.o 	\
                broadcast_order.o order.o 	\
                automaton.o instruction.o 	\
                itemlist.o error.o

############ Make entries ######################################
all: install

lib: $(HFILE) $(OBJS)
	$(AR) r $(LIBFT) $(OBJS)
	- $(RANLIB) $(LIBFT)

install: lib
	cp $(LIBFT) $(FT_LIBDIR)
	cp fthread.h $(FT_INCLDIR)

uninstall: 
	rm -f $(FT_LIBDIR)/$(LIBFT)
	rm -f $(FT_INCLDIR)/fthread.h

clean:
	- rm -rf *.o *~ $(LIBFT) a.out

############ Auxiliary ########################################
TEST		= test.c

lines:
	wc -l $(CFILES) $(HFILES)

test:all
	$(C_COMPILER) $(CFLAGS) -I$(FT_INCLDIR) -L$(FT_LIBDIR) $(TEST) $(LIBS)
	a.out

############ Dependencies ########################################
$(OBJS): $(HFILE)

############ end of makefile #####################################

