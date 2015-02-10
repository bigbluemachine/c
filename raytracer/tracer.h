#ifndef FUNCS_H
#define FUNCS_H

#include "linalg.h"
#include "list.h"
#include "model.h"

#define EPS  1e-9
#define INF  1e100

void reflect(vector * const r, const vector *v, const vector *n) {
	vector t;
	vsmul(&t, 2.0 * dot3(v, n), n);
	vvadd(r, &t, -1, v);
}

void getNormal(
	vector * const n,
	const object *o,
	const vector *d,
	const vector *p
) {
	switch(o->type) {
		case OT_SPHERE:
			vcopy(n, p);
			break;
		case OT_PLANE:
			n->x = n->y = 0;
			n->z = d->z > 0 ? -1 : 1;
			break;
		default:
			return;
	}
	n->w = 0;
}

double intersect(ray * const h, double *u, const object *o, const ray *r) {
	ray local;
	double a, b, c, d, t;

	mvmul(&local.origin, &o->invTrans, &r->origin, 0);
	mvmul(&local.direction, &o->invTrans, &r->direction, 1);

	switch(o->type) {
		case OT_SPHERE:
			a = dot3(&local.direction, &local.direction);
			b = 2.0 * dot3(&local.direction, &local.origin);
			c = dot3(&local.origin, &local.origin) - 1.0;
			if((d = b * b - 4 * a * c) < 0) {
				return 0;
			}
			d = sqrt(d);
			if((t = -0.5 * (b + d) / a) <= 0 && (t = -0.5 * (b - d) / a) <= 0) {
				return 0;
			}
			if(t >= *u) {
				return 0;
			}
			break;
		case OT_PLANE:
			if(local.direction.z == 0) {
				return 0;
			}
			if((t = -local.origin.z / local.direction.z) <= 0) {
				return 0;
			}
			if(t >= *u) {
				return 0;
			}
			break;
		default:
			return 0;
	}

	if(h) {
		vvadd(&h->origin, &local.origin, t, &local.direction);
		getNormal(&h->direction, o, &local.direction, &h->origin);
		mvmulip(&h->origin, &o->trans, 0);
		mvmulip(&h->direction, &o->normalTrans, 1);
		normalize(&h->direction);
	}

	return *u = t;
}

void shade(
	vector * const c,
	const scene *scn,
	const material *m,
	const light *l,
	const ray *h,
	const ray *r
) {
	size_t i;
	double a, d;
	ray t;
	vector u, v;
	const vector *ka = &m->coefs[KA];
	const vector *kd = &m->coefs[KD];
	const vector *ks = &m->coefs[KS];
	const vector *ia = &l->coefs[IA];
	const vector *id = &l->coefs[ID];
	const vector *is = &l->coefs[IS];
	const vector *pos = &l->coefs[POS];
	const vector *att = &l->coefs[ATT];

	vcopy(&t.origin, &h->origin);
	vvadd(&t.direction, pos, -1, &h->origin);
	vvadd(&v, &r->origin, -1, &h->origin);
	reflect(&u, &t.direction, &h->direction);
	vsmul(&u, 1.0 / (d = norm2(&u)), &u);
	normalize(&v);

	c->x = c->y = c->z = 0;
	c->x += ka->x * ia->x;
	c->y += ka->y * ia->y;
	c->z += ka->z * ia->z;
	if((a = dot3(&h->direction, &u)) >= 0) {
		c->x += a * kd->x * id->x;
		c->y += a * kd->y * id->y;
		c->z += a * kd->z * id->z;
	}
	if((a = dot3(&u, &v)) >= 0) {
		a = pow(a, m->exponent);
		c->x += a * ks->x * is->x;
		c->y += a * ks->y * is->y;
		c->z += a * ks->z * is->z;
	}
	vsmul(c, 1.0 / (att->x + att->y * d + att->z * d * d), c);

	d = 1.0 - EPS;
	vvadd(&t.origin, &t.origin, EPS, &t.direction);
	for(i = 0; i < scn->objects->count; i++) {
		if(intersect(0, &d, list_get(scn->objects, i), &t)) {
			vsmul(c, 1.0 - m->shadow, c);
			break;
		}
	}
}

double trace(
	vector * const c,
	const scene *scn,
	const ray *r,
	double u,
	int d
) {
	size_t i;
	const void *ptr;
	const object *o;
	const material *m;
	ray h, a;
	vector t;

	for(o = 0, i = 0; i < scn->objects->count; i++) {
		if(intersect(&h, &u, ptr = list_get(scn->objects, i), r)) {
			o = ptr;
		}
	}

	if(!o) {
		return 0;
	}

	m = list_get(scn->materials, o->matIndex);
	for(i = 0; i < scn->lights->count; i++) {
		shade(&t, scn, m, list_get(scn->lights, i), &h, r);
		vvadd(c, &t, 1, c);
	}

	if(d > 0) {
		if(m->reflect > 0) {
			vsmul(&a.direction, -1.0, &r->direction);
			reflect(&a.direction, &a.direction, &h.direction);
			vvadd(&a.origin, &h.origin, EPS, &a.direction);
			t.x = t.y = t.z = 0;
			trace(&t, scn, &a, INF, d - 1);
			vsmul(&t, m->reflect, &t);
			vvadd(c, &t, 1.0 - m->reflect, c);
		}
	}

	return u;
}

#undef  INF
#undef  EPS

#endif
