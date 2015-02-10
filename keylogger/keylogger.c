/*
	This is for educational purposes only.
	Negative numbers are printed when keys are released.
*/
#include <stdio.h>
#include <string.h>
#include <windows.h>

SHORT state[256];

void process(int i) {
	SHORT temp;

	temp = GetAsyncKeyState(i);
	if(state[i] == 0) {
		if(temp & 0x8000) {
			printf("%d\n", i);
		}
	} else {
		if(temp == 0) {
			printf("-%d\n", i);
		}
	}

	state[i] = temp;
}

int main(void) {
	int i, j;

	memset(state, 0, sizeof(state));
	for(i = 0;; i++) {
		for(j = 0; j < 256; j++) {
			process(j);
		}
		if(i & 0xff) {
			fflush(stdout);
		}
		Sleep(10);
	}

	return 0;
}
