#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model.c"

#define BOOL     unsigned char

#define NEXT()   strtok(0, " \n")
#define NEXTI()  atoi(NEXT())
#define NEXTF()  atof(NEXT())
#define EQ(s,t)  !strcmp(s, t)

char *fmt3f = "%-5s%7.2f %7.2f %7.2f\n";
char *fmtKF = "%-5d%7.2f %7.2f %7.2f\n";

FILE *f;
BOOL COMMENTS = 0;
char *inFile, *s;

int _indexOf(char *s) {
	unsigned int i;
	for(i = 0; i < NP; i++) if(EQ(s, PN[i])) return i;
	return 0;
}

void _readSubs() {
	unsigned int i;
	NP = NEXTI() + 1;
	sscanf("main", "%s", PN[0]);
	for(i = 1; i < NP; i++) sscanf(NEXT(), "%s", PN[i]);
}

void _readDef() {
	unsigned int i = _indexOf(NEXT()), j;
	while(s = NEXT()) {
		if(EQ(s, "dim"))
			V[i][RX] = NEXTF(), V[i][RY] = NEXTF(), V[i][RZ] = NEXTF();
		else if(EQ(s, "org"))
			V[i][OX] = NEXTF(), V[i][OY] = NEXTF(), V[i][OZ] = NEXTF();
		else if(EQ(s, "off"))
			V[i][DX] = NEXTF(), V[i][DY] = NEXTF(), V[i][DZ] = NEXTF();
		else if(EQ(s, "end")) {
			j = NEXTI();
			while(j--) S[i][_indexOf(NEXT())] = 1;
			return;
		}
	}
}

void _readPos() {
	unsigned int d, i, j, prev = 0;
	double w;
	while(!EQ(s = NEXT(), "end")) {
		i = atoi(s);
		P[X][i] = NEXTF(), P[Y][i] = NEXTF(), P[Z][i] = NEXTF();

		if(!(d = i - prev)) continue;
		for(j = prev + 1, w = 1.0 / d; j != i; j++, w += 1.0 / d) {
			P[X][j] = _pint(P[X][prev], P[X][i], w);
			P[Y][j] = _pint(P[Y][prev], P[Y][i], w);
			P[Z][j] = _pint(P[Z][prev], P[Z][i], w);
		}
		prev = i;
	}

	d = NF - prev;
	for(j = (prev + 1) % NF, w = 1.0 / d; j; j = (j + 1) % NF, w += 1.0 / d) {
		P[X][j] = _pint(P[X][prev], P[X][0], w);
		P[Y][j] = _pint(P[Y][prev], P[Y][0], w);
		P[Z][j] = _pint(P[Z][prev], P[Z][0], w);
	}
}

void _readAng() {
	unsigned int d, i, j, k, prev = 0;
	double w;
	i = _indexOf(NEXT());
	while(!EQ(s = NEXT(), "end")) {
		j = atoi(s);
		A[i][X][j] = NEXTF(), A[i][Y][j] = NEXTF(), A[i][Z][j] = NEXTF();

		if(!(d = j - prev)) continue;
		for(k = prev + 1, w = 1.0 / d; k < j; k++, w += 1.0 / d) {
			A[i][X][k] = _aint(A[i][X][prev], A[i][X][j], w);
			A[i][Y][k] = _aint(A[i][Y][prev], A[i][Y][j], w);
			A[i][Z][k] = _aint(A[i][Z][prev], A[i][Z][j], w);
		}
		prev = j;
	}

	d = NF - prev;
	for(k = (prev + 1) % NF, w = 1.0 / d; k; k = (k + 1) % NF, w += 1.0 / d) {
		A[i][X][k] = _aint(A[i][X][prev], A[i][X][0], w);
		A[i][Y][k] = _aint(A[i][Y][prev], A[i][Y][0], w);
		A[i][Z][k] = _aint(A[i][Z][prev], A[i][Z][0], w);
	}
}

void _writeEnd(unsigned int p) {
	unsigned int j = 0, k;
	for(k = 0; k < NP; k++) if(S[p][k]) j++;
	fprintf(f, "end     %d", j);
	for(k = 0; k < NP; k++) if(S[p][k]) fprintf(f, " %s", PN[k]);
	fputc('\n', f);
}

void _writeDefs() {
	unsigned int i;

	fprintf(f, "subs    %d", NP - 1);
	for(i = 1; i < NP; i++) fprintf(f, " %s", PN[i]);
	fputc('\n', f);

	fprintf(f, "\ndef     %s\n", PN[0]);
	fprintf(f, fmt3f, "dim", V[0][RX], V[0][RY], V[0][RZ]);
	_writeEnd(0);

	for(i = 1; i < NP; i++) {
		fprintf(f, "\ndef     %s\n", PN[i]);
		fprintf(f, fmt3f, "dim", V[i][RX], V[i][RY], V[i][RZ]);
		fprintf(f, fmt3f, "org", V[i][OX], V[i][OY], V[i][OZ]);
		fprintf(f, fmt3f, "off", V[i][DX], V[i][DY], V[i][DZ]);
		_writeEnd(i);
	}
}

void _writeCapture() {
	unsigned int i;

	fprintf(f, "###\n\nFrame %.2f/%d\n\n###\n\n", CUR, NF);
	_writeDefs();

	fputs("\nframes  1\n", f);
	fputs("\npos\n", f);
	fprintf(f, fmtKF, 0, INTP[X], INTP[Y], INTP[Z]);
	fputs("end\n", f);

	for(i = 0; i < NP; i++) {
		fprintf(f, "\nang     %s\n", PN[i]);
		fprintf(f, fmtKF, 0, INTA[i][X], INTA[i][Y], INTA[i][Z]);
		fputs("end\n", f);
	}
}

void _writeAll() {
	unsigned int i, j;

	_writeDefs();

	fprintf(f, "\nframes  %d\n", NF);
	fputs("\npos\n", f);
	for(i = 0; i < NF; i++) fprintf(f, fmtKF, i, P[X][i], P[Y][i], P[Z][i]);
	fputs("end\n", f);

	for(i = 0; i < NP; i++) {
		fprintf(f, "\nang     %s\n", PN[i]);
		for(j = 0; j < NF; j++)
			fprintf(f, fmtKF, j, A[i][X][j], A[i][Y][j], A[i][Z][j]);
		fputs("end\n", f);
	}
}

int ioLoad(char *filename) {
	char *buffer;
	long len;
	size_t size;

	if(!(f = fopen(filename, "r")))  return 0;

	fseek(f, 0L, SEEK_END);
	len = ftell(f);
	buffer = (char *) malloc(len);
	rewind(f);
	size = fread(buffer, len, 1, f);
	fclose(f);

	s = strtok(buffer, " \n");
	while(s) {
		if(EQ(s, "###")) COMMENTS ^= 1;
		if(!COMMENTS) {
			if(EQ(s, "subs")) _readSubs();
			else if(EQ(s, "def")) _readDef();
			else if(EQ(s, "frames")) NF = NEXTI();
			else if(EQ(s, "pos")) _readPos();
			else if(EQ(s, "ang")) _readAng();
		}
		s = NEXT();
	}

	free(buffer);
	mSetFrame(0);
	puts("Loaded.");
	return 1;
}

void ioCapture() {
	unsigned int i;
	char filename[16];

	for(i = 0; ; i++) {
		sprintf(filename, "capture%d.txt", i);
		f = fopen(filename, "r+");
		if(f)
			fclose(f);
		else {
			f = fopen(filename, "w+");
			break;
		}
	}

	_writeCapture();
	fclose(f);
	printf("Captured to %s.\n", filename);
}

void ioSave() {
	f = fopen(inFile, "w+");
	_writeAll();
	fclose(f);
	puts("Saved.");
}
