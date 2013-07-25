TARGETS	= todo.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W -fPIC `pkg-config --cflags libDesktop Mailer`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libDesktop`
AR	= ar
RANLIB	= ranlib
CCSHARED= $(CC) -shared
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

todo_OBJS = todo.o
todo_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
todo_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) ../src/task.o ../src/taskedit.o

todo.so: $(todo_OBJS)
	$(CCSHARED) -o todo.so $(todo_OBJS) $(todo_LDFLAGS)

todo.o: todo.c ../src/todo.c
	$(CC) $(todo_CFLAGS) -c todo.c

clean:
	$(RM) -- $(todo_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)/Mailer/plugins
	$(INSTALL) -m 0644 todo.so $(DESTDIR)$(LIBDIR)/Mailer/plugins/todo.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/Mailer/plugins/todo.so

.PHONY: all clean distclean install uninstall
