#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "linalg.h"
#include "model.h"
#include "parser.h"
#include "outbmp.h"
#include "tracer.h"

camera CAM;
scene SCN;
int DEPTH;
double *BUF;
int W, H;

double clamp(double a, double x, double z) {
	return a < x ? (x < z ? x : z) : a;
}

void clampColor(vector * const col) {
	col->x = clamp(0.0, col->x, 1.0);
	col->y = clamp(0.0, col->y, 1.0);
	col->z = clamp(0.0, col->z, 1.0);
}

void initCamera(camera *cam) {
#define EPS  1e-9
	cam->o.x = 0; cam->o.y = 0; cam->o.z = 0; cam->o.w = 1;
	cam->y.x = 0; cam->y.y = 1; cam->y.z = 0; cam->y.w = 0;
	cam->z.x = 0; cam->z.y = 0; cam->z.z = 1; cam->z.w = 0;
	cam->angle = 90.0;
	cam->aspect = 1;
	cam->near = EPS;
#undef  EPS
}

void raytracer(camera *cam, scene *scn) {
#define INF  1e100
	int i, j;
	vector v, x, llc, col;
	double lx, ly, dx, dy;
	ray r;
	double *c;

#define T  6.283185307179586
	ly = cam->near * tan(cam->angle * T / 720.0);
#undef  T
	lx = cam->aspect * ly;
	dx = 2.0 * lx / (W - 1);
	dy = 2.0 * ly / (H - 1);

	normalize(&cam->y);
	normalize(&cam->z);
	cross(&cam->x, &cam->z, &cam->y);
	normalize(&cam->x);

	vvadd(&llc, &cam->o, cam->near, &cam->z);
	vvadd(&llc, &llc, -lx, &cam->x);
	vvadd(&llc, &llc, -ly, &cam->y);
	vvadd(&v, &llc, -1, &cam->o);

	for(i = 0; i < W; i++) {
		vsmul(&x, (double) i * dx, &cam->x);
		for(j = 0; j < H; j++) {
			vvadd(&r.direction, &x, (double) j * dy, &cam->y);
			vvadd(&r.direction, &r.direction, 1, &v);
			vvadd(&r.origin, &r.direction, 1, &cam->o);
			normalize(&r.direction);
			col.x = col.y = col.z = 0;
			trace(&col, scn, &r, INF, DEPTH);
			clampColor(&col);
			c = BUF + 3 * (H * i + j);
			*c = col.x;
			*(c + 1) = col.y;
			*(c + 2) = col.z;
		}
	}
#undef  INF
}

int main(int argc, char** argv) {
	int i;
	char *outFile;
	char *defaultFile = "out.bmp";

	if(argc < 2) {
		fprintf(stderr, "Usage: %s infile [-d depth] [-o outfile]\n", argv[0]);
		return 1;
	}

	outFile = 0;
	DEPTH = 0;
	for(i = 2; i + 1 < argc; i++)
		if(!strcmp(argv[i], "-d"))
			DEPTH = atoi(argv[i + 1]);
		else if(!strcmp(argv[i], "-o"))
			outFile = argv[i + 1];

	initCamera(&CAM);
	SCN.lights = list_make(4, sizeof(light));
	SCN.materials = list_make(4, sizeof(material));
	SCN.objects = list_make(4, sizeof(object));

	parse(argv[1], &CAM, &SCN, &W, &H);
	BUF = (double *) calloc(3 * W * H, sizeof(double));

	printf("Rendering... "); fflush(stdout);
	raytracer(&CAM, &SCN);
	printf("done.\n"); fflush(stdout);

	if(!outFile) {
		outFile = defaultFile;
	}

	printf("Writing bitmap to file %s... ", outFile); fflush(stdout);
	outputBMP(outFile, BUF, W, H);
	printf("done.\n"); fflush(stdout);

#define CLOCKS_PER_MS  (CLOCKS_PER_SEC / 1000.0)
	printf("Completed in %d ms.\n", (int) (clock() / CLOCKS_PER_MS));
#undef  CLOCKS_PER_MS
	fflush(stdout);

	return 0;
}
