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

#include "prototype.h"

#include <string.h>

void
nearest_neighbor_process (guint8 * restrict dst, const guint8  * restrict const src,
    const guint width, const guint height)
{
  // Replace this code with your actual processing
  const gsize size = width*height;
  memcpy (dst, src, size);
}

void
bilinear_process (guint8 * restrict dst, const guint8  * restrict const src,
    const guint width, const guint height)
{
  // Replace this code with your actual processing
  const gsize size = width*height*3;
  memset (dst, 0xFF, size);
}
