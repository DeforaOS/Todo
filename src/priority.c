/* $Id$ */
/* Copyright (c) 2015 Pierre Pronchery <khorben@defora.org> */
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



#include "priority.h"
#define N_(string) (string)


/* variables */
TodoPriorityTitle priorities[] =
{
	{ TODO_PRIORITY_UNKNOWN,N_("Unknown")	},
	{ TODO_PRIORITY_LOW,	N_("Low")	},
	{ TODO_PRIORITY_MEDIUM,	N_("Medium")	},
	{ TODO_PRIORITY_HIGH,	N_("High")	},
	{ TODO_PRIORITY_URGENT,	N_("Urgent")	},
	{ 0,			NULL		}
};
