#ifndef __C_CPP_BRIDGE_H__
#define __C_CPP_BRIDGE_H__

#ifdef __cplusplus
extern "C" {
#endif

int init_C(char** error_msg);
void render_C(float dt);
void resize_C(int width, int height);
void fini_C(void);

void mouse_scroll_C(double delta);
void mouse_motion_C(double dx, double dy);

void loadMatrix(const char* filename);

#ifdef __cplusplus
}
#endif

#endif /* __D_DPP_BRIDGE_H__ */
