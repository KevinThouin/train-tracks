#include <fstream>

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/builder.h>

#include "train_tracks_app.hpp"
#include "worker_thread.hpp"
#include "matrix.hpp"

Glib::RefPtr<TrainTracksApp> TrainTracksApp::create(Glib::RefPtr<TrainTracksApp>& self) {
	return Glib::RefPtr<TrainTracksApp>(new TrainTracksApp(self));
}

TrainTracksApp::TrainTracksApp(Glib::RefPtr<TrainTracksApp>& self) : Gtk::Application("ca.usherbrooke.math.lwatson.train_tracks",
		Gio::APPLICATION_FLAGS_NONE), m_self(self) {}

void TrainTracksApp::on_activate() {
	Gio::Application::on_activate();
	
	if (!m_window) {
		Glib::RefPtr<Gtk::Builder> builder(Gtk::Builder::create_from_resource("/ca/usherbrooke/math/lwatson/train_tracks/train_tracks_app_window.ui"));
		m_window = TrainTracksAppWindow::create(builder, *this);
		m_window->set_application(m_self);
	}
	
	m_window->present();
}

void TrainTracksApp::on_startup() {
	Gio::Application::on_startup();
	
	add_action("quit", sigc::mem_fun(*this, &TrainTracksApp::quitActivated));
	add_action("open", sigc::mem_fun(*this, &TrainTracksApp::openActivated));
	
	Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_resource("/ca/usherbrooke/math/lwatson/train_tracks/train_tracks_app_menu.ui");
	Glib::RefPtr<Gio::MenuModel> app_menu(Glib::RefPtr<Gio::MenuModel>::cast_dynamic(builder->get_object("appmenu")));
	set_app_menu(app_menu);
}

void TrainTracksApp::quitActivated() {
	quit();
}

void TrainTracksApp::openActivated() {
	if (!m_window) return;
	
	std::string filename;
	Gtk::FileChooserDialog dialog(*get_active_window(), "Open File", Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("_Open", Gtk::RESPONSE_OK);
	int res = dialog.run();
	if (res == Gtk::RESPONSE_OK) {
		filename = dialog.get_filename();
	}
	
	if (!filename.empty()) {
		std::ifstream file;
		file.open(filename.c_str());
		
		std::pair<FUMatrix, unsigned int> p;
		unsigned int& k = p.second;
		FUMatrix& mat = p.first;
		file >> p;
		bool fail = file.fail();
		file.close();
		
		if (fail && k<mat.size()) {
			postSetStructure(k, std::move(mat));
		} else {
			std::cout << "Impossible de lire les donnees." << std::endl;
		}
	}
}
