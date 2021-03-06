# ====================================================================
#   "Mixed Finite Element Methods for Coupled 3D/1D Fluid Problems"
#      Course on Advanced Programming for Scientific Computing
#                     Politecnico di Milano
#                         A.Y. 2014-2015
#
#                    Copyright D. Notaro 2015
# ====================================================================
#   FILE        : Makefile
#   DESCRIPTION : makefile for test simulations
#   AUTHOR      : Domenico Notaro <domenico.not@gmail.com>
#   DATE        : April 2015
# ====================================================================

CPPFLAGS=-I../../include -I$(mkGetfemInc) -I$(mkBoostInc) 
CXXFLAGS+=-std=c++11 
# -D=M3D1D_VERBOSE_

CXXFLAGS += -I ${SAMG}/
CXXFLAGS+= -DSAMG_UNIX_LINUX -DSAMG_LCASE_USCORE -DPYRAMID_TRIANGULAR_FACETS

#DEBUG=yes

ifeq ($(DEBUG),yes)
  OPTFLAGS=-g -Wall
else
  OPTFLAGS=-O3 -march=native
  CPPFLAGS+=-DNDEBUG
endif
LDFLAGS=-L../../lib -L$(mkGetfemLib) -L$(mkBoostLib) -L$(mkLapackLib) -L$(mkQhullLib)
LDFLAGS += -L/opt/lib/samg/
LIBRARIES=-lproblem3d1d -lgetfem -lutil -lboost_iostreams -lboost_system -lboost_filesystem
LIBRARIES += -lamg -liomp5 -lblas

SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)
EXEC=M3D1D

OUTDIR=vtk

.PHONY: all clean distclean

all: $(EXEC)
	@echo
	@echo Compilation completed!

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OPTFLAGS) -o $@ -c $<

$(EXEC): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBRARIES)

clean:
	$(RM) $(OBJS) $(EXEC) *~ *.log

distclean: clean
	$(RM) *.txt $(OUTDIR)/*
