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



#include <stdlib.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "todo.h"
#include "window.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* TodoWindow */
/* private */
/* types */
struct _TodoWindow
{
	Todo * todo;

	/* widgets */
	GtkWidget * window;
	GtkWidget * statusbar;
};


/* prototypes */
/* callbacks */
static void _todowindow_on_close(gpointer data);
static gboolean _todowindow_on_closex(gpointer data);
static void _todowindow_on_edit(gpointer data);
static void _todowindow_on_new(gpointer data);
static void _todowindow_on_preferences(gpointer data);

#ifndef EMBEDDED
/* menus */
/* file menu */
static void _todowindow_on_file_new(gpointer data);
static void _todowindow_on_file_edit(gpointer data);
static void _todowindow_on_file_close(gpointer data);

/* edit menu */
static void _todowindow_on_edit_select_all(gpointer data);
static void _todowindow_on_edit_delete(gpointer data);
static void _todowindow_on_edit_preferences(gpointer data);

/* view menu */
static void _todowindow_on_view_all_tasks(gpointer data);
static void _todowindow_on_view_completed_tasks(gpointer data);
static void _todowindow_on_view_remaining_tasks(gpointer data);

/* help menu */
static void _todowindow_on_help_about(gpointer data);
#endif

/* constants */
/* accelerators */
static const DesktopAccel _todo_accel[] =
{
#ifdef EMBEDDED
	{ G_CALLBACK(_todowindow_on_close), GDK_CONTROL_MASK, GDK_KEY_W },
	{ G_CALLBACK(_todowindow_on_edit), GDK_CONTROL_MASK, GDK_KEY_E },
	{ G_CALLBACK(_todowindow_on_new), GDK_CONTROL_MASK, GDK_KEY_N },
	{ G_CALLBACK(_todowindow_on_preferences), GDK_CONTROL_MASK, GDK_KEY_P },
#endif
	{ NULL, 0, 0 }
};

#ifndef EMBEDDED
/* menubar */
static const DesktopMenu _file_menu[] =
{
	{ N_("_New"), G_CALLBACK(_todowindow_on_file_new), GTK_STOCK_NEW,
		GDK_CONTROL_MASK, GDK_KEY_N },
	{ N_("_Edit"), G_CALLBACK(_todowindow_on_file_edit), GTK_STOCK_EDIT,
		GDK_CONTROL_MASK, GDK_KEY_E },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Close"), G_CALLBACK(_todowindow_on_file_close), GTK_STOCK_CLOSE,
		GDK_CONTROL_MASK, GDK_KEY_W },
	{ NULL, NULL, NULL, 0, 0 }
};
static const DesktopMenu _edit_menu[] =
{
	{ N_("Select _All"), G_CALLBACK(_todowindow_on_edit_select_all),
#if GTK_CHECK_VERSION(2, 10, 0)
		GTK_STOCK_SELECT_ALL,
#else
		"edit-select-all",
#endif
		GDK_CONTROL_MASK, GDK_KEY_A },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Delete"), G_CALLBACK(_todowindow_on_edit_delete),
		GTK_STOCK_DELETE, 0, 0 },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Preferences"), G_CALLBACK(_todowindow_on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_CONTROL_MASK, GDK_KEY_P },
	{ NULL, NULL, NULL, 0, 0 }
};
static const DesktopMenu _view_menu[] =
{
	{ N_("_All tasks"), G_CALLBACK(_todowindow_on_view_all_tasks), NULL, 0,
		0 },
	{ N_("_Completed tasks"), G_CALLBACK(
			_todowindow_on_view_completed_tasks), NULL, 0, 0 },
	{ N_("_Remaining tasks"), G_CALLBACK(
			_todowindow_on_view_remaining_tasks), NULL, 0, 0 },
	{ NULL, NULL, NULL, 0, 0 }
};
static const DesktopMenu _help_menu[] =
{
	{ N_("_About"), G_CALLBACK(_todowindow_on_help_about),
#if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_ABOUT, 0, 0 },
#else
		NULL, 0, 0 },
#endif
	{ NULL, NULL, NULL, 0, 0 }
};
static const DesktopMenubar _menubar[] =
{
	{ N_("_File"), _file_menu },
	{ N_("_Edit"), _edit_menu },
	{ N_("_View"), _view_menu },
	{ N_("_Help"), _help_menu },
	{ NULL, NULL },
};
#endif


/* public */
/* functions */
/* todowindow_new */
TodoWindow * todowindow_new(void)
{
	TodoWindow * todo;
	GtkAccelGroup * group;
	GtkWidget * vbox;
	GtkWidget * widget;

	if((todo = malloc(sizeof(*todo))) == NULL)
		return NULL;
	todo->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	group = gtk_accel_group_new();
	todo->todo = todo_new(todo->window, group);
	/* check for errors */
	if(todo->todo == NULL)
	{
		todowindow_delete(todo);
		g_object_unref(group);
		return NULL;
	}
	desktop_accel_create(_todo_accel, todo, group);
	gtk_window_add_accel_group(GTK_WINDOW(todo->window), group);
	g_object_unref(group);
	gtk_window_set_default_size(GTK_WINDOW(todo->window), 640, 480);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(todo->window), "todo");
#endif
	gtk_window_set_title(GTK_WINDOW(todo->window), _("Todo"));
	g_signal_connect_swapped(todo->window, "delete-event", G_CALLBACK(
				_todowindow_on_closex), todo);
	vbox = gtk_vbox_new(FALSE, 0);
#ifndef EMBEDDED
	/* menubar */
	widget = desktop_menubar_create(_menubar, todo, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
#endif
	widget = todo_get_widget(todo->todo);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	/* statusbar */
	todo->statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), todo->statusbar, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(todo->window), vbox);
	gtk_widget_show_all(todo->window);
	return todo;
}


/* todowindow_delete */
void todowindow_delete(TodoWindow * todo)
{
	if(todo->todo != NULL)
		todo_delete(todo->todo);
	gtk_widget_destroy(todo->window);
	free(todo);
}


/* private */
/* functions */
/* callbacks */
/* todowindow_on_close */
static void _todowindow_on_close(gpointer data)
{
	TodoWindow * todo = data;

	_todowindow_on_closex(todo);
}


/* todowindow_on_closex */
static gboolean _todowindow_on_closex(gpointer data)
{
	TodoWindow * todo = data;

	gtk_widget_hide(todo->window);
	gtk_main_quit();
	return TRUE;
}


/* todowindow_on_edit */
static void _todowindow_on_edit(gpointer data)
{
	TodoWindow * todo = data;

	todo_task_edit(todo->todo);
}


/* todowindow_on_new */
static void _todowindow_on_new(gpointer data)
{
	TodoWindow * todo = data;

	todo_task_add(todo->todo, NULL);
}


/* todowindow_on_preferences */
static void _todowindow_on_preferences(gpointer data)
{
	TodoWindow * todo = data;

	todo_show_preferences(todo->todo, TRUE);
}


#ifndef EMBEDDED
/* file menu */
/* todowindow_on_file_close */
static void _todowindow_on_file_close(gpointer data)
{
	TodoWindow * todo = data;

	_todowindow_on_close(todo);
}


/* todowindow_on_file_edit */
static void _todowindow_on_file_edit(gpointer data)
{
	TodoWindow * todo = data;

	_todowindow_on_edit(todo);
}


/* todowindow_on_file_new */
static void _todowindow_on_file_new(gpointer data)
{
	TodoWindow * todo = data;

	_todowindow_on_new(todo);
}


/* edit menu */
/* todowindow_on_edit_delete */
static void _todowindow_on_edit_delete(gpointer data)
{
	TodoWindow * todo = data;

	todo_task_delete_selected(todo->todo);
}


/* todowindow_on_edit_preferences */
static void _todowindow_on_edit_preferences(gpointer data)
{
	TodoWindow * todo = data;

	_todowindow_on_preferences(todo);
}


/* todowindow_on_edit_select_all */
static void _todowindow_on_edit_select_all(gpointer data)
{
	TodoWindow * todo = data;

	todo_task_select_all(todo->todo);
}


/* view menu */
/* todowindow_on_view_all_tasks */
static void _todowindow_on_view_all_tasks(gpointer data)
{
	TodoWindow * todo = data;

	todo_set_view(todo->todo, TODO_VIEW_ALL_TASKS);
}


/* todowindow_on_view_completed_tasks */
static void _todowindow_on_view_completed_tasks(gpointer data)
{
	TodoWindow * todo = data;

	todo_set_view(todo->todo, TODO_VIEW_COMPLETED_TASKS);
}


/* todowindow_on_view_remaining_tasks */
static void _todowindow_on_view_remaining_tasks(gpointer data)
{
	TodoWindow * todo = data;

	todo_set_view(todo->todo, TODO_VIEW_REMAINING_TASKS);
}


/* help menu */
/* todowindow_on_help_about */
static void _todowindow_on_help_about(gpointer data)
{
	TodoWindow * todo = data;

	todo_about(todo->todo);
}
#endif
