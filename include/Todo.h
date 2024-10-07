/* $Id$ */
/* Copyright (c) 2009-2024 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Todo */
/* All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */



#ifndef DESKTOP_TODO_H
# define DESKTOP_TODO_H

# include <gtk/gtk.h>


/* Todo */
/* types */
typedef struct _Todo Todo;

typedef enum _TodoColumn
{
	TD_COL_TASK,
	TD_COL_DONE,
	TD_COL_TITLE,
	TD_COL_START,
	TD_COL_DISPLAY_START,
	TD_COL_END,
	TD_COL_DISPLAY_END,
	TD_COL_PRIORITY,
	TD_COL_DISPLAY_PRIORITY,
	TD_COL_CATEGORY
} TodoColumn;
#define TD_COL_LAST TD_COL_CATEGORY
#define TD_COL_COUNT (TD_COL_LAST + 1)

typedef enum _TodoFilter
{
	TODO_FILTER_ALL_TASKS = 0,
	TODO_FILTER_COMPLETED_TASKS,
	TODO_FILTER_REMAINING_TASKS
} TodoFilter;
# define TODO_FILTER_LAST TODO_FILTER_REMAINING_TASKS
# define TODO_FILTER_COUNT (TODO_FILTER_LAST + 1)

typedef enum _TodoPriority
{
	TODO_PRIORITY_UNKNOWN = 0,
	TODO_PRIORITY_LOW,
	TODO_PRIORITY_MEDIUM,
	TODO_PRIORITY_HIGH,
	TODO_PRIORITY_URGENT
} TodoPriority;


/* functions */
Todo * todo_new(GtkWidget * window, GtkAccelGroup * group);
void todo_delete(Todo * todo);

/* accessors */
unsigned int todo_get_filter(Todo * todo);
GtkWidget * todo_get_view(Todo * todo);
GtkTreeViewColumn * todo_get_view_column(Todo * todo, unsigned i);
GtkWidget * todo_get_widget(Todo * todo);

void todo_set_filter(Todo * todo, unsigned int filter);

/* useful */
void todo_about(Todo * todo);
int todo_error(Todo * todo, char const * message, int ret);

void todo_show_preferences(Todo * todo, gboolean show);

#endif /* !DESKTOP_TODO_H */
