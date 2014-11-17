
CPP	=	g++ -O3  -std=c++14 -fexceptions -fpermissive
INCPATH  =  /usr/local/inc
LIBPATH =  /usr/local/inc
SYSLIBS = 	-lstdc++ -lboost_regex -lboost_thread -lboost_system -lrt  -lpthread
OCCLIBS =    -lTKMath -lTKXSBase -lTKernel -ldl -lTKIGES -lTKSTEP -lTKBRep -lTKMesh
PRGS	=	occd

all: le_build occd


occd: le_build/occd.o
	$(CPP) -fexceptions -D_BOOL  $^ -I$(INCPATH) -L$(LIBPATH) $(SYSLIBS) $(OCCLIBS) -o $@

clean:
	rm -rf le_build riftd;\

le_build/occd.o: occd.cpp blargh.h occd.h
	$(CPP) -fexceptions -D_BOOL -c $< -I$(INCPATH) -L$(LIBPATH)  -DHAVE_IOMANIP -DHAVE_IOSTREAM -DHAVE_LIMITS_H -o $@

le_build:
	mkdir ./le_build












