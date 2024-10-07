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



#ifndef TODO_TODO_H
# define TODO_TODO_H

# include "../include/Todo.h"
# include "task.h"


/* Todo */
/* functions */
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
