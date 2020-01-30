/* $Id$ */
/* Copyright (c) 2010-2015 Pierre Pronchery <khorben@defora.org> */
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



#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <System.h>
#include "task.h"


/* Task */
/* private */
/* types */
struct _Task
{
	Config * config;

	/* internal */
	char * filename;
	String * description;
};


/* prototype */
static int _task_config_get_boolean(Task * task, char const * section,
		char const * variable);


/* public */
/* functions */
/* task_new */
Task * task_new(void)
{
	Task * task;

	if((task = object_new(sizeof(*task))) == NULL)
		return NULL;
	task->config = config_new();
	task->filename = NULL;
	task->description = NULL;
	if(task->config == NULL)
	{
		task_delete(task);
		return NULL;
	}
	task_set_start(task, time(NULL));
	return task;
}


/* task_new_from_file */
Task * task_new_from_file(char const * filename)
{
	Task * task;

	if((task = task_new()) == NULL)
		return NULL;
	if(task_set_filename(task, filename) != 0
			|| task_load(task) != 0)
	{
		task_delete(task);
		return NULL;
	}
	return task;
}


/* task_delete */
void task_delete(Task * task)
{
	string_delete(task->description);
	free(task->filename);
	if(task->config != NULL)
		config_delete(task->config);
	object_delete(task);
}


/* accessors */
/* task_get_description */
char const * task_get_description(Task * task)
{
	String const * p;
	String * q;

	if(task->description != NULL)
		return task->description;
	if((p = config_get(task->config, NULL, "description")) == NULL)
		return "";
	if((q = string_new_replace(p, "\\n", "\n")) == NULL
			|| string_replace(&q, "\\\\", "\\") != 0)
		return NULL;
	task->description = q;
	return task->description;
}


/* task_get_done */
int task_get_done(Task * task)
{
	return _task_config_get_boolean(task, NULL, "done");
}


/* task_get_end */
time_t task_get_end(Task * task)
{
	char const * end;

	if((end = config_get(task->config, NULL, "end")) == NULL)
		return 0;
	return atoi(end);
}


/* task_get_filename */
char const * task_get_filename(Task * task)
{
	return task->filename;
}


/* task_get_priority */
char const * task_get_priority(Task * task)
{
	char const * ret;

	if((ret = config_get(task->config, NULL, "priority")) == NULL)
		return "";
	return ret;
}


/* task_get_start */
time_t task_get_start(Task * task)
{
	char const * start;

	if((start = config_get(task->config, NULL, "start")) == NULL)
		return 0;
	return atoi(start);
}


/* task_get_title */
char const * task_get_title(Task * task)
{
	char const * ret;

	if((ret = config_get(task->config, NULL, "title")) == NULL)
		return "";
	return ret;
}


/* task_set_description */
int task_set_description(Task * task, char const * description)
{
	String * d;

	if((d = string_new_replace(description, "\\", "\\\\")) == NULL)
		return -1;
	if(string_replace(&d, "\n", "\\n") != 0
			|| config_set(task->config, NULL, "description", d)
			!= 0)
	{
		string_delete(d);
		return -1;
	}
	string_delete(task->description);
	task->description = d;
	return 0;
}


/* task_set_done */
int task_set_done(Task * task, int done)
{
	task_set_end(task, done ? time(NULL) : 0);
	return config_set(task->config, NULL, "done", done ? "1" : "0");
}


/* task_set_end */
int task_set_end(Task * task, time_t end)
{
	char buf[32];

	if(end == 0)
		return config_set(task->config, NULL, "end", NULL);
	snprintf(buf, sizeof(buf), "%lu", (unsigned long)end);
	return config_set(task->config, NULL, "end", buf);
}


/* task_set_filename */
int task_set_filename(Task * task, char const * filename)
{
	char * p;

	if((p = strdup(filename)) == NULL)
		return -1; /* XXX set error */
	free(task->filename);
	task->filename = p;
	return 0;
}


/* task_set_priority */
int task_set_priority(Task * task, char const * priority)
{
	return config_set(task->config, NULL, "priority", priority);
}


/* task_set_start */
int task_set_start(Task * task, time_t start)
{
	char buf[16];

	snprintf(buf, sizeof(buf), "%lu", (unsigned long)start);
	return config_set(task->config, NULL, "start", buf);
}


/* task_set_title */
int task_set_title(Task * task, char const * title)
{
	return config_set(task->config, NULL, "title", title);
}


/* useful */
/* task_load */
int task_load(Task * task)
{
	config_reset(task->config);
	return config_load(task->config, task->filename);
}


/* task_save */
int task_save(Task * task)
{
	if(task->filename == NULL)
		return -1; /* XXX set error */
	return config_save(task->config, task->filename);
}


/* task_unlink */
int task_unlink(Task * task)
{
	if(task->filename == NULL)
		return -1; /* XXX set error */
	return unlink(task->filename);
}


/* private */
/* functions */
/* task_config_get_boolean */
static int _task_config_get_boolean(Task * task, char const * section,
		char const * variable)
{
	int ret;
	char const * string;
	char * p;

	if((string = config_get(task->config, section, variable)) == NULL)
		return -1;
	ret = strtol(string, &p, 10);
	if(string[0] == '\0' || *p != '\0')
		return -1;
	return ret ? 1 : 0;
}
