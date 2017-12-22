#ifndef __TRAIN_TRACKS_ERROR_H__
#define __TRAIN_TRACKS_ERROR_H__

#include <glib.h>

G_BEGIN_DECLS

#define TRAIN_TRACKS_ERROR (train_tracks_error_quark ())

typedef enum {
	TRAIN_TRACKS_ERROR_SHADER
} TrainTracksError;

GQuark train_tracks_error_quark (void);

G_END_DECLS

#endif /* __TRAIN_TRACKS_ERROR_H__ */
