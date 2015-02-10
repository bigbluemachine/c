#include <stdio.h>

void p16(FILE *f, short s) {
	fputc(s & 0xff, f);
	fputc((s >> 8) & 0xff, f);
}

void p32(FILE *f, int i) {
	fputc(i & 0xff, f);
	fputc((i >> 8) & 0xff, f);
	fputc((i >> 16) & 0xff, f);
	fputc((i >> 24) & 0xff, f);
}

void outputBMP(char *filename, double *buf, int w, int h) {
	FILE *f;
	int i, j;
	double *c;

	f = fopen(filename, "wb");
	i = 8 * w * h;

	fputc('B', f);	// (2) Signature
	fputc('M', f);
	p32(f, 54 + i);	// (4) File size
	p32(f, 0);		// (4) Reserved
	p32(f, 54);		// (4) Offset to start of data
	p32(f, 40);		// (4) Size of info header
	p32(f, w);		// (4) Width
	p32(f, h);		// (4) Height
	p16(f, 1);		// (2) Planes
	p16(f, 24);		// (2) Bits per pixel
	p32(f, 0);		// (4) Compression type
	p32(f, i);		// (4) Image data size
	p32(f, 0);		// (16) Unused
	p32(f, 0);
	p32(f, 0);
	p32(f, 0);

	for(j = 0; j < h; j++) {
		for(i = 0; i < w; i++) {
			c = buf + 3 * (h * i + j);
			fputc((int) (*(c + 2) * 0xff), f);
			fputc((int) (*(c + 1) * 0xff), f);
			fputc((int) (*(c) * 0xff), f);
		}
	}

	fclose(f);
}
