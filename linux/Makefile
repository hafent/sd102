INCS    = -I../include -I../../../../include -I../../base102/include
#普通版本头文件包含 (非256块表的)
#INCS    += -I/home/lee/test/hl3104/trunk/hl3104jd/软件/hl3104/include
#INCS    += -I/home/lee/test/hl3104/trunk/hl3104jd/软件/hl3104/src/protocollib/base102/include
#INCS    += -I../include

LIB	= ../../../../lib
#普通版本库
#LIB += /home/lee/test/hl3104/trunk/hl3104jd/软件/hl3104/lib

LIBS    = -L$(LIB) -lsys_utl -lbase102
CXX	= arm-linux-g++
hostcxx	= g++
NAME    = sd102
DebugName = sd102-debug
SOURCE  =       ../src/sd102.cpp
wflag = -Wall -Wextra -Wfloat-equal -Wshadow -Wno-conversion    \
	-Wmissing-declarations -Wundef -Wuninitialized -Wcast-align \
	-Wsign-compare -Wcast-qual -Wwrite-strings -Waggregate-return \
	-Woverloaded-virtual -Wfatal-errors
	
#wflag = -Wall -Wextra
CXXFLAGS = #-std=gnu99

fdebug = -g -O
all:$(NAME)
$(NAME):$(SOURCE) 
#	arm-linux-gcc ../src/type.c -c -o type.o $(INCS)
	$(CXX) $(LDFALGS) $(CXXFLAGS) -shared $(MARCO) \
	$(INCS) $^ type.o -o $(LIB)/lib$@.so $(LIBS) \
	$(fdebug) $(wflag)

debug:a
a:$(SOURCE)
	$(hostcxx) $(LDFALGS) $(CXXFLAGS) -shared $(MARCO) \
	$(INCS) $^ -o $(LIB)/lib$@.so $(LIBS) \
	$(fdebug) $(wflag)