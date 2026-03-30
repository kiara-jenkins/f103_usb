CC            = gcc
LIB           = ar -ru
CFLAGS        = -O2 -Wall
INCPATH       = -I"pm_common" -I"porttime" -I"pm_win"

OBJECTS_DIR   = 

SOURCES       = pm_win\pmwinmm.c \
		pm_win\pmwin.c \
		pm_common\portmidi.c \
		pm_common\pmutil.c \
		porttime\porttime.c \
		porttime\ptwinmm.c 
OBJECTS       = pmwinmm.o \
		pmwin.o \
		portmidi.o \
		pmutil.o \
		porttime.o \
		ptwinmm.o

TARGET         = libportmidi.a

####### Implicit rules

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

####### Build rules

all:    $(OBJECTS) 
	$(LIB) $(TARGET) $(OBJECTS) 


####### Compile

pmwinmm.o: pm_win/pmwinmm.c pm_win/pmwinmm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o pmwinmm.o pm_win\pmwinmm.c

pmwin.o: pm_win/pmwin.c pm_win/pmwinmm.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o pmwin.o pm_win\pmwin.c

portmidi.o: pm_common/portmidi.c pm_common/portmidi.h \
		pm_common/pmutil.h \
		pm_common/pminternal.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o portmidi.o pm_common\portmidi.c

pmutil.o: pm_common/pmutil.c pm_common/portmidi.h \
		pm_common/pmutil.h \
		pm_common/pminternal.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o pmutil.o pm_common\pmutil.c

porttime.o: porttime/porttime.c 
	$(CC) -c $(CFLAGS) $(INCPATH) -o porttime.o porttime\porttime.c

ptwinmm.o: porttime/ptwinmm.c porttime/porttime.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o ptwinmm.o porttime\ptwinmm.c
