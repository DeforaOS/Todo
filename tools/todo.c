/* $Id$ */
/* Copyright (c) 2012-2015 Pierre Pronchery <khorben@defora.org> */
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



#ifndef EMBEDDED
# define EMBEDDED
#endif
#include <stdlib.h>
#include <Desktop/Mailer/plugin.h>

#include "../src/priority.c"
#include "../src/task.c"
#include "../src/taskedit.c"
#include "../src/todo.c"


/* Todo */
/* private */
/* types */
typedef struct _MailerPlugin TodoPlugin;

struct _MailerPlugin
{
	MailerPluginHelper * helper;

	Todo * todo;

	/* widgets */
	GtkWidget * widget;
	GtkWidget * view;
};


/* protected */
/* prototypes */
/* plug-in */
static MailerPlugin * _todo_init(MailerPluginHelper * helper);
static void _todo_destroy(TodoPlugin * todo);
static GtkWidget * _todo_get_widget(TodoPlugin * todo);


/* public */
/* variables */
/* plug-in */
MailerPluginDefinition plugin =
{
	"Todo",
	"todo",
	NULL,
	_todo_init,
	_todo_destroy,
	_todo_get_widget,
	NULL
};


/* protected */
/* functions */
/* plug-in */
/* todo_init */
static MailerPlugin * _todo_init(MailerPluginHelper * helper)
{
	TodoPlugin * todo;
	GtkWidget * widget;
	size_t i;

	if((todo = malloc(sizeof(*todo))) == NULL)
		return NULL;
	if((todo->todo = todo_new(NULL, NULL)) == NULL)
	{
		_todo_destroy(todo);
		return NULL;
	}
	todo->helper = helper;
	todo->widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	widget = todo_get_widget(todo->todo);
	gtk_box_pack_start(GTK_BOX(todo->widget), widget, TRUE, TRUE, 0);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(todo->todo->view),
			FALSE);
	for(i = 0; i < TD_COL_COUNT; i++)
		if(todo->todo->columns[i] != NULL && i != TD_COL_TITLE)
			gtk_tree_view_column_set_visible(todo->todo->columns[i],
					FALSE);
	gtk_widget_show_all(todo->widget);
	return todo;
}


/* todo_destroy */
static void _todo_destroy(TodoPlugin * todo)
{
	if(todo->todo != NULL)
		todo_delete(todo->todo);
	free(todo);
}


/* todo_get_widget */
static GtkWidget * _todo_get_widget(TodoPlugin * todo)
{
	return todo->widget;
}
