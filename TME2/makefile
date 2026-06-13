CC = gcc

CFLAGS = -I$(HOME)/fairthreads/include/ft_v1.1
LDFLAGS = -L$(HOME)/fairthreads/lib/ft_v1.1 -lfthread -lpthread

TARGET = tme2
SRC = tme2.c

all:
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

