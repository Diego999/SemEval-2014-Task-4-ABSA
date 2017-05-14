#Makefile for sst-light

CXX = g++
CFLAGS = -g -O1 -DPIC -fPIC -ILIB -I.

CXXFLAGS = $(CFLAGS) -fno-exceptions
#CXXFLAGS = -g
#LIB = -lm ${TAO_LIB} ${PETSC_SNES_LIB}
#include ${TAO_DIR}/bmake/tao_common

SOURCES = sst-light.cc \
	LIB/utils/utils.cc \
	LIB/stats/stats.cc \
	LIB/results/results.cc \
	LIB/evaluate/evaluate.cc \
	LIB/Chain/Chain.cc  \
	LIB/Perceptron/Perceptron.cc  \
	LIB/tagger_light/Tlight.cc \
	LIB/examples/examples.cc \
	LIB/features/features.cc

MLIBOBJECTS = LIB/utils/utils.o \
        LIB/stats/stats.o \
        LIB/results/results.o \
        LIB/evaluate/evaluate.o \
        LIB/Chain/Chain.o  \
        LIB/Perceptron/Perceptron.o  \
        LIB/tagger_light/Tlight.o \
        LIB/examples/examples.o \
	LIB/features/features.o 


OBJECTS = sst-light.o \
	LIB/utils/utils.o \
	LIB/stats/stats.o \
	LIB/results/results.o \
	LIB/evaluate/evaluate.o \
	LIB/Chain/Chain.o  \
	LIB/Perceptron/Perceptron.o  \
	LIB/tagger_light/Tlight.o \
	LIB/examples/examples.o \
	LIB/features/features.o #\
#	/usr/local/WordNet-2.1/lib/libWN.a

PEROBJ  = Perceptron/Perceptron.o  Perceptron/Kernel_Perceptron.o
CRFOBJ  = CRF/CRF.o

SuperSenseTagger: $(OBJECTS) 
	g++ $(OBJECTS) -o sst

videotagger: $(MLIBOBJECTS) videotagger.o
	g++ $(MLIBOBJECTS) videotagger.o -o videotagger

to-conll: to-conll.o
	g++ to-conll.o -o to-conll

libwnss.so: 	$(MLIBOBJECTS)
	g++ -shared -o libwnss.so  $(MLIBOBJECTS) -lm -lpthread -lc -lstdc++ -lgcc_s

.PHONY: clean

clean:
	find . -name '*.[od]' -print -exec rm {} \;
clean_bak:
	find . -name '*~' -print -exec rm {} \;

# this command tells GNU make to look for dependencies in *.d files
-include $(patsubst %.c,%.d,$(SOURCES:%.cc=%.d))
