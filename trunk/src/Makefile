DEBUG=1

OPENCV_COIN=0

include Makefile.local

MAINOBJ=main.o
APP=cluster_editing

OBJS=	$(MAINOBJ)				\
		fmem.o					\
		datasource.o			\
		sched.o					\
		strategy_depth_first.o	\
		strategy_best_first.o	\
		graphstate.o			\
		graph.o					\
		graphfile.o				\
		alg_3k.o				\
		alg_2k.o				\
		alg_2_62k.o				\
		gen.o					\
		visual.o				\
		splitting.o				\
		postprocess.o			\
		datasource_random.o		\
		kernel.o				\
		datasource_file.o

INCL=-I.

CFLAGS_CSTD=-Wall -std=c89 -pedantic -O0
CFLAGS_NORMAL=-g $(INCL)

ifneq ($(DEBUG),0)
  CFLAGS_NORMAL+=-DDEBUG=$(DEBUG)
  OBJS+=debug.o
endif

LDFLAGS=-lm

ifeq ($(OPENCV_COIN),1)
  OBJS+=datasource_cv_coin.o
  OBJS+=datasource_cv_coin_anim.o
  OBJS+=cv_coin.o
  CFLAGS_NORMAL+=-DOPENCV_COIN

  CV_CFLAGS=$(CFLAGS_NORMAL) -I/usr/include/opencv
  LDFLAGS+=-lcv -lhighgui -lcvaux
endif

CC=gcc

CFLAGS=$(CFLAGS_NORMAL) $(CFLAGS_CSTD)

all: app
#	@echo "***************"
#	@echo "* To make tests, type: $(MAKE) tests"
#	@echo "***************"

app: $(APP)

$(APP): $(OBJS)
	gcc -o $(APP) $(LDFLAGS) $(OBJS)

clean: tests_clean
	rm -f $(APP) $(OBJS)

tests:
	for F in tests/*.c; do make app MAINOBJ=$${F/.c/.o} APP=$${F/.c/}; done

tests_clean:
	for F in tests/*.c; do rm -f $${F/.c/} $${F/.c/.o}; done

datasource_cv_coin.o: datasource_cv_coin.c
	$(CC) $(CV_CFLAGS) -c -o $@ $<

datasource_cv_coin_anim.o: datasource_cv_coin_anim.c
	$(CC) $(CV_CFLAGS) -c -o $@ $<

cv_coin.o: cv_coin.c
	$(CC) $(CV_CFLAGS) -c -o $@ $<

.PHONY: all app tests tests_clean
