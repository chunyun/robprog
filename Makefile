CC = /opt/centos/devtoolset-1.1/root/usr/bin/g++ 
CCINC = -I ./include/ 

DEFINES = #-DDEBUG
CCFLAGS = -std=c++11 -Wall $(DEFINES)
CCProfFLAGS = -ggdb -pg
CCLIBS =

SRCDIR = ./src
OBJDIR = ./obj
CPPSRC = etaExample.cpp
CPPOBJ = $(CPPSRC:%.cpp=$(OBJDIR)/%.o)

BINDIR = ./bin
EXE = $(BINDIR)/testETAProgressBar.exe


all: $(CPPOBJ) $(EXE)

$(EXE): $(CPPOBJ) 	
	@mkdir -p $(BINDIR)
	$(CC) $(CPPOBJ) -o $@ $(CCINC) $(CCFLAGS) $(CCLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp 
	@mkdir -p $(OBJDIR)
	$(CC) -c $< -o $@ $(CCINC) $(CCFLAGS)

clean:
	rm -rf $(OBJ)

cleanall: 
	rm -rf $(BINDIR) $(OBJDIR)

