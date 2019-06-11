CXXFLAGS += -std=c++11
#LDFLAGS += -lrt
bmp_test_obj=bmp_test.o bmp.o
bmp_info_obj=bmp_info.o bmp.o
h2bmp_obj=h2bmp.o bmp.o

all:

clean:
	@rm -f *~ a.out *.o copy.bmp crop.bmp
	@find . -type f -perm -a+x -exec rm -f {} \;

timer_create: timer_create.c
	gcc -o $@ $< -lrt

bmp_test: $(bmp_test_obj)
	gcc -o $@ $+

bmp_info: $(bmp_info_obj)
	gcc -o $@ $+

h2bmp: $(h2bmp_obj)
	gcc -o $@ $+
