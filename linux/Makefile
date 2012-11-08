INCS    =       -I../include -I../../../../include -I../../base102/include
LIB	= ../../../../lib
LIBS    =       -L$(LIB) -lsys_utl
CC	= arm-linux-g++
CC-host	= g++
NAME    = sd102
DebugName = sd102-debug
SOURCE  =       ../src/sd102.cpp
debug_SOURCE = ../src/main.cpp
wflag = -Wall -Wextra
fdebug = -g
all:$(NAME)
$(NAME):$(SOURCE)
	$(CC) $(LDFALGS) $(CXXFLAGS) -shared $(MARCO) \
	$(INCS) $^ -o $(LIB)/lib$@.so $(LIBS) $(fdebug) $(wflag)

debug:$(DebugName)
$(DebugName):$(debug_SOURCE)
	$(CC-host) $(LDFALGS) $(CXXFLAGS)  $(MARCO) $(INCS) $^ -o $@ -L$(LIB) \
	$(fdebug) $(wflag)
