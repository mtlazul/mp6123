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

#ifndef __PROTOTYPE_H__
#define __PROTOTYPE_H__

#include <gst/gst.h>

void nearest_neighbor_process (guint8 * dst, const guint8 * const restrict src,
    guint width, guint height);

void bilinear_process (guint8 * dst, const guint8 * const restrict src,
    guint width, guint height);

#endif //__PROTOTYPE_H__
