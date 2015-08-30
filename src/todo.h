/* $Id$ */
/* Copyright (c) 2009-2015 Pierre Pronchery <khorben@defora.org> */
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



#ifndef TODO_TODO_H
# define TODO_TODO_H

# include "task.h"
# include <gtk/gtk.h>


/* Todo */
/* types */
typedef struct _Todo Todo;

typedef enum _TodoPriority
{
	TODO_PRIORITY_UNKNOWN,
	TODO_PRIORITY_LOW,
	TODO_PRIORITY_MEDIUM,
	TODO_PRIORITY_HIGH,
	TODO_PRIORITY_URGENT
} TodoPriority;

typedef enum _TodoView
{
	TODO_VIEW_ALL_TASKS = 0,
	TODO_VIEW_COMPLETED_TASKS,
	TODO_VIEW_REMAINING_TASKS
} TodoView;
# define TODO_VIEW_LAST TODO_VIEW_REMAINING_TASKS
# define TODO_VIEW_COUNT (TODO_VIEW_LAST + 1)


/* functions */
Todo * todo_new(GtkWidget * window, GtkAccelGroup * group);
void todo_delete(Todo * todo);

/* accessors */
TodoView todo_get_view(Todo * todo);
GtkWidget * todo_get_widget(Todo * todo);
void todo_set_view(Todo * todo, TodoView view);

/* useful */
void todo_about(Todo * todo);
int todo_error(Todo * todo, char const * message, int ret);

void todo_show_preferences(Todo * todo, gboolean show);

/* tasks */
Task * todo_task_add(Todo * todo, Task * task);
void todo_task_delete_selected(Todo * todo);
void todo_task_remove_all(Todo * todo);

/* accessors */
void todo_task_set_priority(Todo * todo, GtkTreePath * path,
		char const * priority);
void todo_task_set_title(Todo * todo, GtkTreePath * path, char const * title);

void todo_task_cursor_changed(Todo * todo);
void todo_task_edit(Todo * todo);
int todo_task_reload_all(Todo * todo);
void todo_task_save_all(Todo * todo);
void todo_task_select_all(Todo * todo);
void todo_task_toggle_done(Todo * todo, GtkTreePath * path);

#endif /* !TODO_TODO_H */
