#include <stdlib.h>
#include <stdio.h>

#include "train_tracks_app.h"
#include "train_tracks_app_window.h"
#include "worker_thread.h"
#include "c_cpp_bridge.h"

struct _TrainTracksApp
{
	GtkApplication parent_instance;
	
	GtkWidget *window;
};

struct _TrainTracksAppClass
{
	GtkApplicationClass parent_class;
};

G_DEFINE_TYPE (TrainTracksApp, train_tracks_app, GTK_TYPE_APPLICATION)

static void quit_activated (GSimpleAction* action, GVariant* parameter, gpointer app)
{
	g_application_quit (G_APPLICATION (app));
	g_object_unref(G_OBJECT(TRAIN_TRACKS_APP(app)->window));
}

static void open_activated(GSimpleAction* act, GVariant* parameter, gpointer app) {
	char* filename = NULL;
	TrainTracksApp* train_app = TRAIN_TRACKS_APP(app);
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	GtkWidget* dialog = gtk_file_chooser_dialog_new ("Open File", GTK_WINDOW(train_app->window), action, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);
	
	gint res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	}
	gtk_widget_destroy(dialog);
	
	if (filename!=NULL) {
		loadMatrix(filename);
		g_free(filename);
	}
}

static GActionEntry app_entries[] =
{
	{"quit", quit_activated, NULL, NULL, NULL},
	{"open", open_activated, NULL, NULL, NULL}
};

static void train_tracks_app_startup (GApplication *app)
{
	GtkBuilder *builder;
	GMenuModel *app_menu;

	G_APPLICATION_CLASS (train_tracks_app_parent_class)->startup (app);

	g_action_map_add_action_entries (G_ACTION_MAP (app),
									app_entries, G_N_ELEMENTS (app_entries),
									app);

	builder = gtk_builder_new_from_resource ("/ca/usherbrooke/math/lwatson/train_tracks/train_tracks_app_menu.ui");
	app_menu = G_MENU_MODEL (gtk_builder_get_object (builder, "appmenu"));
	gtk_application_set_app_menu (GTK_APPLICATION (app), app_menu);
	g_object_unref (builder);
}

static void train_tracks_app_activate (GApplication *app)
{
	TrainTracksApp *self = TRAIN_TRACKS_APP (app);
	
	if (self->window == NULL) {
		self->window = train_tracks_app_window_new (TRAIN_TRACKS_APP (app));
	}

	gtk_window_present (GTK_WINDOW (self->window));
}

static void train_tracks_app_class_init (TrainTracksAppClass *klass)
{
	GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

	app_class->startup  = train_tracks_app_startup;
	app_class->activate = train_tracks_app_activate;
}

static void train_tracks_app_init (TrainTracksApp *self) {}

GtkApplication * train_tracks_app_new (void)
{
	return g_object_new (train_tracks_app_get_type (), "application-id", "ca.usherbrooke.math.lwatson.train_tracks", NULL);
}
