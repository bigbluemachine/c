#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linalg.h"
#include "list.h"
#include "model.h"

#define EQ(a,b)  !strcmp(a, b)
#define GET()    (parser.t = strtok(parser.t ? 0 : parser.buf, "\t\n\r "))
#define GETI(x)  x = atoi(GET())
#define GETD(x)  x = atof(GET())
#define DO(x,y)  if(EQ(parser.t, x)) y

struct _parser {
	light l;
	material m;
	object o;
	matrix cur, temp;
	list *names, *trans;
	char *buf, *t;
} parser;

int getMatIndex(const char *k) {
	unsigned int i;
	for(i = 0; i < parser.names->count; i++)
		if(EQ(*((char **) list_get(parser.names, i)), k)) return i;
	return -1;
}

void setLight(light * const l, int p, double x, double y, double z) {
	l->coefs[p].x = x;
	l->coefs[p].y = y;
	l->coefs[p].z = z;
	l->coefs[p].w = 1.0;
}

void setMat(material * const m, int c, double x, double y, double z) {
	m->coefs[c].x = x;
	m->coefs[c].y = y;
	m->coefs[c].z = z;
	m->coefs[c].w = 1.0;
}

void parseLight(const scene *scn) {
	double x, y, z;
	memset(&parser.l, 0, sizeof(light));
#define SET(a)  GETD(x); GETD(y); GETD(z); setLight(&parser.l, a, x, y, z)
	while(GET()) {
		DO( "end",  { break; } )
		DO( "pos",  { SET(POS); } )
		DO( "ia",   { SET(IA); } )
		DO( "id",   { SET(ID); } )
		DO( "is",   { SET(IS); } )
		DO( "att",  { SET(ATT); } )
	}
#undef  SET
	list_add(scn->lights, &parser.l);
}

void parseMaterial(const scene *scn) {
	double x, y, z;
	char *name = GET();
	memset(&parser.m, 0, sizeof(material));
	parser.m.index = 1.0;
#define SET(a)  GETD(x); GETD(y); GETD(z); setMat(&parser.m, a, x, y, z)
	while(GET()) {
		DO( "end",  { break; } )
		DO( "ka",   { SET(KA); } )
		DO( "kd",   { SET(KD); } )
		DO( "ks",   { SET(KS); } )
		DO( "exp",  { GETD(parser.m.exponent); } )
		DO( "shd",  { GETD(parser.m.shadow); } )
		DO( "ref",  { GETD(parser.m.reflect); } )
	}
#undef  SET
	list_add(parser.names, &name);
	list_add(scn->materials, &parser.m);
}

void parsePush() {
	list_add(parser.trans, &parser.cur);
}

void parsePop() {
	mcopy(&parser.cur, list_get(parser.trans, --parser.trans->count - 1));
}

void parseTranslate() {
	double x, y, z;
	GETD(x); GETD(y); GETD(z);
	translation(&parser.temp, x, y, z);
	mmmulip(&parser.cur, &parser.temp);
}

void parseScale() {
	double x, y, z;
	GETD(x); GETD(y); GETD(z);
	scaling(&parser.temp, x, y, z);
	mmmulip(&parser.cur, &parser.temp);
}

void parseRotate() {
	double a, x, y, z;
	GETD(a); GETD(x); GETD(y); GETD(z);
	rotationDeg(&parser.temp, a, x, y, z);
	mmmulip(&parser.cur, &parser.temp);
}

void parseObject(const scene *scn, int type) {
	memset(&parser.o, 0, sizeof(object));
	parser.o.type = type;
	mcopy(&parser.o.trans, &parser.cur);
	invert(&parser.o.invTrans, &parser.o.trans);
	transpose(&parser.o.normalTrans, &parser.o.invTrans);
	parser.o.matIndex = getMatIndex(GET());
	list_add(scn->objects, &parser.o);
}

void parseView(camera * const cam) {
	while(GET()) {
		DO( "end",   { break; } )
		DO( "angle",  { GETD(cam->angle); } )
		DO( "aspect", { GETD(cam->aspect); } )
		DO( "near",   { GETD(cam->near); } )
	}
}

void parseCamera(camera * const cam) {
	mvmulip(&cam->o, &parser.cur, 0);
	mvmulip(&cam->y, &parser.cur, 1);
	mvmulip(&cam->z, &parser.cur, 1);
}

void parseImage(int * const width, int * const height) {
	while(GET()) {
		DO( "end",    { break; } )
		DO( "width",  { GETI(*width); } )
		DO( "height", { GETI(*height); } )
	}
}

void parse(
	const char *filename,
	camera * const cam,
	const scene *scn,
	int * const width,
	int * const height
) {
	FILE *fp;
	long len;
	int ignore = 0;

	if(!(fp = fopen(filename, "r"))) {
		fprintf(stderr, "Input file %s not found.\n", filename);
		exit(1);
	}

	fseek(fp, 0L, SEEK_END);
	parser.buf = (char *) malloc(len = ftell(fp));
	rewind(fp);
	fread(parser.buf, len, 1, fp);
	fclose(fp);

	parser.names = list_make(4, sizeof(char *));
	parser.trans = list_make(4, sizeof(matrix));

	identity(&parser.cur);
	list_add(parser.trans, &parser.cur);

	while(GET()) {
		DO( "###",       { ignore ^= 1; } )
		if( ignore )     { continue; }
		DO( "light",     { parseLight(scn); } )
		DO( "material",  { parseMaterial(scn); } )
		DO( "push",      { parsePush(); } )
		DO( "pop",       { parsePop(); } )
		DO( "translate", { parseTranslate(); } )
		DO( "scale",     { parseScale(); } )
		DO( "rotate",    { parseRotate(); } )
		DO( "sphere",    { parseObject(scn, OT_SPHERE); } )
		DO( "plane",     { parseObject(scn, OT_PLANE); } )
		DO( "view",      { parseView(cam); } )
		DO( "camera",    { parseCamera(cam); } )
		DO( "image",     { parseImage(width, height); } )
	}

	list_free(parser.names);
	list_free(parser.trans);
	free(parser.buf);
}

#undef  DO
#undef  GETD
#undef  GETI
#undef  GET
#undef  EQ

#endif
