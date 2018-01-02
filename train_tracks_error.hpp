#ifndef __TRAIN_TRACKS_ERROR_HPP__
#define __TRAIN_TRACKS_ERROR_HPP__

#include <glibmm/quark.h>

extern const Glib::Quark train_tracks_error_quark;

namespace train_tracks_errors {
	enum type {
		TRAIN_TRACKS_ERROR_SHADER
	};
}

#endif // __TRAIN_TRACKS_ERROR_HPP__
