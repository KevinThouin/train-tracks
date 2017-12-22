#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <math.h>

#include <pthread.h>

#include "train_tracks_app_window.h"
#include "train_tracks_error.h"
#include "c_cpp_bridge.h"
#include "worker_thread.h"
#include "color.h"

enum {
	X_AXIS,
	Y_AXIS,
	Z_AXIS,
	N_AXES
};

struct _TrainTracksAppWindow
{
	GtkApplicationWindow parent_instance;
	
	/* GL rendering widget */
	GtkWidget *gl_drawing_area;
	
	GtkWidget *play_button;
	gint64 last_frame_time;
	
	/* Mouse data */
	gboolean mouse_pressed;
	gdouble mouse_x;
	gdouble mouse_y;
	
	/* Speed of animations */
	float speed;
	
	/* Tick callback ID*/
	gint tick_callback_id;
};

struct _TrainTracksAppWindowClass
{
	GtkApplicationWindowClass parent_class;
};

G_DEFINE_TYPE (TrainTracksAppWindow, train_tracks_app_window, GTK_TYPE_APPLICATION_WINDOW)

int glInited = 0;

static gboolean tick_callback(GtkWidget* widget, GdkFrameClock* frameClock, gpointer data) {
	TrainTracksAppWindow* app_window = TRAIN_TRACKS_APP_WINDOW(data);
	gtk_gl_area_queue_render(GTK_GL_AREA(app_window->gl_drawing_area));
	return G_SOURCE_CONTINUE;
}

static void gl_init (TrainTracksAppWindow *self)
{
	char *title;
	const char *renderer;
	
	/* we need to ensure that the GdkGLContext is set before calling GL API */
	gtk_gl_area_make_current (GTK_GL_AREA (self->gl_drawing_area));
	
	/* if the GtkGLArea is in an error state we don't do anything */
	if (gtk_gl_area_get_error (GTK_GL_AREA (self->gl_drawing_area)) != NULL)
		return;
	
	/* initialize glew */
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		printf("Failed to initialise glew\n");
		return;
	}
	
	glLineWidth(1.0);
	glEnable(GL_LINE_SMOOTH);
	char* error_msg;
	if (init_C(&error_msg)) {
		GError* error = NULL;
		fprintf(stderr, error_msg);
		g_set_error (&error, TRAIN_TRACKS_ERROR, TRAIN_TRACKS_ERROR_SHADER, error_msg);
		gtk_gl_area_set_error (GTK_GL_AREA (self->gl_drawing_area), error);
		free(error_msg);
	}
	
	/* set the window title */
	renderer = (char *) glGetString (GL_RENDERER);
	title = g_strdup_printf ("Train tracks on %s", renderer ? renderer : "Unknown");
	gtk_window_set_title (GTK_WINDOW (self), title);
	g_free (title);
	
	initWorker();
	self->tick_callback_id = gtk_widget_add_tick_callback(self->gl_drawing_area, tick_callback, self, NULL);
	self->last_frame_time = gdk_frame_clock_get_frame_time(gtk_widget_get_frame_clock(self->gl_drawing_area));
	glInited = 1;
}

static void gl_fini (TrainTracksAppWindow *self)
{
	/* we need to ensure that the GdkGLContext is set before calling GL API */
	gtk_gl_area_make_current (GTK_GL_AREA (self->gl_drawing_area));
	
	gtk_widget_remove_tick_callback(self->gl_drawing_area, self->tick_callback_id);
	stopWorker();
	
	/* skip everything if we're in error state */
	if (gtk_gl_area_get_error (GTK_GL_AREA (self->gl_drawing_area)) != NULL)
		return;
	
	/* Destroy GL resources */
	fini_C();
}

static gboolean gl_draw (TrainTracksAppWindow *self)
{
	/* clear the viewport; the viewport is automatically resized when
	* the GtkGLArea gets a new size allocation
	*/
	glClearColor (backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0);
	glClear (GL_COLOR_BUFFER_BIT);
	
	gint64 frame_time = gdk_frame_clock_get_frame_time(gtk_widget_get_frame_clock(self->gl_drawing_area));
	
	/* draw */
	render_C(self->speed*self->speed * (frame_time-self->last_frame_time)/50000000.0);
	
	/* flush the contents of the pipeline */
	glFlush ();
	self->last_frame_time = frame_time;
	
	return TRUE;
}

static void gl_resize(TrainTracksAppWindow* self, gint width, gint height) {
	resize_C(width, height);
}

static gboolean mouse_scroll_evt(TrainTracksAppWindow* self, GdkEvent* evt) {
	gdouble delta = evt->scroll.delta_y;
	mouse_scroll_C(delta);
	gtk_gl_area_queue_render (GTK_GL_AREA(self->gl_drawing_area));
	return TRUE;
}

static gboolean button_evt(TrainTracksAppWindow* self, GdkEvent* evt) {
	if (evt->button.button == 1) {
		if (evt->button.type == GDK_BUTTON_PRESS) {
			self->mouse_pressed = TRUE;
			self->mouse_x = evt->button.x;
			self->mouse_y = evt->button.y;
			return TRUE;
		}
		else if (evt->button.type == GDK_BUTTON_RELEASE) {
			self->mouse_pressed = FALSE;
			return TRUE;
		}
	}
	return FALSE;
}

static gboolean motion_evt(TrainTracksAppWindow* self, GdkEvent* evt) {
	if (self->mouse_pressed == TRUE) {
		gdouble dx = evt->motion.x - self->mouse_x;
		gdouble dy = self->mouse_y - evt->motion.y;
		self->mouse_x = evt->motion.x;
		self->mouse_y = evt->motion.y;
		mouse_motion_C(dx, dy);
		gtk_gl_area_queue_render (GTK_GL_AREA(self->gl_drawing_area));
		return TRUE;
	}
	return FALSE;
}

static void speed_changed (TrainTracksAppWindow *self,
                    GtkAdjustment   *adj)
{
	float value = gtk_adjustment_get_value (adj);
	self->speed = value;
}

static void play_toggled (TrainTracksAppWindow *self)
{
	gboolean toggled = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (self->play_button));
	
	if (toggled)
	{
		postOnPlay();
	}
	else
	{
		
	}
}

static void train_tracks_app_window_class_init (TrainTracksAppWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	
	gtk_widget_class_set_template_from_resource (widget_class, "/ca/usherbrooke/math/lwatson/train_tracks/train_tracks_app_window.ui");
	
	gtk_widget_class_bind_template_child (widget_class, TrainTracksAppWindow, gl_drawing_area);
	gtk_widget_class_bind_template_child (widget_class, TrainTracksAppWindow, play_button);
	
	gtk_widget_class_bind_template_callback (widget_class, speed_changed);
	gtk_widget_class_bind_template_callback (widget_class, play_toggled);
	gtk_widget_class_bind_template_callback (widget_class, gl_init);
	gtk_widget_class_bind_template_callback (widget_class, gl_draw);
	gtk_widget_class_bind_template_callback (widget_class, gl_resize);
	gtk_widget_class_bind_template_callback (widget_class, gl_fini);
	gtk_widget_class_bind_template_callback (widget_class, mouse_scroll_evt);
	gtk_widget_class_bind_template_callback (widget_class, button_evt);
	gtk_widget_class_bind_template_callback (widget_class, motion_evt);
}

static void train_tracks_app_window_init (TrainTracksAppWindow *self)
{
	gtk_widget_init_template (GTK_WIDGET (self));
	gtk_window_set_icon_name (GTK_WINDOW (self), "train_tracks");
	self->mouse_pressed = FALSE;
	self->speed = 55;
}

GtkWidget* train_tracks_app_window_new (TrainTracksApp *app)
{
	return g_object_new (train_tracks_app_window_get_type (), "application", app, NULL);
}
