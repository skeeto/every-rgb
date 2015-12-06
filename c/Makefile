CFLAGS  = -std=c11 -Wall -Wextra -pedantic -g3 -Ofast
LDLIBS  = -lm

obj = color.o octree.o image.o rand.o colorset.o naive.o kdtree.o

color : $(obj)

clean :
	$(RM) color $(obj)

color.o: color.c octree.h color.h finder.h naive.h image.h rand.h colorset.h
colorset.o: colorset.c colorset.h color.h rand.h
image.o: image.c image.h color.h
kdtree.o: kdtree.c kdtree.h finder.h color.h
naive.o: naive.c naive.h finder.h color.h
octree.o: octree.c octree.h color.h finder.h
rand.o: rand.c rand.h
