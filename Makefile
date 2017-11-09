CXXFLAGS += -std=c++11
#LDFLAGS += -lrt

all:

clean:
	@rm -f *~ a.out *.o
	@find . -type f -perm -a+x -exec rm -f {} \;

timer_create: timer_create.c
	gcc -o $@ $< -lrt
