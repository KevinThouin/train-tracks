#ifndef __TRAIN_TRACKS_APP_WINDOW_H__
#define __TRAIN_TRACKS_APP_WINDOW_H__

#include <gtk/gtk.h>
#include "train_tracks_app.h"

G_BEGIN_DECLS

#define TRAIN_TRACKS_TYPE_APP_WINDOW (train_tracks_app_window_get_type ())

G_DECLARE_FINAL_TYPE (TrainTracksAppWindow, train_tracks_app_window, TRAIN_TRACKS, APP_WINDOW, GtkApplicationWindow)

GtkWidget *train_tracks_app_window_new (TrainTracksApp *app);

extern int glInited;

G_END_DECLS

#endif /* __TRAIN_TRACKS_APP_WINDOW_H__ */
