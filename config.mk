
UNAME:=$(shell uname -s)

ifeq ($(UNAME),SunOS)
        ifeq ($(CC),cc)
                CFLAGS?=-O
        else
                CFLAGS?=-Wall -ggdb -O2
        endif
else
        CFLAGS?=-Wall -ggdb -O2 -Wconversion
endif

