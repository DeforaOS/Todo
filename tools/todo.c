/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Todo */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <stdlib.h>
#include <Desktop/Mailer/plugin.h>
#include "../src/todo.h"


/* Mailing-lists */
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

	if((todo = malloc(sizeof(*todo))) == NULL)
		return NULL;
	if((todo->todo = todo_new(NULL, NULL)) == NULL)
	{
		_todo_destroy(todo);
		return NULL;
	}
	todo->helper = helper;
	todo->widget = gtk_vbox_new(FALSE, 4);
	widget = todo_get_widget(todo->todo);
	gtk_box_pack_start(GTK_BOX(todo->widget), widget, TRUE, TRUE, 0);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	todo->view = gtk_tree_view_new();
	gtk_container_add(GTK_CONTAINER(widget), todo->view);
	gtk_box_pack_start(GTK_BOX(todo->widget), widget, TRUE, TRUE, 0);
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
