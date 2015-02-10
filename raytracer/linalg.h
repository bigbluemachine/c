#ifndef LINALG_H
#define LINALG_H

#include <math.h>

struct _vector { double x, y, z, w; };
typedef struct _vector vector;

struct _matrix { vector i, j, k, l; };
typedef struct _matrix matrix;

/*
	Copies vector.
	r <- a
*/
void vcopy(vector * const r, const vector *a) {
	r->x = a->x;
	r->y = a->y;
	r->z = a->z;
	r->w = a->w;
}

/*
	2-norm of vector.
*/
double norm2(const vector *a) {
	return sqrt(a->x * a->x + a->y * a->y + a->z * a->z);
}

/*
	Dot product of vectors ignoring position.
*/
double dot3(const vector *a, const vector *b) {
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

/*
	Dot product of vectors with position.
*/
double dot4(const vector *a, const vector *b) {
	return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}

/*
	Multiplies vector with scalar.
	r <- m * a
*/
void vsmul(vector * const r, double m, const vector *a) {
	r->x = m * a->x;
	r->y = m * a->y;
	r->z = m * a->z;
	r->w = m * a->w;
}

/*
	Normalizes vector.
	r <- r / |r|
*/
void normalize(vector * const r) {
	vsmul(r, 1.0 / norm2(r), r);
}

/*
	Adds vectors.
	r <- a + m * b
*/
void vvadd(vector * const r, const vector *a, double m, const vector *b) {
	r->x = a->x + m * b->x;
	r->y = a->y + m * b->y;
	r->z = a->z + m * b->z;
	r->w = a->w + m * b->w;
}

/*
	Cross product of vectors.
	r <- a X b
*/
void cross(vector * const r, const vector *a, const vector *b) {
	r->x = a->y * b->z - a->z * b->y;
	r->y = a->z * b->x - a->x * b->z;
	r->z = a->x * b->y - a->y * b->x;
	r->w = 0;
}

#define SET(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P)  \
	r->i.x=A;r->i.y=B;r->i.z=C;r->i.w=D;  \
	r->j.x=E;r->j.y=F;r->j.z=G;r->j.w=H;  \
	r->k.x=I;r->k.y=J;r->k.z=K;r->k.w=L;  \
	r->l.x=M;r->l.y=N;r->l.z=O;r->l.w=P;

/*
	Copies matrix.
	r <- m
*/
void mcopy(matrix * const r, const matrix *m) {
	SET(
		m->i.x, m->i.y, m->i.z, m->i.w,
		m->j.x, m->j.y, m->j.z, m->j.w,
		m->k.x, m->k.y, m->k.z, m->k.w,
		m->l.x, m->l.y, m->l.z, m->l.w
	)
}

/*
	Multiplies matrix with vector.
	r <- m * a
*/
void mvmul(vector * const r, const matrix *m, const vector *a, int dir) {
	if(dir) {
		r->x = dot3(&m->i, a);
		r->y = dot3(&m->j, a);
		r->z = dot3(&m->k, a);
		r->w = 0;
	} else {
		r->x = dot4(&m->i, a);
		r->y = dot4(&m->j, a);
		r->z = dot4(&m->k, a);
		r->w = dot4(&m->l, a);
	}
}

/*
	Multiplies matrix with vector in place.
	r <- m * r
*/
void mvmulip(vector * const r, const matrix *m, int dir) {
	vector t;
	mvmul(&t, m, r, dir);
	vcopy(r, &t);
}

/*
	Multiplies matrices.
	r <- m * n
*/
void mmmul(matrix * const r, const matrix *m, const matrix *n) {
#define F(A,B)  (m->A.x*n->i.B+m->A.y*n->j.B+m->A.z*n->k.B+m->A.w*n->l.B)
	SET(
		F(i, x), F(i, y), F(i, z), F(i, w),
		F(j, x), F(j, y), F(j, z), F(j, w),
		F(k, x), F(k, y), F(k, z), F(k, w),
		F(l, x), F(l, y), F(l, z), F(l, w)
	)
#undef  F
}

/*
	Multiples matrices in place.
	r <- r * m
*/
void mmmulip(matrix * const r, const matrix *m) {
	matrix t;
	mmmul(&t, r, m);
	mcopy(r, &t);
}

/*
	Transposes matrix.
	r <- m^T
*/
void transpose(matrix * const r, const matrix *m) {
	SET(
		m->i.x, m->j.x, m->k.x, m->l.x,
		m->i.y, m->j.y, m->k.y, m->l.y,
		m->i.z, m->j.z, m->k.z, m->l.z,
		m->i.w, m->j.w, m->k.w, m->l.w
	)
}

/*
	Inverts matrix.
	r <- m^-1
*/
void invert(matrix * const r, const matrix *m) {
	double rdet;
#define D3(A,B,C,D,E,F,G,H,I)  (A*(E*I-F*H)-B*(D*I-F*G)+C*(D*H-E*G))
#define DT(A,B,C,D,E,F)  \
	D3(m->A.D,m->A.E,m->A.F,m->B.D,m->B.E,m->B.F,m->C.D,m->C.E,m->C.F)
	r->i.x =  DT(j, k, l, y, z, w);
	r->i.y = -DT(i, k, l, y, z, w);
	r->i.z =  DT(i, j, l, y, z, w);
	r->i.w = -DT(i, j, k, y, z, w);
	r->j.x = -DT(j, k, l, x, z, w);
	r->j.y =  DT(i, k, l, x, z, w);
	r->j.z = -DT(i, j, l, x, z, w);
	r->j.w =  DT(i, j, k, x, z, w);
	r->k.x =  DT(j, k, l, x, y, w);
	r->k.y = -DT(i, k, l, x, y, w);
	r->k.z =  DT(i, j, l, x, y, w);
	r->k.w = -DT(i, j, k, x, y, w);
	r->l.x = -DT(j, k, l, x, y, z);
	r->l.y =  DT(i, k, l, x, y, z);
	r->l.z = -DT(i, j, l, x, y, z);
	r->l.w =  DT(i, j, k, x, y, z);
#undef  DT
#undef  D3
	rdet = 1.0 / (m->i.x * r->i.x + m->i.y * r->j.x
		+ m->i.z * r->k.x + m->i.w * r->l.x);
	vsmul(&r->i, rdet, &r->i);
	vsmul(&r->j, rdet, &r->j);
	vsmul(&r->k, rdet, &r->k);
	vsmul(&r->l, rdet, &r->l);
}

/*
	Identity matrix.
*/
void identity(matrix * const r) {
	SET(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	)
}

/*
	Scaling matrix.
*/
void scaling(matrix * const r, double x, double y, double z) {
	SET(
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1
	)
}

/*
	Translation matrix.
*/
void translation(matrix * const r, double x, double y, double z) {
	SET(
		1, 0, 0, x,
		0, 1, 0, y,
		0, 0, 1, z,
		0, 0, 0, 1
	)
}

/*
	Rotation matrix in radians.
*/
void rotationRad(matrix * const r, double a, double x, double y, double z) {
	double ca = cos(a), sa = sin(a), ta = 1.0 - ca;
	SET(
		x * x * ta + ca, y * x * ta - z * sa, z * x * ta + y * sa, 0,
		x * y * ta + z * sa, y * y * ta + ca, z * y * ta - x * sa, 0,
		x * z * ta - y * sa, y * z * ta + x * sa, z * z * ta + ca, 0,
		0, 0, 0, 1
	)
}

/*
	Rotation matrix in degrees.
*/
void rotationDeg(matrix * const r, double a, double x, double y, double z) {
#define T  6.283185307179586
	rotationRad(r, T / 360.0 * a, x, y, z);
#undef  T
}

#undef  SET

#endif
