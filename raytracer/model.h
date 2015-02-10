#ifndef MODEL_H
#define MODEL_H

#include "linalg.h"
#include "list.h"

enum LightCoefficient { POS, IA, ID, IS, ATT };
struct _light {
	vector coefs[5];
};

enum MaterialCoefficient { KA, KD, KS };
struct _material {
	vector coefs[3];
	double exponent, shadow, reflect;
	int index;
};

enum ObjectType { OT_SPHERE, OT_PLANE };
struct _object {
	matrix trans, invTrans, normalTrans;
	int type, matIndex;
};

struct _ray {
	vector origin, direction;
};

struct _camera {
	vector o, x, y, z;
	double angle, aspect, near;
};

struct _scene {
	list *lights, *materials, *objects;
};

typedef struct _light light;
typedef struct _material material;
typedef struct _object object;
typedef struct _ray ray;
typedef struct _camera camera;
typedef struct _scene scene;

#endif
