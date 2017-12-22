#include <cstring>
#include <cstdio>
#include <cmath>
#include <string>
#include <utility>
#include <fstream>

#include "c_cpp_bridge.h"
#include "prog_gl.hpp"
#include "draw_gl.hpp"
#include "curves.hpp"
#include "track.hpp"
#include "zero_handle.hpp"
#include "worker_thread.h"
#include "display_cmd.hpp"
#include "io.hpp"

ProgramsGL programsGL;
RendererGL rendererGL;

int init_C(char** error_msg)
{	
	int ret = 0;
	std::string str;
	try {
		programsGL = ProgramsGL(0);
		rendererGL = RendererGL(programsGL);
	} catch (std::string str) {
		*error_msg = static_cast<char*>(malloc(str.size()));
		strncpy(*error_msg, str.c_str(), str.size());
		ret = -1;
	}
	return ret;
}

void render_C(float dt) {
	processCommand(rendererGL, dt);
	
	rendererGL.draw();
}

void resize_C(int width, int height) {
	rendererGL.resize(width, height);
}

void fini_C(void) {
	rendererGL = RendererGL();
	programsGL = ProgramsGL();
}

void mouse_scroll_C(double delta) {
	float factor = pow(0.909090909090, delta);
	rendererGL.scale(factor);
}

void mouse_motion_C(double dx, double dy) {
	rendererGL.translate(static_cast<float>(dx), static_cast<float>(dy));
}

void loadMatrix(const char* filename) {
	std::ifstream file;
	file.open(filename);
	
	std::pair<FUMatrix, unsigned int> p;
	unsigned int& k = p.second;
	FUMatrix& mat = p.first;
	file >> p;
	bool fail = file.fail();
	file.close();
	
	if (fail && k<mat.size()) {
		postSetStructure(k, std::move(mat));
	} else
		std::cout << "Impossible de lire les donnees." << std::endl;
}
