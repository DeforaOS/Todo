/* $Id$ */
static char _copyright[] =
"Copyright © 2009-2022 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Todo */
static char const _license[] = "All rights reserved.\n"
"\n"
"Redistribution and use in source and binary forms, with or without\n"
"modification, are permitted provided that the following conditions are\n"
"met:\n"
"\n"
"1. Redistributions of source code must retain the above copyright\n"
"   notice, this list of conditions and the following disclaimer.\n"
"\n"
"2. Redistributions in binary form must reproduce the above copyright\n"
"   notice, this list of conditions and the following disclaimer in the\n"
"   documentation and/or other materials provided with the distribution.\n"
"\n"
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS\n"
"IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED\n"
"TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A\n"
"PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n"
"HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n"
"SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED\n"
"TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR\n"
"PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF\n"
"LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING\n"
"NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
"SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";
/* TODO:
 * - handle when the time/date is not set yet
 * - add a clear/apply button (allocate a temporary object) */



#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <System.h>
#include <Desktop.h>
#include "priority.h"
#include "taskedit.h"
#include "todo.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)

#ifndef PROGNAME_TODO
# define PROGNAME_TODO	"todo"
#endif


/* Todo */
/* private */
/* types */
typedef enum _TodoColumn { TD_COL_TASK, TD_COL_DONE, TD_COL_TITLE, TD_COL_START,
	TD_COL_DISPLAY_START, TD_COL_END, TD_COL_DISPLAY_END, TD_COL_PRIORITY,
	TD_COL_DISPLAY_PRIORITY, TD_COL_CATEGORY } TodoColumn;
#define TD_COL_LAST TD_COL_CATEGORY
#define TD_COL_COUNT (TD_COL_LAST + 1)

struct _Todo
{
	GtkWidget * window;
	GtkWidget * widget;
	GtkWidget * scrolled;
	GtkListStore * store;
	GtkListStore * priorities;
	GtkTreeModel * filter;
	GtkTreeModel * filter_sort;
	TodoView filter_view;
	GtkWidget * view;
	GtkTreeViewColumn * columns[TD_COL_COUNT];
	GtkWidget * about;
};


/* prototypes */
static int _todo_confirm(GtkWidget * window, char const * message);
static gboolean _todo_get_iter(Todo * todo, GtkTreeIter * iter,
		GtkTreePath * path);
static char * _todo_task_get_directory(void);
static char * _todo_task_get_filename(char const * filename);
static char * _todo_task_get_new_filename(void);
static void _todo_task_save(Todo * todo, GtkTreeIter * iter);

/* callbacks */
/* toolbar */
static void _todo_on_new(gpointer data);
static void _todo_on_edit(gpointer data);
static void _todo_on_select_all(gpointer data);
static void _todo_on_delete(gpointer data);
#ifdef EMBEDDED
static void _todo_on_preferences(gpointer data);
#endif
static void _todo_on_view_as(gpointer data);

/* view */
static void _todo_on_task_activated(gpointer data);
static void _todo_on_task_cursor_changed(gpointer data);
static void _todo_on_task_done_toggled(GtkCellRendererToggle * renderer,
		gchar * path, gpointer data);
static void _todo_on_task_priority_edited(GtkCellRendererText * renderer,
		gchar * path, gchar * priority, gpointer data);
static void _todo_on_task_title_edited(GtkCellRendererText * renderer,
		gchar * path, gchar * title, gpointer data);
static void _todo_on_view_all_tasks(gpointer data);
static void _todo_on_view_completed_tasks(gpointer data);
static void _todo_on_view_remaining_tasks(gpointer data);

static gboolean _todo_on_filter_view(GtkTreeModel * model, GtkTreeIter * iter,
		gpointer data);


/* constants */
static const struct
{
	int col;
	char const * title;
	int sort;
	GCallback callback;
} _todo_columns[] =
{
	{ TD_COL_DONE, N_("Done"), TD_COL_DONE, G_CALLBACK(
			_todo_on_task_done_toggled) },
	{ TD_COL_TITLE, N_("Title"), TD_COL_TITLE, G_CALLBACK(
			_todo_on_task_title_edited) },
	{ TD_COL_DISPLAY_START, N_("Beginning"), TD_COL_START, NULL },
	{ TD_COL_DISPLAY_END, N_("Completion"), TD_COL_END, NULL },
	{ 0, NULL, 0, NULL }
};


static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

/* toolbar */
static DesktopToolbar _toolbar[] =
{
	{ N_("New task"), G_CALLBACK(_todo_on_new), GTK_STOCK_NEW, 0, 0, NULL },
	{ N_("Edit task"), G_CALLBACK(_todo_on_edit), GTK_STOCK_EDIT, 0, 0,
		NULL },
	{ "", NULL, NULL, 0, 0, NULL },
#if GTK_CHECK_VERSION(2, 10, 0)
	{ N_("Select all"), G_CALLBACK(_todo_on_select_all),
		GTK_STOCK_SELECT_ALL, 0, 0, NULL },
#else
	{ N_("Select all"), G_CALLBACK(_todo_on_select_all), "edit-select-all",
		0, 0, NULL },
#endif
	{ N_("Delete task"), G_CALLBACK(_todo_on_delete), GTK_STOCK_DELETE, 0,
		0, NULL },
#ifdef EMBEDDED
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Preferences"), G_CALLBACK(_todo_on_preferences),
		GTK_STOCK_PREFERENCES, 0, 0, NULL },
#endif
	{ "", NULL, NULL, 0, 0, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL }
};


/* public */
/* functions */
/* todo_new */
static void _new_view(Todo * todo);
static gboolean _new_idle(gpointer data);

Todo * todo_new(GtkWidget * window, GtkAccelGroup * group)
{
	Todo * todo;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkToolItem * toolitem;
	GtkWidget * menu;
	GtkWidget * menuitem;

	if((todo = object_new(sizeof(*todo))) == NULL)
		return NULL;
	/* main window */
	todo->window = window;
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	todo->widget = vbox;
	/* toolbar */
	widget = desktop_toolbar_create(_toolbar, todo, group);
	toolitem = gtk_menu_tool_button_new(NULL, _("View..."));
	g_signal_connect_swapped(toolitem, "clicked", G_CALLBACK(
				_todo_on_view_as), todo);
	menu = gtk_menu_new();
	menuitem = gtk_menu_item_new_with_label(_("All tasks"));
	g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(
				_todo_on_view_all_tasks), todo);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label(_("Completed tasks"));
	g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(
				_todo_on_view_completed_tasks), todo);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label(_("Remaining tasks"));
	g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(
				_todo_on_view_remaining_tasks), todo);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	gtk_widget_show_all(menu);
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(toolitem), menu);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* view */
	todo->scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(todo->scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	_new_view(todo);
	gtk_box_pack_start(GTK_BOX(vbox), todo->scrolled, TRUE, TRUE, 0);
	todo->about = NULL;
	g_idle_add(_new_idle, todo);
	return todo;
}

static void _new_view(Todo * todo)
{
	size_t i;
	GtkTreeIter iter;
	GtkTreeSelection * sel;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	todo->store = gtk_list_store_new(TD_COL_COUNT,
			G_TYPE_POINTER, /* task */
			G_TYPE_BOOLEAN, /* done */
			G_TYPE_STRING,	/* title */
			G_TYPE_UINT64,	/* start */
			G_TYPE_STRING,	/* display start */
			G_TYPE_UINT64,	/* end */
			G_TYPE_STRING,	/* display end */
			G_TYPE_UINT,	/* priority */
			G_TYPE_STRING,	/* display priority */
			G_TYPE_STRING);	/* category */
	todo->priorities = gtk_list_store_new(2, G_TYPE_UINT, G_TYPE_STRING);
	for(i = 0; priorities[i].title != NULL; i++)
	{
		gtk_list_store_append(todo->priorities, &iter);
		gtk_list_store_set(todo->priorities, &iter,
				0, priorities[i].priority,
				1, _(priorities[i].title), -1);
	}
	todo->filter = gtk_tree_model_filter_new(GTK_TREE_MODEL(todo->store),
			NULL);
	todo->filter_view = TODO_VIEW_ALL_TASKS;
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(
				todo->filter), _todo_on_filter_view, todo,
			NULL);
	todo->filter_sort = gtk_tree_model_sort_new_with_model(todo->filter);
	todo->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				todo->filter_sort));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(todo->view), TRUE);
	if((sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(todo->view)))
			!= NULL)
		gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
	g_signal_connect_swapped(todo->view, "cursor-changed", G_CALLBACK(
				_todo_on_task_cursor_changed), todo);
	g_signal_connect_swapped(todo->view, "row-activated", G_CALLBACK(
				_todo_on_task_activated), todo);
	/* columns */
	memset(&todo->columns, 0, sizeof(todo->columns));
	/* done column */
	renderer = gtk_cell_renderer_toggle_new();
	g_signal_connect(renderer, "toggled", G_CALLBACK(
				_todo_columns[0].callback), todo);
	column = gtk_tree_view_column_new_with_attributes(
			_(_todo_columns[0].title), renderer, "active",
			_todo_columns[0].col, NULL);
	todo->columns[TD_COL_DONE] = column;
	gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(column),
			GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(column), 50);
	gtk_tree_view_column_set_sort_column_id(column, TD_COL_DONE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(todo->view), column);
	/* other columns */
	for(i = 1; _todo_columns[i].title != NULL; i++)
	{
		renderer = gtk_cell_renderer_text_new();
		if(_todo_columns[i].callback != NULL)
		{
			g_object_set(G_OBJECT(renderer), "editable", TRUE,
					"ellipsize", PANGO_ELLIPSIZE_END, NULL);
			g_signal_connect(renderer, "edited", G_CALLBACK(
						_todo_columns[i].callback),
					todo);
		}
		column = gtk_tree_view_column_new_with_attributes(
				_(_todo_columns[i].title), renderer, "text",
				_todo_columns[i].col, NULL);
		todo->columns[_todo_columns[i].col] = column;
#if GTK_CHECK_VERSION(2, 4, 0)
		gtk_tree_view_column_set_expand(column, TRUE);
#endif
		gtk_tree_view_column_set_resizable(column, TRUE);
		gtk_tree_view_column_set_sort_column_id(column,
				_todo_columns[i].sort);
		gtk_tree_view_append_column(GTK_TREE_VIEW(todo->view), column);
	}
	/* priority column */
	renderer = gtk_cell_renderer_combo_new();
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END,
			"model", todo->priorities, "text-column", 1,
			"editable", TRUE, NULL);
	g_signal_connect(renderer, "edited", G_CALLBACK(
				_todo_on_task_priority_edited), todo);
	column = gtk_tree_view_column_new_with_attributes(_("Priority"),
			renderer, "text", TD_COL_DISPLAY_PRIORITY, NULL);
	todo->columns[TD_COL_DISPLAY_PRIORITY] = column;
#if GTK_CHECK_VERSION(2, 4, 0)
	gtk_tree_view_column_set_expand(column, TRUE);
#endif
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, TD_COL_PRIORITY);
	gtk_container_add(GTK_CONTAINER(todo->scrolled), todo->view);
	gtk_tree_view_append_column(GTK_TREE_VIEW(todo->view), column);
}

static gboolean _new_idle(gpointer data)
{
	Todo * todo = data;

	todo_task_reload_all(todo);
	return FALSE;
}


/* todo_delete */
void todo_delete(Todo * todo)
{
	todo_task_save_all(todo);
	todo_task_remove_all(todo);
	free(todo);
	object_delete(todo);
}


/* accessors */
/* todo_get_view */
TodoView todo_get_view(Todo * todo)
{
	return todo->filter_view;
}


/* todo_get_widget */
GtkWidget * todo_get_widget(Todo * todo)
{
	return todo->widget;
}


/* todo_set_view */
void todo_set_view(Todo * todo, TodoView view)
{
	todo->filter_view = view;
	gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(todo->filter));
}


/* useful */
/* todo_about */
static gboolean _about_on_closex(gpointer data);

void todo_about(Todo * todo)
{
	if(todo->about != NULL)
	{
		gtk_widget_show(todo->about);
		return;
	}
	todo->about = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(todo->about),
			GTK_WINDOW(todo->window));
	desktop_about_dialog_set_authors(todo->about, _authors);
	desktop_about_dialog_set_comments(todo->about,
			_("TODO-list manager for the DeforaOS desktop"));
	desktop_about_dialog_set_copyright(todo->about, _copyright);
	desktop_about_dialog_set_logo_icon_name(todo->about, "todo");
	desktop_about_dialog_set_license(todo->about, _license);
	desktop_about_dialog_set_program_name(todo->about, PACKAGE);
	desktop_about_dialog_set_translator_credits(todo->about,
			_("translator-credits"));
	desktop_about_dialog_set_version(todo->about, VERSION);
	desktop_about_dialog_set_website(todo->about,
			"https://www.defora.org/");
	g_signal_connect_swapped(todo->about, "delete-event", G_CALLBACK(
				_about_on_closex), todo);
	gtk_widget_show(todo->about);
}

static gboolean _about_on_closex(gpointer data)
{
	Todo * todo = data;

	gtk_widget_hide(todo->about);
	return TRUE;
}


/* todo_error */
static int _error_text(char const * message, int ret);

int todo_error(Todo * todo, char const * message, int ret)
{
	GtkWidget * dialog;

	if(todo == NULL)
		return _error_text(message, ret);
	dialog = gtk_message_dialog_new(GTK_WINDOW(todo->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s",
#if GTK_CHECK_VERSION(2, 8, 0)
			_("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}

static int _error_text(char const * message, int ret)
{
	fputs(PROGNAME_TODO ": ", stderr);
	fputs(message, stderr);
	fputc('\n', stderr);
	return ret;
}


/* todo_show_preferences */
void todo_show_preferences(Todo * todo, gboolean show)
{
	/* FIXME implement */
}


/* tasks */
/* todo_task_add */
Task * todo_task_add(Todo * todo, Task * task)
{
	GtkTreeIter iter;
	char * filename;
	time_t start;
	struct tm t;
	char beginning[32] = "";
	time_t end;
	char completion[32] = "";
	char const * priority;
	TodoPriority tp = TODO_PRIORITY_UNKNOWN;
	size_t i;

	if(task == NULL)
	{
		if((task = task_new()) == NULL)
			return NULL;
		if((filename = _todo_task_get_new_filename()) == NULL)
		{
			todo_error(todo, error_get(NULL), 0);
			task_delete(task);
			return NULL;
		}
		task_set_filename(task, filename);
		free(filename);
		task_set_title(task, _("New task"));
		task_save(task);
	}
	gtk_list_store_insert(todo->store, &iter, 0);
	if((start = task_get_start(task)) != 0)
	{
		localtime_r(&start, &t);
		strftime(beginning, sizeof(beginning), "%c", &t);
	}
	if((end = task_get_end(task)) != 0)
	{
		localtime_r(&end, &t);
		strftime(completion, sizeof(completion), "%c", &t);
	}
	priority = task_get_priority(task);
	for(i = 0; priority != NULL && priorities[i].title != NULL; i++)
		if(strcmp(_(priorities[i].title), priority) == 0)
		{
			tp = priorities[i].priority;
			break;
		}
	gtk_list_store_set(todo->store, &iter, TD_COL_TASK, task,
			TD_COL_DONE, task_get_done(task) > 0 ? TRUE : FALSE,
			TD_COL_TITLE, task_get_title(task),
			TD_COL_START, start,
			TD_COL_DISPLAY_START, beginning,
			TD_COL_END, end,
			TD_COL_DISPLAY_END, completion,
			TD_COL_PRIORITY, tp,
			TD_COL_DISPLAY_PRIORITY, priority, -1);
	return task;
}


/* todo_task_delete_selected */
static void _task_delete_selected_foreach(GtkTreeRowReference * reference,
		Todo * todo);

void todo_task_delete_selected(Todo * todo)
{
	GtkTreeSelection * treesel;
	GList * selected;
	GtkTreeModel * model = GTK_TREE_MODEL(todo->store);
	GtkTreeRowReference * reference;
	GList * s;
	GtkTreePath * path;

	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(todo->view)))
			== NULL)
		return;
	if((selected = gtk_tree_selection_get_selected_rows(treesel, NULL))
			== NULL)
		return;
	if(_todo_confirm(todo->window, _("Are you sure you want to delete the"
					" selected task(s)?")) != 0)
		return;
	for(s = g_list_first(selected); s != NULL; s = g_list_next(s))
	{
		if((path = s->data) == NULL)
			continue;
		reference = gtk_tree_row_reference_new(model, path);
		s->data = reference;
		gtk_tree_path_free(path);
	}
	g_list_foreach(selected, (GFunc)_task_delete_selected_foreach, todo);
	g_list_free(selected);
}

static void _task_delete_selected_foreach(GtkTreeRowReference * reference,
		Todo * todo)
{
	GtkTreeModel * model = GTK_TREE_MODEL(todo->store);
	GtkTreePath * path;
	GtkTreeIter iter;
	Task * task;

	if(reference == NULL)
		return;
	if((path = gtk_tree_row_reference_get_path(reference)) == NULL)
		return;
	if(_todo_get_iter(todo, &iter, path) == TRUE)
	{
		gtk_tree_model_get(model, &iter, TD_COL_TASK, &task, -1);
		task_unlink(task);
		task_delete(task);
	}
	gtk_list_store_remove(todo->store, &iter);
	gtk_tree_path_free(path);
}


/* todo_task_cursor_changed */
static void _task_cursor_changed_date_end(GtkWidget * widget, gpointer data);
static void _task_cursor_changed_date_start(GtkWidget * widget, gpointer data);
static void _task_cursor_changed_hour_end(GtkWidget * widget, gpointer data);
static void _task_cursor_changed_hour_start(GtkWidget * widget, gpointer data);
static void _task_cursor_changed_min_end(GtkWidget * widget, gpointer data);
static void _task_cursor_changed_min_start(GtkWidget * widget, gpointer data);
static void _task_cursor_changed_sec_end(GtkWidget * widget, gpointer data);
static void _task_cursor_changed_sec_start(GtkWidget * widget, gpointer data);

void todo_task_cursor_changed(Todo * todo)
{
	GtkTreeModel * model = GTK_TREE_MODEL(todo->store);
	GtkTreePath * path = NULL;
	GtkTreeViewColumn * column = NULL;
	GtkTreeIter iter;
	Task * task = NULL;
	gint id = -1;
	GdkRectangle rect;
	GtkWidget * popup;
	GtkWidget * vbox;
	GtkWidget * hbox;
	time_t tim;
	struct tm t;
	GtkWidget * button;
	GtkWidget * label;
	GtkWidget * image;
	GtkWidget * calendar;

	gtk_tree_view_get_cursor(GTK_TREE_VIEW(todo->view), &path, &column);
	if(path == NULL)
		return;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, TD_COL_TASK, &task, -1);
	if(column != NULL)
		id = gtk_tree_view_column_get_sort_column_id(column);
	if(id == TD_COL_END || id == TD_COL_START)
	{
		/* window */
		popup = gtk_window_new(GTK_WINDOW_POPUP);
		gtk_container_set_border_width(GTK_CONTAINER(popup), 4);
		gtk_window_set_modal(GTK_WINDOW(popup), TRUE);
		gtk_window_set_transient_for(GTK_WINDOW(popup), GTK_WINDOW(
					todo->window));
		vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
		hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
		if((tim = (id == TD_COL_START) ? task_get_start(task)
					: task_get_end(task)) == 0)
			tim = time(NULL);
		localtime_r(&tim, &t);
		/* time */
		label = gtk_label_new(_("Time: "));
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
		button = gtk_spin_button_new_with_range(0.0, 23.0, 1.0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(button), t.tm_hour);
		g_signal_connect(button, "value-changed", G_CALLBACK(
					(id == TD_COL_START)
					? _task_cursor_changed_hour_start
					: _task_cursor_changed_hour_end), task);
		gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 0);
		label = gtk_label_new(_(":"));
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
		button = gtk_spin_button_new_with_range(0.0, 59.0, 1.0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(button), t.tm_min);
		g_signal_connect(button, "value-changed", G_CALLBACK(
					(id == TD_COL_START)
					? _task_cursor_changed_min_start
					: _task_cursor_changed_min_end), task);
		gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 0);
		label = gtk_label_new(_(":"));
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
		button = gtk_spin_button_new_with_range(0.0, 59.0, 1.0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(button), t.tm_sec);
		g_signal_connect(button, "value-changed", G_CALLBACK(
					(id == TD_COL_START)
					? _task_cursor_changed_sec_start
					: _task_cursor_changed_sec_end), task);
		gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 0);
		/* close button */
		button = gtk_button_new();
		image = gtk_image_new_from_icon_name("gtk-close",
				GTK_ICON_SIZE_MENU);
		gtk_button_set_image(GTK_BUTTON(button), image);
		gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
		g_signal_connect_swapped(button, "clicked", G_CALLBACK(
					gtk_widget_destroy), popup);
		gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
		/* date */
		calendar = gtk_calendar_new();
		gtk_calendar_select_day(GTK_CALENDAR(calendar), t.tm_mday);
		gtk_calendar_select_month(GTK_CALENDAR(calendar), t.tm_mon,
				1900 + t.tm_year);
		g_signal_connect(calendar, "day-selected-double-click",
				G_CALLBACK((id == TD_COL_START)
					? _task_cursor_changed_date_start
					: _task_cursor_changed_date_end), task);
		gtk_box_pack_start(GTK_BOX(vbox), calendar, FALSE, TRUE, 0);
		gtk_container_add(GTK_CONTAINER(popup), vbox);
		gtk_tree_view_get_cell_area(GTK_TREE_VIEW(todo->view), path,
				column, &rect);
		gtk_window_get_position(GTK_WINDOW(todo->window), &rect.width,
				&rect.height);
		gtk_window_move(GTK_WINDOW(popup), rect.width + rect.x,
				rect.height + rect.y);
		gtk_widget_show_all(popup);
	}
	gtk_tree_path_free(path);
}

static time_t _task_cursor_changed_date_get(GtkWidget * widget, time_t time)
{
	struct tm t;
	unsigned int year;
	unsigned int month;
	unsigned int day;

	localtime_r(&time, &t);
	gtk_calendar_get_date(GTK_CALENDAR(widget), &year, &month, &day);
	t.tm_year = year - 1900;
	t.tm_mon = month;
	t.tm_mday = day;
	return mktime(&t);
}

static void _task_cursor_changed_date_end(GtkWidget * widget, gpointer data)
{
	Task * task = data;
	time_t time;

	time = task_get_end(task);
	time = _task_cursor_changed_date_get(widget, time);
	task_set_end(task, time);
	task_save(task);
	/* FIXME actually reflect this in the GtkTreeView */
}

static void _task_cursor_changed_date_start(GtkWidget * widget, gpointer data)
{
	Task * task = data;
	time_t time;

	time = task_get_start(task);
	time = _task_cursor_changed_date_get(widget, time);
	task_set_start(task, time);
	task_save(task);
	/* FIXME actually reflect this in the GtkTreeView */
}

static void _task_cursor_changed_hour_end(GtkWidget * widget, gpointer data)
{
	Task * task = data;
	time_t time;
	struct tm t;

	time = task_get_end(task);
	localtime_r(&time, &t);
	t.tm_hour = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	time = mktime(&t);
	task_set_end(task, time);
	task_save(task);
	/* FIXME actually reflect this in the GtkTreeView */
}

static void _task_cursor_changed_hour_start(GtkWidget * widget, gpointer data)
{
	Task * task = data;
	time_t time;
	struct tm t;

	time = task_get_start(task);
	localtime_r(&time, &t);
	t.tm_hour = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	time = mktime(&t);
	task_set_start(task, time);
	task_save(task);
	/* FIXME actually reflect this in the GtkTreeView */
}

static void _task_cursor_changed_min_end(GtkWidget * widget, gpointer data)
{
	Task * task = data;
	time_t time;
	struct tm t;

	time = task_get_end(task);
	localtime_r(&time, &t);
	t.tm_min = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	time = mktime(&t);
	task_set_end(task, time);
	task_save(task);
	/* FIXME actually reflect this in the GtkTreeView */
}

static void _task_cursor_changed_min_start(GtkWidget * widget, gpointer data)
{
	Task * task = data;
	time_t time;
	struct tm t;

	time = task_get_start(task);
	localtime_r(&time, &t);
	t.tm_min = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	time = mktime(&t);
	task_set_start(task, time);
	task_save(task);
	/* FIXME actually reflect this in the GtkTreeView */
}

static void _task_cursor_changed_sec_end(GtkWidget * widget, gpointer data)
{
	Task * task = data;
	time_t time;
	struct tm t;

	time = task_get_end(task);
	localtime_r(&time, &t);
	t.tm_sec = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	time = mktime(&t);
	task_set_end(task, time);
	task_save(task);
	/* FIXME actually reflect this in the GtkTreeView */
}

static void _task_cursor_changed_sec_start(GtkWidget * widget, gpointer data)
{
	Task * task = data;
	time_t time;
	struct tm t;

	time = task_get_start(task);
	localtime_r(&time, &t);
	t.tm_sec = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	time = mktime(&t);
	task_set_start(task, time);
	task_save(task);
	/* FIXME actually reflect this in the GtkTreeView */
}


/* todo_task_edit */
void todo_task_edit(Todo * todo)
{
	GtkTreeSelection * treesel;
	GList * selected;
	GtkTreeModel * model = GTK_TREE_MODEL(todo->store);
	GList * s;
	GtkTreePath * path;
	GtkTreeIter iter;
	Task * task;

	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(todo->view)))
			== NULL)
		return;
	if((selected = gtk_tree_selection_get_selected_rows(treesel, NULL))
			== NULL)
		return;
	for(s = g_list_first(selected); s != NULL; s = g_list_next(s))
	{
		if((path = s->data) == NULL)
			continue;
		if(_todo_get_iter(todo, &iter, path) != TRUE)
			continue;
		gtk_tree_model_get(model, &iter, TD_COL_TASK, &task, -1);
		if(task != NULL)
			taskedit_new(todo, task);
	}
	g_list_free(selected);
}


/* todo_task_reload_all */
int todo_task_reload_all(Todo * todo)
{
	int ret = 0;
	char * filename;
	DIR * dir;
	struct dirent * de;
	Task * task;

	if((filename = _todo_task_get_directory()) == NULL)
		return todo_error(todo, error_get(NULL), 1);
	if((dir = opendir(filename)) == NULL)
	{
		if(errno != ENOENT)
		{
			error_set("%s: %s", filename, strerror(errno));
			ret = todo_error(todo, error_get(NULL), 1);
		}
	}
	else
	{
		todo_task_remove_all(todo);
		while((de = readdir(dir)) != NULL)
		{
			if(strncmp(de->d_name, "task.", 5) != 0)
				continue;
			free(filename);
			if((filename = _todo_task_get_filename(de->d_name))
					== NULL)
				continue; /* XXX report error */
			if((task = task_new_from_file(filename)) == NULL)
			{
				todo_error(NULL, error_get(NULL), 1);
				continue;
			}
			if(todo_task_add(todo, task) == NULL)
			{
				task_delete(task);
				continue; /* XXX report error */
			}
		}
	}
	free(filename);
	return ret;
}


/* todo_task_remove_all */
void todo_task_remove_all(Todo * todo)
{
	GtkTreeModel * model = GTK_TREE_MODEL(todo->store);
	GtkTreeIter iter;
	gboolean valid;
	Task * task;

	valid = gtk_tree_model_get_iter_first(model, &iter);
	for(; valid == TRUE; valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, TD_COL_TASK, &task, -1);
		task_delete(task);
	}
	gtk_list_store_clear(todo->store);
}


/* todo_task_save_all */
void todo_task_save_all(Todo * todo)
{
	GtkTreeModel * model = GTK_TREE_MODEL(todo->store);
	GtkTreeIter iter;
	gboolean valid;

	valid = gtk_tree_model_get_iter_first(model, &iter);
	for(; valid == TRUE; valid = gtk_tree_model_iter_next(model, &iter))
		_todo_task_save(todo, &iter);
}


/* todo_task_select_all */
void todo_task_select_all(Todo * todo)
{
	GtkTreeSelection * sel;

	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(todo->view));
	gtk_tree_selection_select_all(sel);
}


/* todo_task_set_priority */
void todo_task_set_priority(Todo * todo, GtkTreePath * path,
		char const * priority)
{
	GtkTreeModel * model = GTK_TREE_MODEL(todo->store);
	GtkTreeIter iter;
	Task * task;
	size_t i;
	TodoPriority tp = TODO_PRIORITY_UNKNOWN;

	_todo_get_iter(todo, &iter, path);
	gtk_tree_model_get(model, &iter, TD_COL_TASK, &task, -1);
	task_set_priority(task, priority);
	for(i = 0; priorities[i].title != NULL; i++)
		if(strcmp(_(priorities[i].title), priority) == 0)
		{
			tp = priorities[i].priority;
			break;
		}
	gtk_list_store_set(todo->store, &iter, TD_COL_PRIORITY, tp,
			TD_COL_DISPLAY_PRIORITY, priority, -1);
	task_save(task);
}


/* todo_task_set_title */
void todo_task_set_title(Todo * todo, GtkTreePath * path, char const * title)
{
	GtkTreeModel * model = GTK_TREE_MODEL(todo->store);
	GtkTreeIter iter;
	Task * task;

	_todo_get_iter(todo, &iter, path);
	gtk_tree_model_get(model, &iter, TD_COL_TASK, &task, -1);
	task_set_title(task, title);
	gtk_list_store_set(todo->store, &iter, TD_COL_TITLE, title, -1);
	task_save(task);
}


/* todo_task_toggle_done */
void todo_task_toggle_done(Todo * todo, GtkTreePath * path)
{
	GtkTreeIter iter;
	Task * task;
	gboolean done;
	time_t end;
	struct tm t;
	char completion[32] = "";

	_todo_get_iter(todo, &iter, path);
	gtk_tree_model_get(GTK_TREE_MODEL(todo->store), &iter,
			TD_COL_TASK, &task, TD_COL_DONE, &done, -1);
	done = !done;
	task_set_done(task, done);
	if((end = task_get_end(task)) != 0) /* XXX code duplication */
	{
		localtime_r(&end, &t);
		strftime(completion, sizeof(completion), "%c", &t);
	}
	gtk_list_store_set(todo->store, &iter, TD_COL_DONE, done,
			TD_COL_END, end, TD_COL_DISPLAY_END, completion, -1);
	task_save(task);
}


/* private */
/* functions */
/* todo_confirm */
static int _todo_confirm(GtkWidget * window, char const * message)
{
	GtkWidget * dialog;
	int res;

	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s",
#if GTK_CHECK_VERSION(2, 8, 0)
			_("Question"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Question"));
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(res == GTK_RESPONSE_YES)
		return 0;
	return 1;
}


/* todo_get_iter */
static gboolean _todo_get_iter(Todo * todo, GtkTreeIter * iter,
		GtkTreePath * path)
{
	GtkTreeIter p;

	if(gtk_tree_model_get_iter(GTK_TREE_MODEL(todo->filter_sort), iter,
				path) == FALSE)
		return FALSE;
	gtk_tree_model_sort_convert_iter_to_child_iter(GTK_TREE_MODEL_SORT(
				todo->filter_sort), &p, iter);
	gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(
				todo->filter), iter, &p);
	return TRUE;
}


/* todo_task_get_directory */
static char * _todo_task_get_directory(void)
{
	char const * homedir;
	size_t len;
	char const directory[] = ".todo";
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(directory);
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, directory);
	return filename;
}


/* todo_task_get_filename */
static char * _todo_task_get_filename(char const * filenam)
{
	char const * homedir;
	int len;
	char const directory[] = ".todo";
	char * pathname;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(directory) + 1 + strlen(filenam) + 1;
	if((pathname = malloc(len)) == NULL)
		return NULL;
	snprintf(pathname, len, "%s/%s/%s", homedir, directory, filenam);
	return pathname;
}


/* todo_task_get_new_filename */
static char * _todo_task_get_new_filename(void)
{
	char const * homedir;
	int len;
	char const directory[] = ".todo";
	char template[] = "task.XXXXXX";
	char * filename;
	int fd;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(directory) + 1 + sizeof(template);
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, directory);
	if((mkdir(filename, 0777) != 0 && errno != EEXIST)
			|| snprintf(filename, len, "%s/%s/%s", homedir,
				directory, template) >= len
			|| (fd = mkstemp(filename)) < 0)
	{
		error_set("%s: %s", filename, strerror(errno));
		free(filename);
		return NULL;
	}
	close(fd);
	return filename;
}


/* todo_task_save */
static void _todo_task_save(Todo * todo, GtkTreeIter * iter)
{
	GtkTreeModel * model = GTK_TREE_MODEL(todo->store);
	Task * task;

	gtk_tree_model_get(model, iter, TD_COL_TASK, &task, -1);
	task_save(task);
}


/* callbacks */
/* todo_on_view_all_tasks */
static void _todo_on_view_all_tasks(gpointer data)
{
	Todo * todo = data;

	todo_set_view(todo, TODO_VIEW_ALL_TASKS);
}


/* todo_on_view_completed_tasks */
static void _todo_on_view_completed_tasks(gpointer data)
{
	Todo * todo = data;

	todo_set_view(todo, TODO_VIEW_COMPLETED_TASKS);
}


/* todo_on_view_remaining_tasks */
static void _todo_on_view_remaining_tasks(gpointer data)
{
	Todo * todo = data;

	todo_set_view(todo, TODO_VIEW_REMAINING_TASKS);
}


/* toolbar */
/* todo_on_delete */
static void _todo_on_delete(gpointer data)
{
	Todo * todo = data;

	todo_task_delete_selected(todo);
}


/* todo_on_edit */
static void _todo_on_edit(gpointer data)
{
	Todo * todo = data;

	todo_task_edit(todo);
}


/* todo_on_new */
static void _todo_on_new(gpointer data)
{
	Todo * todo = data;

	todo_task_add(todo, NULL);
}


#ifdef EMBEDDED
/* todo_on_preferences */
static void _todo_on_preferences(gpointer data)
{
	Todo * todo = data;

	todo_show_preferences(todo, TRUE);
}
#endif


/* todo_on_view_as */
static void _todo_on_view_as(gpointer data)
{
	Todo * todo = data;
	TodoView view;

	view = todo_get_view(todo);
	view = (view + 1) % TODO_VIEW_COUNT;
	todo_set_view(todo, view);
}


/* todo_on_select_all */
static void _todo_on_select_all(gpointer data)
{
	Todo * todo = data;

	todo_task_select_all(todo);
}


/* view */
/* todo_on_task_activated */
static void _todo_on_task_activated(gpointer data)
{
	Todo * todo = data;

	todo_task_edit(todo);
}


/* todo_on_task_cursor_changed */
static void _todo_on_task_cursor_changed(gpointer data)
{
	Todo * todo = data;

	todo_task_cursor_changed(todo);
}


/* todo_on_task_done_toggled */
static void _todo_on_task_done_toggled(GtkCellRendererToggle * renderer,
		gchar * path, gpointer data)
{
	Todo * todo = data;
	GtkTreePath * treepath;
	(void) renderer;

	treepath = gtk_tree_path_new_from_string(path);
	todo_task_toggle_done(todo, treepath);
	gtk_tree_path_free(treepath);
}


/* todo_on_task_priority_edited */
static void _todo_on_task_priority_edited(GtkCellRendererText * renderer,
		gchar * path, gchar * priority, gpointer data)
{
	Todo * todo = data;
	GtkTreePath * treepath;
	(void) renderer;

	treepath = gtk_tree_path_new_from_string(path);
	todo_task_set_priority(todo, treepath, priority);
	gtk_tree_path_free(treepath);
}


/* todo_on_task_title_edited */
static void _todo_on_task_title_edited(GtkCellRendererText * renderer,
		gchar * path, gchar * title, gpointer data)
{
	Todo * todo = data;
	GtkTreePath * treepath;
	(void) renderer;

	treepath = gtk_tree_path_new_from_string(path);
	todo_task_set_title(todo, treepath, title);
	gtk_tree_path_free(treepath);
}


/* todo_on_filter_view */
static gboolean _todo_on_filter_view(GtkTreeModel * model, GtkTreeIter * iter,
		gpointer data)
{
	Todo * todo = data;
	gboolean done = FALSE;

	switch(todo->filter_view)
	{
		case TODO_VIEW_COMPLETED_TASKS:
			gtk_tree_model_get(model, iter, TD_COL_DONE, &done, -1);
			return done ? TRUE : FALSE;
		case TODO_VIEW_REMAINING_TASKS:
			gtk_tree_model_get(model, iter, TD_COL_DONE, &done, -1);
			return done ? FALSE : TRUE;
		default:
		case TODO_VIEW_ALL_TASKS:
			return TRUE;
	}
}
