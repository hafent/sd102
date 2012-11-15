INCS    = -I../include -I../../../../include -I../../base102/include
LIB	= ../../../../lib
LIBS    = -L$(LIB) -lsys_utl -lbase102
CC	= arm-linux-g++
NAME    = sd102
DebugName = sd102-debug
SOURCE  =       ../src/sd102.cpp
wflag = -Wall -Wextra -Wfloat-equal -Wshadow -Wno-conversion    \
	-Wmissing-declarations -Wundef -Wuninitialized -Wcast-align \
	-Wsign-compare -Wcast-qual -Wwrite-strings -Waggregate-return \
	-Woverloaded-virtual
fdebug = -g
all:$(NAME)
$(NAME):$(SOURCE)
	$(CC) $(LDFALGS) $(CXXFLAGS) -shared $(MARCO) \
	$(INCS) $^ -o $(LIB)/lib$@.so $(LIBS) \
	$(fdebug) $(wflag)

