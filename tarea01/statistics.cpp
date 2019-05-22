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

#include "statistics.h"

static GstClockTime do_stats (Stats * stats);

void stats_print (Stats * stats)
{
  GstClockTime end;
  GstClockTime diff;
  
  g_return_if_fail (stats);

  end = gst_util_get_timestamp ();
  diff = GST_CLOCK_DIFF(stats->_start, end);
  
  g_print ("=========================================\n");
  g_print (" Stat   \t\tValue (HH:MM:SS)\n");
  g_print ("=========================================\n");
  g_print (" Minimum\t\t%" GST_TIME_FORMAT "\n",
      GST_TIME_ARGS(stats->min));
  g_print (" Maximum\t\t%" GST_TIME_FORMAT "\n",
      GST_TIME_ARGS(stats->max));
  g_print (" Average\t\t%" GST_TIME_FORMAT "\n",
      GST_TIME_ARGS(stats->average));
  g_print ("=========================================\n");
  g_print (" Total time\t\t%" GST_TIME_FORMAT "\n",
      GST_TIME_ARGS(diff));
  g_print (" Samples\t\t%0.17lu\n", stats->count);
  g_print ("=========================================\n");
}

void stats_tic (Stats * stats)
{
  g_return_if_fail (stats);
    
  stats->_tic = gst_util_get_timestamp ();

  if (0 == stats->_start) {
    stats->min = GST_CLOCK_TIME_NONE;
    stats->_start = stats->_tic;
  }
}

GstClockTime stats_toc (Stats * stats)
{
  g_return_val_if_fail (stats, GST_CLOCK_TIME_NONE);
  
  stats->_toc = gst_util_get_timestamp ();

  return do_stats (stats);
}

static GstClockTime do_stats (Stats * stats)
{
  GstClockTime measurement;
  
  g_return_val_if_fail (stats, GST_CLOCK_TIME_NONE);

  measurement = GST_CLOCK_DIFF(stats->_tic, stats->_toc);

  if (measurement > stats->max) {
    stats->max = measurement;
  } else if (measurement < stats->min) {
    stats->min = measurement;
  }

  /* Average computation using accumulator */
  stats->average = (stats->average*stats->count + measurement)/(stats->count+1.0);
  stats->count++;

  return stats->average;
}
