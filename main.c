#include <gtk/gtk.h>

#include "train_tracks_app.h"

int main (int argc, char *argv[])
{
	return g_application_run (G_APPLICATION (train_tracks_app_new ()), argc, argv);
}
