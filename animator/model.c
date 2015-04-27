#include <math.h>

#define BOOL  unsigned char

#define MAXP  32
#define MAXF  32

enum _attr { RX, RY, RZ, OX, OY, OZ, DX, DY, DZ };
enum _axis { X, Y, Z };
enum _mode { NONE, POS, ANG };

char PN[MAXP][MAXP] = { 0 }, S[MAXP][MAXP] = { 0 };
double V[MAXP][9] = { 0.0 }, P[3][MAXF] = { 0.0 }, A[MAXP][3][MAXF] = { 0.0 };
unsigned int NP = 1, NF = 1;

double INTP[3] = { 0.0 }, INTA[MAXP][3] = { 0.0 };

double CUR = 0.0;
unsigned int CURR = 0;

BOOL TEXT = 1, PLAY = 0, INFO = 0, SELECT = 0, KMOD = 0, FILEMOD = 0;

unsigned int SEL = 0, EDMODE = 0, RESP = 0, RESA = 0;
unsigned int RA[4] = { 1, 10, 30, 90 };
double RP[3] = { 0.01, 0.10, 1.00 };

inline double _pint(double a, double b, double w) {
	return (1 - w) * a + w * b;
}

double _aint(double a, double b, double w) {
	while(a + 359.99 <= b)
		a += 360;
	while(a >= b + 359.99)
		b += 360;
	return (1 - w) * a + w * b;
}

inline double _calcP(double *v) {
	return _pint(v[CURR], v[(CURR + 1) % NF], CUR - CURR);
}

double _calcA(double *v) {
	return _aint(v[CURR], v[(CURR + 1) % NF], CUR - CURR);
}

void _update() {
	unsigned int i;
	CURR = (int) CUR;
	INTP[X] = _calcP(P[X]);
	INTP[Y] = _calcP(P[Y]);
	INTP[Z] = _calcP(P[Z]);
	for(i = 0; i < NP; i++) {
		INTA[i][X] = _calcA(A[i][X]);
		INTA[i][Y] = _calcA(A[i][Y]);
		INTA[i][Z] = _calcA(A[i][Z]);
	}
}

void mToggleText() {
	TEXT ^= 1;
	if(!TEXT) SELECT = 0, EDMODE = NONE;
}

void mTogglePlay() {
	KMOD = 0, PLAY ^= 1;
	if(PLAY) EDMODE = NONE;
}

void mToggleInfo() {
	if(!TEXT) TEXT = INFO = 1;
	else INFO ^= 1;
	if(!INFO) EDMODE = NONE;
}

void mToggleSel() {
	SELECT ^= 1;
	if(SELECT) TEXT = INFO = 1;
	else if(!SELECT && EDMODE == ANG) EDMODE = POS;
}

void mSwitchMode() {
	switch(EDMODE) {
	case NONE: EDMODE = POS, TEXT = INFO = 1, PLAY = 0; break;
	case POS: EDMODE = SELECT ? ANG : NONE; break;
	case ANG: EDMODE = NONE; break;
	}
}

void mSwitchRes() {
	switch(EDMODE) {
	case POS: RESP = (RESP + 1) % 3; break;
	case ANG: RESA = (RESA + 1) % 4; break;
	}
}

void mSelectNext() { if(SELECT) SEL = (SEL + 1) % NP; }
void mSelectPrev() { if(SELECT) SEL = (SEL + NP - 1) % NP; }

void mDecMod(int axis) {
	switch(EDMODE) {
	case POS: KMOD = 1, INTP[axis] -= RP[RESP];      break;
	case ANG: KMOD = 1, INTA[SEL][axis] -= RA[RESA]; break;
	}
}

void mIncMod(int axis) {
	switch(EDMODE) {
	case POS: KMOD = 1, INTP[axis] += RP[RESP];      break;
	case ANG: KMOD = 1, INTA[SEL][axis] += RA[RESA]; break;
	}
}

void mIncrement(double d) { KMOD = 0, CUR = fmod(CUR + NF + d, NF), _update(); }
void mPrevFrame() { KMOD = 0, CUR = (CURR + NF - 1) % NF, _update(); }
void mNextFrame() { KMOD = 0, CUR = (CURR + 1) % NF, _update(); }
void mSetFrame(double d) { KMOD = 0, CUR = fmod(d, NF), _update(); }

void mSaveKF(BOOL overwrite) {
	unsigned int i, j;
	if(!overwrite)
		for(i = NF++; i > CURR; i--) {
			P[X][i] = P[X][i - 1], P[Y][i] = P[Y][i - 1], P[Z][i] = P[Z][i - 1];
			for(j = 0; j < NP; j++) {
				A[j][X][i] = A[j][X][i - 1];
				A[j][Y][i] = A[j][Y][i - 1];
				A[j][Z][i] = A[j][Z][i - 1];
			}
		}
	P[X][CURR] = INTP[X], P[Y][CURR] = INTP[Y], P[Z][CURR] = INTP[Z];
	for(i = 0; i < NP; i++) {
		A[i][X][CURR] = INTA[i][X];
		A[i][Y][CURR] = INTA[i][Y];
		A[i][Z][CURR] = INTA[i][Z];
	}
	KMOD = 0, FILEMOD = 1;
	_update();
}

void mRemoveKF() {
	unsigned int i, j;
	P[X][CURR] = P[Y][CURR] = P[Z][CURR] = 0;
	for(i = 0; i < NP; i++) A[i][X][CURR] = A[i][Y][CURR] = A[i][Z][CURR] = 0;
	if(NF > 1) {
		NF--;
		for(i = CURR; i < NF; i++) {
			P[X][i] = P[X][i + 1], P[Y][i] = P[Y][i + 1], P[Z][i] = P[Z][i + 1];
			for(j = 0; j < NP; j++) {
				A[j][X][i] = A[j][X][i + 1];
				A[j][Y][i] = A[j][Y][i + 1];
				A[j][Z][i] = A[j][Z][i + 1];
			}
		}
		if(CURR == NF)
			CUR = CURR = NF - 1;
	}
	KMOD = 0, FILEMOD = 1;
	_update();
}
