TARGETS	= $(OBJDIR)todo
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
SBINDIR	= $(PREFIX)/sbin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libDesktop`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libDesktop` -lintl
LDFLAGS	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

todo_OBJS = $(OBJDIR)task.o $(OBJDIR)taskedit.o $(OBJDIR)todo.o $(OBJDIR)window.o $(OBJDIR)main.o
todo_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
todo_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

$(OBJDIR)todo: $(todo_OBJS)
	$(CC) -o $(OBJDIR)todo $(todo_OBJS) $(todo_LDFLAGS)

$(OBJDIR)task.o: task.c task.h
	$(CC) $(todo_CFLAGS) -fPIC -o $(OBJDIR)task.o -c task.c

$(OBJDIR)taskedit.o: taskedit.c
	$(CC) $(todo_CFLAGS) -fPIC -o $(OBJDIR)taskedit.o -c taskedit.c

$(OBJDIR)todo.o: todo.c task.h todo.h ../config.h
	$(CC) $(todo_CFLAGS) -fPIC -o $(OBJDIR)todo.o -c todo.c

$(OBJDIR)window.o: window.c todo.h window.h
	$(CC) $(todo_CFLAGS) -o $(OBJDIR)window.o -c window.c

$(OBJDIR)main.o: main.c task.h todo.h ../config.h
	$(CC) $(todo_CFLAGS) -o $(OBJDIR)main.o -c main.c

clean:
	$(RM) -- $(todo_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $(OBJDIR)todo $(DESTDIR)$(BINDIR)/todo

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/todo

.PHONY: all clean distclean install uninstall
