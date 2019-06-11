CXXFLAGS += -std=c++11
#LDFLAGS += -lrt
h2bin_obj=h2bin.o bmp.o

all:

clean:
	@rm -f *~ a.out *.o
	@find . -type f -perm -a+x -exec rm -f {} \;

timer_create: timer_create.c
	gcc -o $@ $< -lrt

h2bin: $(h2bin_obj)
	gcc -o $@ $(h2bin_obj)