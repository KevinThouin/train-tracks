#ifndef __TRAIN_TRACKS_APP_WINDOW_HPP__
#define __TRAIN_TRACKS_APP_WINDOW_HPP__

#include <glibmm/timer.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/glarea.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/builder.h>
#include <gdkmm/event.h>

#include "prog_gl.hpp"
#include "draw_gl.hpp"

class TrainTracksApp;

extern bool glInited;

class TrainTracksAppWindow final : public Gtk::ApplicationWindow {
	ProgramsGL m_programsGL;
	RendererGL m_rendererGL;
	const TrainTracksApp&			m_app;
	Gtk::GLArea*					m_glDrawingArea;
	Gtk::ToggleButton*				m_playButton;
	Glib::RefPtr<Gtk::Adjustment>	m_speedAdjustment;
	sigc::connection				m_drawConnection;
	Glib::Timer					  	m_timer;
	double							m_lastFrameTime;
	double							m_mouseX;
	double							m_mouseY;
	float							m_speed;
	int								m_tickCallbackId;
	bool							m_mousePressed;
	
	
public:
	// A etre appeler a l'interne de la bibliotheque gtkmm seulement!
	TrainTracksAppWindow(BaseObjectType* cobj, const Glib::RefPtr<Gtk::Builder>& builder, const TrainTracksApp& app);
	~TrainTracksAppWindow();
	static Glib::RefPtr<TrainTracksAppWindow> create(Glib::RefPtr<Gtk::Builder>& builder, const TrainTracksApp& app);
	
private:
	void glInit();
	void glFini();
	bool glDraw(const Glib::RefPtr<Gdk::GLContext>& context);
	void glResize(int width, int height);
	bool mouseScrollEvt(GdkEventScroll* evt);
	bool buttonEvt(GdkEventButton* evt);
	bool motionEvt(GdkEventMotion* evt);
	bool on_drag_motion(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time) override;
	void speedChanged();
	void playToggled();
	bool onTimeout();
	
	void updateTargetFrameRate();
};

#endif // __TRAIN_TRACKS_APP_WINDOW_HPP__
