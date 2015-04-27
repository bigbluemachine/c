#include <windows.h>
#include <GL/freeglut.h>
#include "io.c"

#define KEY_ENT  13
#define KEY_ESC  27
#define KEY_DEL  127

#define SLICES   20
#define STACKS   20

#define TEXTSC   3.125

struct _wnd { int w, h; } wnd;
struct _view { double theta, phi; } view = { -45, 15 };
struct _mouse { int x, y; } mouse;

float ambient[] = { 0.05, 0.05, 0.15, 1.0 };
float diffuse[] = { 0.9, 0.9, 0.8, 1.0 };
float direction[] = { 2, 5, 1, 0 };

unsigned int FPS = 5;
unsigned int elapsed = 0;
unsigned int textX, textY;
char buf[64];

void initOpenGL() {
	glClearColor(0, 0, 0, 1);
	glClearDepth(1);
	glEnable(GL_BLEND);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
}

void drawGround() {
	int x, y;
	for(x = -5; x < 5; x++) {
		for(y = -5; y < 5; y++) {
			glColor4f(1, 1, 1, (x + y) % 2 ? 0.75 : 0.5);
			glNormal3f(0, 1, 0);
			glBegin(GL_POLYGON);
			glVertex3f(x    , 0, y    );
			glVertex3f(x + 1, 0, y    );
			glVertex3f(x + 1, 0, y + 1);
			glVertex3f(x    , 0, y + 1);
			glEnd();
		}
	}
}

void draw(unsigned int i) {
	unsigned int j;

	glPushMatrix();

	glTranslatef(V[i][OX], V[i][OY], V[i][OZ]);
	glRotatef(INTA[i][X], 1, 0, 0);
	glRotatef(INTA[i][Y], 0, 1, 0);
	glRotatef(INTA[i][Z], 0, 0, 1);
	glTranslatef(V[i][DX], V[i][DY], V[i][DZ]);

	SELECT && SEL == i ? glColor3f(0.7, 1.0, 0.5) : glColor3f(1.0, 1.0, 1.0);

	glPushMatrix();
	glScalef(V[i][RX], V[i][RY], V[i][RZ]);
	glutSolidSphere(1.0, SLICES, STACKS);
	glPopMatrix();

	for(j = 0; j < MAXP; j++) if(S[i][j]) draw(j);

	glPopMatrix();
}

void textSetup() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, wnd.w, 0, wnd.h);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	glColor3f(0.0, 1.0, 0.0);
	glScalef(1 / TEXTSC, 1 / TEXTSC, 1 / TEXTSC);
	glRasterPos2s(textX = TEXTSC * 10, textY = TEXTSC * (wnd.h - 20));
}

void textCleanup() {
	glEnable(GL_LIGHTING);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void putLine(char *s) {
	unsigned int i;

	if(s) {
		// Using glutBitmapString makes the program crash horribly. (Why?)
		for(i = 0; i < strlen(s); i++) {
			glutBitmapCharacter(GLUT_BITMAP_8_BY_13, s[i]);
		}
	}

	glRasterPos2s(textX, textY -= TEXTSC * 15);
}

void drawText() {
	textSetup();

	putLine(PLAY ? "[ ] Paused" : "[*] Paused");
	putLine(PLAY ? "[*] Playing" : "[ ] Playing");
	sprintf(buf, "Frame %3d of %3d%s",
		CURR, NF, KMOD ? " *" : ""), putLine(buf);
	sprintf(buf, "FPS   %3d / sec", FPS), putLine(buf);

	if(INFO) {
		putLine(0);
		if(SELECT) {
			sprintf(buf, "%s", PN[SEL]), putLine(buf);
			sprintf(buf, "Position (%.2f, %.2f, %.2f)",
				INTP[X], INTP[Y], INTP[Z]), putLine(buf);
			sprintf(buf, "Angle    (%.2f, %.2f, %.2f)",
				INTA[SEL][X], INTA[SEL][Y], INTA[SEL][Z]), putLine(buf);
		} else {
			putLine("None selected");
			sprintf(buf, "Position (%.2f, %.2f, %.2f)",
				INTP[X], INTP[Y], INTP[Z]), putLine(buf);
		}
		switch(EDMODE) {
		case POS: sprintf(buf, "Editing position (+/- %.2f)",
					RP[RESP]), putLine(buf); break;
		case ANG: sprintf(buf, "Editing angle (+/- %d)",
					RA[RESA]), putLine(buf); break;
		}
	}

	textCleanup();
}

void updateTitle() {
	sprintf(buf, "%s%s", inFile, FILEMOD ? " *" : "");
	glutSetWindowTitle(buf);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0, (double) wnd.w / wnd.h, 0.1, 25);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		0.0, 2.5, 10.0,
		0.0, 2.5,  0.0,
		0.0, 1.0,  0.0
	);
	glRotated(view.phi, 1, 0, 0);
	glRotated(view.theta, 0, 1, 0);
	glLightfv(GL_LIGHT0, GL_POSITION, direction);

	glPushMatrix();
	glTranslatef(INTP[X], INTP[Y], INTP[Z]);
	draw(0);
	glPopMatrix();
	drawGround();
	if(TEXT) drawText();

	glutSwapBuffers();
}

void reshape(int w, int h) {
	glViewport(0, 0, wnd.w = w, wnd.h = h);
}

void mouseMove(int x, int y) {
	y = wnd.h - y;
	view.theta = ((int) view.theta + (x - mouse.x) + 360) % 360;
	view.phi -= (y - mouse.y);
	view.phi = view.phi < -90.0 ? -90.0 : view.phi > 90.0 ? 90.0 : view.phi;
	mouse.x = x;
	mouse.y = y;
}

void mouseClick(int button, int state, int x, int y) {
	y = wnd.h - y;
	mouse.x = x;
	mouse.y = y;
}

void keyboardSpec(int key, int x, int y) {
	switch(key) {
	case GLUT_KEY_F1: glutLeaveMainLoop(); break;
	case GLUT_KEY_F5:
		FILEMOD = PLAY = 0;
		ioLoad(inFile);
		updateTitle();
		break;
	case GLUT_KEY_F8:
		FILEMOD = 0;
		ioSave();
		updateTitle();
		break;
	case GLUT_KEY_F12: ioCapture(); break;
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch(key) {
	case ' ': mToggleText(); break;
	case 'p': mTogglePlay(); break;
	case 'i': mToggleInfo(); break;
	case 's': mToggleSel();  break;
	case 'e': mSwitchMode(); break;
	case 'r': mSwitchRes();  break;
	case 'g': mSelectPrev(); break;
	case 'h': mSelectNext(); break;

	case 'X': mDecMod(X); break; case 'x': mIncMod(X); break;
	case 'Y': mDecMod(Y); break; case 'y': mIncMod(Y); break;
	case 'Z': mDecMod(Z); break; case 'z': mIncMod(Z); break;

	case '[': mIncrement(-0.1); break;
	case ']': mIncrement(0.1);  break;
	case '{': mPrevFrame();     break;
	case '}': mNextFrame();     break;

	case '-': FPS = FPS <= 2 ? 1 :    FPS - 1; break;
	case '_': FPS = FPS <= 6 ? 1 :    FPS - 5; break;
	case '=': FPS = FPS >= 99 ? 100 : FPS + 1; break;
	case '+': FPS = FPS >= 95 ? 100 : FPS + 5; break;

	case KEY_DEL:
		mRemoveKF();
		updateTitle();
		break;
	case KEY_ENT:
		mSaveKF(glutGetModifiers() & GLUT_ACTIVE_SHIFT);
		updateTitle();
		break;

	default:
		if(key >= '0' && key <= '9') mSetFrame(0.1 * (key - '0') * NF);
	}
}

void step(int last) {
	elapsed = glutGet(GLUT_ELAPSED_TIME);
	if (PLAY)
		mIncrement(FPS * (elapsed - last) / 1000.0);

	glutPostRedisplay();

	int next = 30 + last - elapsed;
	glutTimerFunc(next > 0 ? next : 0, step, elapsed);
}

int main(int argc, char **argv) {
	switch(argc) {
	case 2:
		if(!ioLoad(inFile = argv[1])) {
			fprintf(stdout, "Failed to load %s.\n", argv[1]);
			return 1;
		}
		break;
	default: printf("Usage: %s infile\n", argv[0]); return 1;
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutCreateWindow(argv[0]);
	glutSetWindowTitle(inFile);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMotionFunc(mouseMove);
	glutMouseFunc(mouseClick);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboardSpec);
	glutTimerFunc(50, step, 0);

	initOpenGL();

	glutMainLoop();
	return 0;
}
