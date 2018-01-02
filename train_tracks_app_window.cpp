#include <cmath>
#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GL/gl.h>
#include <glibmm/ustring.h>
#include <glibmm/main.h>
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>

#include "train_tracks_app_window.hpp"
#include "train_tracks_app.hpp"
#include "train_tracks_error.hpp"
#include "worker_thread.hpp"
#include "display_cmd.hpp"
#include "color.hpp"
#include "util.hpp"

bool glInited = false;

Glib::RefPtr<TrainTracksAppWindow> TrainTracksAppWindow::create(Glib::RefPtr<Gtk::Builder>& builder, const TrainTracksApp& app) {
	TrainTracksAppWindow* ptr_obj;
	builder->get_widget_derived("train_tracks_app_window", ptr_obj, app);
	return Glib::RefPtr<TrainTracksAppWindow>(ptr_obj);
}

TrainTracksAppWindow::TrainTracksAppWindow(BaseObjectType* cobj, const Glib::RefPtr<Gtk::Builder>& builder, const TrainTracksApp& app) : Gtk::ApplicationWindow(cobj),
		m_app(app), m_speed(55), m_mousePressed(false) {
	builder->get_widget("gl_drawing_area", m_glDrawingArea);
	builder->get_widget("play_button", m_playButton);
	m_speedAdjustment = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic((builder->get_object("speed_adjustment")));
	
	m_glDrawingArea->signal_realize().connect(sigc::mem_fun(*this, &TrainTracksAppWindow::glInit));
	m_glDrawingArea->signal_unrealize().connect(sigc::mem_fun(*this, &TrainTracksAppWindow::glFini));
	m_glDrawingArea->signal_render().connect(sigc::mem_fun(*this, &TrainTracksAppWindow::glDraw));
	m_glDrawingArea->signal_resize().connect(sigc::mem_fun(*this, &TrainTracksAppWindow::glResize));
	
	signal_scroll_event().connect(sigc::mem_fun(*this, &TrainTracksAppWindow::mouseScrollEvt));
	signal_button_press_event().connect(sigc::mem_fun(*this, &TrainTracksAppWindow::buttonEvt));
	signal_button_release_event().connect(sigc::mem_fun(*this, &TrainTracksAppWindow::buttonEvt));
	signal_motion_notify_event().connect(sigc::mem_fun(*this, &TrainTracksAppWindow::motionEvt));
	
	m_speedAdjustment->signal_value_changed().connect(sigc::mem_fun(*this , &TrainTracksAppWindow::speedChanged));
	
	m_playButton->signal_clicked().connect(sigc::mem_fun(*this, &TrainTracksAppWindow::playToggled));
	
	set_icon_name("train_tracks");
}

TrainTracksAppWindow::~TrainTracksAppWindow() {
	if (m_drawConnection) m_drawConnection.disconnect();
}

void TrainTracksAppWindow::glInit() {
	Glib::ustring title;
	const char* renderer;
	
	// we need to ensure that the Gdk::GLContext is set before calling GL API
	m_glDrawingArea->make_current();
	
	// if the GtkGLArea is in an error state we don't do anything
	try {
		m_glDrawingArea->throw_if_error();
		
		// initialize glew
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			std::cerr << "Failed to initialise glew" << std::endl;
			return;
		}
		
		// set the window title
		renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
#ifdef NDEBUG
		title = "Train tracks" VERSION " on ";
#else
		title = "Train tracks" VERSION "(DEBUG) on ";
#endif
		if (renderer)
			title.append(renderer);
		else
			title.append("Unknown");
		
		set_title(title);
		
		glLineWidth(1.0);
		glEnable(GL_LINE_SMOOTH);
		m_programsGL = ProgramsGL(0);
		m_rendererGL = RendererGL(m_programsGL);
		
		initWorker();
		updateTargetFrameRate();
		glInited = true;
	} catch (Glib::Error& err) {
		std::cerr << err.what() << std::endl;
	} catch (ProgGlBaseException& err) {
		std::cerr << err.what() << err.getMessage() << std::endl;
		Glib::ustring errorMsg(err.what());
		errorMsg += err.getMessage();
		Glib::Error error(train_tracks_error_quark.id(), train_tracks_errors::TRAIN_TRACKS_ERROR_SHADER, errorMsg);
		m_glDrawingArea->set_error(error);
	}
}

void TrainTracksAppWindow::glFini() {
	// we need to ensure that the Gdk::GLContext is set before calling GL API
	m_glDrawingArea->make_current();
	
	if (m_drawConnection) m_drawConnection.disconnect();
	stopWorker();
	
	try {
		m_glDrawingArea->throw_if_error();
		
		// Destroy GL resources
		m_rendererGL = RendererGL();
		m_programsGL = ProgramsGL();
	} catch (Glib::Error& err) {
		std::cerr << err.what() << std::endl;
		return;
	}
}

bool TrainTracksAppWindow::glDraw(const Glib::RefPtr<Gdk::GLContext>& context) {
	/* clear the viewport; the viewport is automatically resized when
	* the GtkGLArea gets a new size allocation
	*/
	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	double frame_time = m_timer.elapsed();
	
	// draw
	processDisplayCommand(m_rendererGL, m_speed*m_speed * (frame_time-m_lastFrameTime)/50.0);
	m_rendererGL.draw();
	
	// flush the contents of the pipeline 
	glFlush ();
	m_lastFrameTime = frame_time;
	
	return true;
}

void TrainTracksAppWindow::glResize(int width, int height) {
	m_rendererGL.resize(width, height);
	updateTargetFrameRate();
}

bool TrainTracksAppWindow::mouseScrollEvt(GdkEventScroll* evt) {
	double delta = evt->delta_y;
	float factor = pow(0.909090909090, delta);
	m_rendererGL.scale(factor);
	m_glDrawingArea->queue_render();
	return true;
}

bool TrainTracksAppWindow::buttonEvt(GdkEventButton* evt) {
	if (evt->button == 1) {
		if (evt->type == GDK_BUTTON_PRESS) {
			m_mousePressed = true;
			m_mouseX = evt->x;
			m_mouseY = evt->y;
			return true;
		}
		else if (evt->type == GDK_BUTTON_RELEASE) {
			m_mousePressed = false;
			return true;
		}
	}
	return false;	
}

bool TrainTracksAppWindow::motionEvt(GdkEventMotion* evt) {
	if (m_mousePressed) {
		double dx = evt->x - m_mouseX;
		double dy = m_mouseY - evt->y;
		m_mouseX = evt->x;
		m_mouseY = evt->y;
		m_rendererGL.translate(static_cast<float>(dx), static_cast<float>(dy));
		m_glDrawingArea->queue_render();
		return true;
	}
	return false;	
}

bool TrainTracksAppWindow::on_drag_motion(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time) {
	updateTargetFrameRate();
	return Gtk::Widget::on_drag_motion(context, x, y, time);
}

void TrainTracksAppWindow::speedChanged() {
	m_speed = m_speedAdjustment->get_value();
}

void TrainTracksAppWindow::playToggled() {
	bool toggled = m_playButton->get_active();
	
	if (toggled)
	{
		postOnPlay();
	}
	else
	{
		
	}	
}

bool TrainTracksAppWindow::onTimeout() {
	m_glDrawingArea->queue_render();
	return true;
}

void TrainTracksAppWindow::updateTargetFrameRate() {
	const Glib::RefPtr<Gdk::Window> window(get_window());
	if (window) {
		Glib::RefPtr<const Gdk::Display> display(get_display());
		Glib::RefPtr<const Gdk::Monitor> monitor(display->get_monitor_at_window(window));
		
		int refreshRate = monitor->get_refresh_rate();
		refreshRate = (refreshRate) ? refreshRate : 120000;
		unsigned int timeout = static_cast<unsigned int>(1e+6/refreshRate + 0.5);
		
		if (m_drawConnection) m_drawConnection.disconnect();
		m_drawConnection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &TrainTracksAppWindow::onTimeout), timeout);
		m_lastFrameTime = m_timer.elapsed();
	}
}
