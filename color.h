#ifndef __COLOR_H__
#define __COLOR_H__

#ifdef __cplusplus
extern "C" {
#endif

struct RGB_t {
	float r;
	float g;
	float b;
};
typedef struct RGB_t RGB_t;

extern RGB_t backgroundColor;
extern RGB_t handleColor;
extern RGB_t borderColor;
extern RGB_t trackColor;
extern RGB_t arrowColor;

#ifdef __cplusplus
}
#endif

#endif /* __COLOR_H__ */
