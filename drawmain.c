#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <math.h>
#include "surfacepaint.h"
#include "neural.h"

Surface * screen; /* the global bitmap */
Brush brush; /* global brush */

double lastx, lasty;
guint32 lastt = 0;
double lastpaintx, lastpainty, lastpaintpress;
double dist;
double spacing = 0.2;
double vx10;

GtkWidget *statusline;
/*
GtkCheckMenuItem *recorddata;
GtkCheckMenuItem *suggestsize;
*/

static gint
my_button_press (GtkWidget *widget, GdkEventButton *event)
{
  /*
  double pressure;
  gdk_event_get_axis ((GdkEvent *)event, GDK_AXIS_PRESSURE, &pressure);
  g_print ("button press %f %f %f\n", event->x, event->y, pressure);
  return TRUE;
  */
  return FALSE;
}

static gint
my_motion (GtkWidget *widget, GdkEventMotion *event)
{
  double pressure;
  double dx, dy, dt;
  double d_dist;

  gdk_event_get_axis ((GdkEvent *)event, GDK_AXIS_PRESSURE, &pressure);
  /* g_print ("motion %f %f %f %d\n", event->x, event->y, pressure, event->state); */

  /* no button pressed */
  /* don't care, always paint - FIXME: breaks mouse support (always pressure set)
  if (!(event->state & 256))
    return TRUE;
  */

  dx = event->x - lastx;
  dy = event->y - lasty;
  dt = (double)(event->time - lastt) / 1000.0; /* in seconds */

  d_dist = sqrt(dx*dx + dy*dy);
  dist += d_dist;

  neural_process_movement (dt, event->x, event->y, pressure, d_dist);
  brush.radius = neural_get_suggested_brushsize ();
  if (brush.radius < 1.2) {
    brush.radius = 1.2;
    neural_set_current_brushsize (brush.radius);
  }

  while (dist >= spacing*brush.radius)
    {
      /* interpolate position and presssure (a bit wrong probably, but works great so far) */
      lastpaintx = lastpaintx + (event->x - lastpaintx) * spacing*brush.radius / dist;
      lastpainty = lastpainty + (event->y - lastpainty) * spacing*brush.radius / dist;
      lastpaintpress = lastpaintpress + (pressure - lastpaintpress) * spacing*brush.radius / dist;
      dist -= spacing*brush.radius;

      brush.opaque = lastpaintpress / 8.0;
      surface_draw (screen,
                    lastpaintx,
                    lastpainty,
                    &brush);

      gtk_widget_queue_draw_area (widget, 
                                  floor(lastpaintx-(brush.radius+1)),
                                  floor(lastpainty-(brush.radius+1)),
                                  /* FIXME: think about it exactly */
                                  ceil (2*(brush.radius+1)),
                                  ceil (2*(brush.radius+1))
                                  );
    }

  lastx = event->x;
  lasty = event->y;
  lastt = event->time;

  return TRUE;
}

static gint
my_expose (GtkWidget *widget, GdkEventExpose *event, Surface *s)
{
  byte *rgb;
  int rowstride;

  rowstride = event->area.width * 3;
  rowstride = (rowstride + 3) & -4; /* align to 4-byte boundary */
  rgb = g_new (byte, event->area.height * rowstride);

  surface_render (s,
                  rgb, rowstride,
                  event->area.x, event->area.y,
                  event->area.width, event->area.height);

  gdk_draw_rgb_image (widget->window,
		      widget->style->black_gc,
		      event->area.x, event->area.y,
		      event->area.width, event->area.height,
		      GDK_RGB_DITHER_MAX,
		      rgb,
		      rowstride);

  g_free (rgb);
  return FALSE;
}


static void
init_input (void)
{
  GList *tmp_list;
  GdkDevice *info;

  tmp_list = gdk_devices_list();

  info = NULL;
  while (tmp_list)
    {
      info = (GdkDevice *)tmp_list->data;
      /*
      g_print ("device: %s\n", info->name);
      */
      if (!g_strcasecmp (info->name, "wacom") ||
	  !g_strcasecmp (info->name, "stylus") ||
	  !g_strcasecmp (info->name, "eraser"))
	  {
	    gdk_device_set_mode (info, GDK_MODE_SCREEN);
	  }
      tmp_list = tmp_list->next;
    }
  if (!info) return;
}

static void
brush_bigger (GtkAction *action, GtkWidget *window)
{
  brush.radius *= 1.4;
  neural_set_current_brushsize (brush.radius);
}

static void
brush_smaller (GtkAction *action, GtkWidget *window)
{
  brush.radius /= 1.4;
  neural_set_current_brushsize (brush.radius);
}

static void
invert_colors (GtkAction *action, GtkWidget *window)
{
  brush.color[0] = 255 - brush.color[0];
  brush.color[1] = 255 - brush.color[1];
  brush.color[2] = 255 - brush.color[2];
}

static void
clear_image (GtkAction *action, GtkWidget *window)
{
  surface_clear (screen);
  gtk_widget_draw (window, NULL);
  neural_notice_clear_image ();
}

static void
train_nn (GtkAction *action, GtkWidget *window)
{
  neural_train ();
}

static GtkActionEntry my_actions[] = {
  { "ImageMenu", NULL, "Image" },
  { "BrushMenu", NULL, "Brush" },
  { "LearnMenu", NULL, "Learn" },
  { "ClearImage", NULL, "Clear", "<control>E", "Clear the image", G_CALLBACK (clear_image) },
  /*  { "Quit", NULL, "Quit", "<control>Q", "XXXX", G_CALLBACK (quit) }, */
  { "BrushBigger", NULL, "Bigger", "F", NULL, G_CALLBACK (brush_bigger) },
  { "BrushSmaller", NULL, "Smaller", "D", NULL, G_CALLBACK (brush_smaller) },
  { "InvertColor", NULL, "Invert Color", "X", NULL, G_CALLBACK (invert_colors) },
  { "TrainNN", NULL, "Train", "<control>T", NULL, G_CALLBACK (train_nn) },
};

/*
static GtkToggleActionEntry my_toggle_actions[] = {
  { "RecordData", NULL, "Record Data", "F11", "Record your inputs for later training", G_CALLBACK (nothing), TRUE },
  { "SuggestSize", NULL, "Suggest Size", "F12", "Foobar", G_CALLBACK (nothing), FALSE },
};
*/

static const char * ui_description = 
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='ImageMenu'>"
"      <menuitem action='ClearImage' />"
"      <menuitem action='Quit' />"
"    </menu>"
"    <menu action='BrushMenu'>"
"      <menuitem action='BrushBigger' />"
"      <menuitem action='BrushSmaller' />"
"      <menuitem action='InvertColor' />"
"    </menu>"
"    <menu action='LearnMenu'>"
/*
"      <menuitem action='RecordData' />"
"      <menuitem action='SuggestSize' />"
*/
"      <menuitem action='TrainNN' />"
"    </menu>"
"  </menubar>"
"</ui>"
;

int
main (int argc, char **argv)
{
  GtkWidget *w;
  GtkWidget *v;
  GtkWidget *eb;
  GtkWidget *da;
  int xs = SIZE;
  int ys = SIZE;

  gtk_init (&argc, &argv);

  if (argc >= 3)
    {
      xs = atoi (argv[1]);
      ys = atoi (argv[2]);
      if (xs == 0) xs = SIZE;
      if (ys == 0) ys = SIZE;
    }

  init_input ();
  neural_init ();

  gdk_rgb_init ();

  gtk_widget_set_default_colormap (gdk_rgb_get_cmap ());
  gtk_widget_set_default_visual (gdk_rgb_get_visual ());

  screen = new_surface (xs, ys);

  surface_clear (screen);
  neural_notice_clear_image ();

  brush.radius = 8.0;
  brush.color[0] = 0;
  brush.color[1] = 0;
  brush.color[2] = 0;


  w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect (GTK_OBJECT (w), "destroy",
		      (GtkSignalFunc) gtk_main_quit, NULL);

  v = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (w), v);
  gtk_widget_show (v);

  { /* Menu */
    GtkUIManager *uim;
    GError * error;
    GtkAccelGroup *accel_group;
    GtkActionGroup *action_group;
    GtkWidget *menu_bar;

    uim = gtk_ui_manager_new ();
    error = NULL;
    gtk_ui_manager_add_ui_from_string (uim, ui_description, -1, &error);
    if (error) 
      {
        g_print ("ERROR: %s\n", error->message);
        g_clear_error (&error);
        exit (1);
      }

    action_group = gtk_action_group_new ("myactiongroup");
    gtk_action_group_add_actions (action_group, my_actions, G_N_ELEMENTS (my_actions), w);
    /*
    gtk_action_group_add_toggle_actions (action_group, my_toggle_actions, G_N_ELEMENTS (my_toggle_actions), w);
    */
    gtk_ui_manager_insert_action_group (uim, action_group, 0);
    accel_group = gtk_ui_manager_get_accel_group (uim);
    gtk_window_add_accel_group (GTK_WINDOW (w), accel_group);

    menu_bar = gtk_ui_manager_get_widget (uim, "/MainMenu");
    /*
    recorddata = GTK_CHECK_MENU_ITEM (gtk_ui_manager_get_widget (uim, "/MainMenu/LearnMenu/RecordData"));
    suggestsize = GTK_CHECK_MENU_ITEM (gtk_ui_manager_get_widget (uim, "/MainMenu/LearnMenu/SuggestSize"));
    */
    gtk_container_add (GTK_CONTAINER (v), menu_bar);
    gtk_widget_show (menu_bar);

    gtk_accel_map_load ("accelmap.conf");
  }

  eb = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (v), eb);

  gtk_widget_set_extension_events (eb, GDK_EXTENSION_EVENTS_ALL);

  gtk_widget_set_events (eb, GDK_EXPOSURE_MASK
			 | GDK_LEAVE_NOTIFY_MASK
			 | GDK_BUTTON_PRESS_MASK
			 | GDK_KEY_PRESS_MASK
			 | GDK_POINTER_MOTION_MASK
			 | GDK_PROXIMITY_OUT_MASK);

  GTK_WIDGET_SET_FLAGS(eb, GTK_CAN_FOCUS); /* for key events */

  gtk_signal_connect (GTK_OBJECT (eb), "button_press_event",
		      (GtkSignalFunc) my_button_press, NULL);
  gtk_signal_connect (GTK_OBJECT (eb), "motion_notify_event",
		      (GtkSignalFunc) my_motion, NULL);

  da = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (da), xs, ys);
  gtk_container_add (GTK_CONTAINER (eb), da);

  gtk_signal_connect (GTK_OBJECT (da), "expose_event",
		      (GtkSignalFunc) my_expose, screen);

  statusline = gtk_label_new ("hello world");
  gtk_container_add (GTK_CONTAINER (v), statusline);

  gtk_widget_show_all (w);

  /*
  gtk_timeout_add (50, (GtkFunction) dry_timer, da);
  */

  gtk_main ();

  gtk_accel_map_save ("accelmap.conf");
  neural_finish ();

  return 0;
}
