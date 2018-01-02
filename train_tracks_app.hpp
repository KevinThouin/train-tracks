#ifndef __TRAIN_TRACKS_APP_HPP__
#define __TRAIN_TRACKS_APP_HPP__

#include <gtkmm/application.h>

#include "train_tracks_app_window.hpp"

class TrainTracksApp final : public Gtk::Application {
	Glib::RefPtr<TrainTracksApp>& m_self;
	Glib::RefPtr<TrainTracksAppWindow> m_window;
	
public:
	static Glib::RefPtr<TrainTracksApp> create(Glib::RefPtr<TrainTracksApp>& self);
	Glib::RefPtr<TrainTracksAppWindow>& getWindow() {return m_window;}
	const Glib::RefPtr<TrainTracksAppWindow>& getWindow() const {return m_window;}
	
private:
	TrainTracksApp(Glib::RefPtr<TrainTracksApp>& self);
	
	virtual void on_activate() override;
	virtual void on_startup() override;
	void quitActivated();
	void openActivated();
};

#endif // __TRAIN_TRACKS_APP_HPP__
