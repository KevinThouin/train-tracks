#include "train_tracks_app.hpp"

int main(int argc, char* argv[]) {
	Glib::RefPtr<TrainTracksApp> app(TrainTracksApp::create(app));
	app->run(argc, argv);
}
