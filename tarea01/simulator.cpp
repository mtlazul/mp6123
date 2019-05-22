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

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>
#include <signal.h>
#include <stdlib.h>

#include "prototype.h"
#include "statistics.h"

/* Function declaration */
static void signal_handler(gint signal);
static GstElement *build_pipeline(const gchar *description);
static void install_buffer_callback(GstAppSink *appsink);
static GstFlowReturn buffer_callback(GstAppSink *appsink, gpointer user_data);
static gchar *bayer_factory(const gchar *filename);
static gchar *rgb_factory(const gchar *filename);

enum Algorithm
{
  NEAREST_NEIGHBOR,
  BILINEAR
};

typedef struct _Context Context;
struct _Context
{
  gchar *input;
  gchar *output;
  gboolean nn;
  gboolean bilinear;
  Algorithm algo;
};

/* Module wide constants */
#define WIDTH "1920"
#define HEIGHT "1080"
#define BAYER_CAPS "video/x-bayer,format=rggb"
#define RGB_CAPS "video/x-raw,format=RGB"
#define APPSINK "appsink drop=false max-buffers=1 name=bayersink async=false"
#define APPSRC "appsrc format=time name=rgbsrc"

/* Global variables */
static GMainLoop *loop = NULL;

static void
signal_handler(gint signal)
{
  if (loop)
  {
    g_print("Signal received, shutting down...\n");
    g_main_loop_quit(loop);
  }
}

static gchar *
bayer_factory(const gchar *filename)
{
  gchar *desc;

  if (filename)
  {
    g_print("Reading input from %s\n", filename);
    desc = g_strdup_printf("filesrc location=%s ! decodebin ! videoconvert ! "
                           "queue ! video/x-raw,format=ARGB ! rgb2bayer ! " BAYER_CAPS " ! " APPSINK,
                           filename);
  }
  else
  {
    g_print("Generating input from pattern\n");
    desc = g_strdup_printf("videotestsrc is-live=true ! queue ! " BAYER_CAPS ",width=" WIDTH ",height=" HEIGHT " ! " APPSINK);
  }

  return desc;
}

static gchar *
rgb_factory(const gchar *filename)
{
  gchar *desc;

  if (filename)
  {
    g_print("Writing output to %s\n", filename);
    desc = g_strdup_printf(APPSRC " ! videoconvert ! pngenc ! multifilesink next-file=buffer location=%s", filename);
  }
  else
  {
    g_print("Displaying output to window\n");
    desc = g_strdup_printf(APPSRC " ! videoconvert ! autovideosink sync=false async=false");
  }

  return desc;
}

static GstElement *
build_pipeline(const gchar *description)
{
  GstElement *pipeline;
  GError *error;

  g_return_val_if_fail(description, NULL);

  error = NULL;
  pipeline = gst_parse_launch(description, &error);
  if (error)
  {
    g_printerr("An error ocurred while building the pipeline: %d: %s\n", error->code, error->message);
    g_error_free(error);
  }

  return pipeline;
}

static GstFlowReturn
buffer_callback(GstAppSink *appsink, gpointer user_data)
{
  GstAppSrc *appsrc;
  GstSample *sample;
  GstBuffer *buffer;
  GstBuffer *outbuffer;
  GstCaps *caps;
  guint8 *dst;
  guint8 *src;
  gpointer *cb_data;
  Stats *stats;
  GstVideoInfo videoinfo;
  GstMapInfo srcinfo;
  GstMapInfo dstinfo;
  guint width;
  guint height;
  gsize rgbsize;
  GstCaps *appsrc_caps;
  gchar *appsrc_scaps;
  Algorithm algo;

  g_return_val_if_fail(appsink, GST_FLOW_ERROR);
  g_return_val_if_fail(user_data, GST_FLOW_ERROR);

  /* Recover callback data */
  cb_data = (gpointer *)user_data;
  appsrc = GST_APP_SRC(cb_data[0]);

  stats = (Stats *)cb_data[1];
  algo = *(Algorithm *)cb_data[2];

  sample = gst_app_sink_pull_sample(appsink);
  buffer = gst_sample_get_buffer(sample);

  /* Parse media information */
  caps = gst_sample_get_caps(sample);
  gst_video_info_from_caps(&videoinfo, caps);
  width = videoinfo.width;
  height = videoinfo.height;
  rgbsize = width * height * 3;

  /* Create output buffer */
  outbuffer = gst_buffer_new_allocate(NULL, rgbsize, NULL);
  gst_buffer_map(outbuffer, &dstinfo, GST_MAP_WRITE);
  dst = dstinfo.data;

  /* Pull input data */
  gst_buffer_map(buffer, &srcinfo, GST_MAP_READ);
  src = srcinfo.data;

  stats_tic(stats);
  if (NEAREST_NEIGHBOR == algo)
  {
    nearest_neighbor_process(dst, src, width, height);
  }
  else
  {
    bilinear_process(dst, src, width, height);
  }
  stats_toc(stats);

  gst_buffer_unmap(buffer, &srcinfo);
  gst_buffer_unmap(buffer, &dstinfo);

  gst_sample_unref(sample);

  /* Set the caps in the other pipeline if not set already */
  appsrc_caps = gst_app_src_get_caps(appsrc);
  if (!appsrc_caps)
  {
    appsrc_scaps = g_strdup_printf(RGB_CAPS ",width=%d,height=%d", width, height);
    appsrc_caps = gst_caps_from_string(appsrc_scaps);
    gst_app_src_set_caps(appsrc, appsrc_caps);
    g_free(appsrc_scaps);
  }
  gst_caps_unref(appsrc_caps);

  /* Push the buffer into the RGB pipeline */
  return gst_app_src_push_buffer(appsrc, outbuffer);
}

static void
install_buffer_callback(GstAppSink *appsink, GstAppSrc *appsrc, Stats *stats, Algorithm *algo)
{
  GstAppSinkCallbacks cb;
  static gpointer cb_data[3];

  g_return_if_fail(appsink);
  g_return_if_fail(appsrc);
  g_return_if_fail(stats);

  cb.eos = NULL;
  cb.new_preroll = NULL;
  cb.new_sample = buffer_callback;

  /* Prepare a typeless array to pass custom data to callback */
  cb_data[0] = appsrc;
  cb_data[1] = stats;
  cb_data[2] = algo;

  gst_app_sink_set_callbacks(appsink, &cb, cb_data, NULL);
}

Context parse_cmdline(gint *argc, gchar **argv[])
{
  GError *error = NULL;
  GOptionContext *context;
  Context ctx = {0};
  GOptionEntry entries[] =
      {
          {"input", 'i', 0, G_OPTION_ARG_FILENAME, &ctx.input, "Read input from image I", "I"},
          {"output", 'o', 0, G_OPTION_ARG_FILENAME, &ctx.output, "Write output to PNG image O", "O"},
          {"nearest", 'n', 0, G_OPTION_ARG_NONE, &ctx.nn, "Use nearest neighbor algorithm (default)", NULL},
          {"bilinear", 'b', 0, G_OPTION_ARG_NONE, &ctx.bilinear, "Use bilinear algorithm", NULL},
          {NULL}};

  context = g_option_context_new("- Bayer to RGB coupling module");
  g_option_context_add_main_entries(context, entries, NULL);
  g_option_context_add_group(context, gst_init_get_option_group());

  if (!g_option_context_parse(context, argc, argv, &error))
  {
    g_printerr("Option parsing failed: %s\n", error->message);
    g_error_free(error);
    exit(1);
  }

  if (ctx.nn && ctx.bilinear)
  {
    g_printerr("Only one interpolation algorithm can be used\n");
    exit(1);
  }

  if (!ctx.nn && !ctx.bilinear)
  {
    g_print("No interpolation method specified, defaulting to Nearest Neighbor\n");
    ctx.nn = TRUE;
  }

  if (ctx.nn)
  {
    ctx.algo = NEAREST_NEIGHBOR;
  }
  else
  {
    ctx.algo = BILINEAR;
  }

  return ctx;
}

gint main(gint argc, gchar *argv[])
{
  GstElement *bayer_pipe;
  GstElement *rgb_pipe;
  GstAppSink *bayer_sink;
  GstAppSrc *rgb_src;
  Stats stats = STATS_NEW;
  gint ret;
  Context ctx;
  gchar *bayer_desc;
  gchar *rgb_desc;

  gst_init(&argc, &argv);

  ctx = parse_cmdline(&argc, &argv);

  /* Exit on CTRL+C */
  signal(SIGINT, signal_handler);

  bayer_desc = bayer_factory(ctx.input);

  bayer_pipe = build_pipeline(bayer_desc);
  g_free(bayer_desc);
  if (!bayer_pipe)
  {
    g_printerr("Unrecoverable error building Bayer pipe... closing.\n");
    ret = EXIT_FAILURE;
    goto bayer_error;
  }

  bayer_sink = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(bayer_pipe), "bayersink"));

  rgb_desc = rgb_factory(ctx.output);

  rgb_pipe = build_pipeline(rgb_desc);
  g_free(rgb_desc);
  if (!rgb_pipe)
  {
    g_printerr("Unrecoverable error building RGB pipe... closing.\n");
    ret = EXIT_FAILURE;
    goto rgb_error;
  }

  rgb_src = GST_APP_SRC(gst_bin_get_by_name(GST_BIN(rgb_pipe), "rgbsrc"));

  /* Install the callback that notifies us when a new Bayer buffer is
     available so we can turn it into an RGB buffer*/
  install_buffer_callback(bayer_sink, rgb_src, &stats, &ctx.algo);

  if (GST_STATE_CHANGE_FAILURE == gst_element_set_state(rgb_pipe, GST_STATE_PLAYING))
  {
    g_printerr("Unable to play RGB pipe... closing.\n");
    ret = EXIT_FAILURE;
    goto rgb_play_error;
  }

  if (GST_STATE_CHANGE_FAILURE == gst_element_set_state(bayer_pipe, GST_STATE_PLAYING))
  {
    g_printerr("Unable to play Bayer pipe... closing.\n");
    ret = EXIT_FAILURE;
    goto bayer_play_error;
  }

  g_print("Starting main loop, press CTRL+C to quit\n");
  loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  stats_print(&stats);

  ret = EXIT_SUCCESS;

  /* Start Tear Down */
  gst_element_set_state(bayer_pipe, GST_STATE_NULL);

bayer_play_error:
{
  gst_element_set_state(bayer_pipe, GST_STATE_NULL);
}
rgb_play_error:
{
  gst_object_unref(rgb_src);
  gst_object_unref(rgb_pipe);
}
rgb_error:
{
  gst_object_unref(bayer_sink);
  gst_object_unref(bayer_pipe);
}
bayer_error:
{
  g_print("Bye!\n");
  return ret;
}
}
