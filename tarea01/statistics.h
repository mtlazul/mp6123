/*
 * Copyright (C) 2016 Michael Gruner <grunermonzon@gmail.com>
 *
 * This file is part of Simulator
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#include <gst/gst.h>

typedef struct _Stats Stats;

struct _Stats
{
  GstClockTime max;
  GstClockTime min;
  GstClockTime average;
  GstClockTime _tic;
  GstClockTime _toc;
  GstClockTime _start;
  gsize count;
};

#define STATS_NEW {0}

void stats_tic (Stats * stats);
GstClockTime stats_toc (Stats * stats);
void stats_print (Stats *stats);

#endif //__STATISTICS_H__
