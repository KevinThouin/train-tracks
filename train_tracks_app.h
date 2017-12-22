#ifndef __TRAIN_TRACKS_APP_H__
#define __TRAIN_TRACKS_APP_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TRAIN_TRACKS_TYPE_APP (train_tracks_app_get_type ())

G_DECLARE_FINAL_TYPE (TrainTracksApp, train_tracks_app, TRAIN_TRACKS, APP, GtkApplication)

GtkApplication *train_tracks_app_new (void);

G_END_DECLS

#endif /* __TRAIN_TRACKS_APP_H__ */
