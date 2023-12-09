#include "graphics.h"
#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "x86.h"




// Current x and y coordinates to be used by lineto
int current_x = 0;
int current_y = 0;

// Current color to be used by setpencolour
char current_color = DEFAULT_COLOR;

void clear320x200x256() {
    char* video_memory = (char*)VIDEO_MEMORY;
    for (int i = 0; i < 320 * 200; i++) {
        video_memory[i] = 0; // Black color
    }
}

void plotPixel(int x, int y, char color) {
	char* video_memory = (char*)VIDEO_MEMORY;
	video_memory[y * 320 + x] = color;
}

int sys_setpixel(void) {
	int hdc, x, y;
	if (argint(0, &hdc) < 0) {
		return -1;
	}
	if (argint(1, &x) < 0) {
		return -1;
	}
	if (argint(2, &y) < 0) {
		return -1;
	}

	if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return -1;  // Coordinates are out of screen bounds
    }

	plotPixel(x, y, DEFAULT_COLOR);
	current_x = x;
	current_y = y;
	return 0;
}

int clip(int num, int max) {
	if (num < 0) {
		return 0;
	} else if (num >= max) {
		return max- 1;
	} else {
		return num;
	}
}

int sys_moveto(void) {
	int hdc, x, y;
	if (argint(0, &hdc) < 0) {
		return -1;
	}
	if (argint(1, &x) < 0) {
		return -1;
	}
	if (argint(2, &y) < 0) {
		return -1;
	}

	x = clip(x, SCREEN_WIDTH);
    y = clip(y, SCREEN_HEIGHT);

	current_x = x;
	current_y = y;


	return 0;
}

static inline int abs(int n) {
    return (n >= 0) ? n : -n;
}

void plotLineLow(int x0, int y0, int x1, int y1) {
	int dx = x1 - x0;
	int dy = y1 - y0;
	int yi = 1;
	if (dy < 0) {
		yi = -1;
		dy = -dy;
	}
	int D = (dy << 1) - dx; // Use bit shifting instead of multiplication by 2
	int incrE = dy << 1; // increment when moving east
	int incrNE = (dy - dx) << 1; // increment when moving north-eastW

	int y = y0;

	for (int x = x0; x <= x1; x++) {
		plotPixel(x, y, DEFAULT_COLOR);
		if (D > 0) {
			y = y + yi;
			D += incrNE;
		}
		D += incrE;
	}
}

void plotLineHigh(int x0, int y0, int x1, int y1) {
	int dx = x1 - x0;
	int dy = y1 - y0;
	int xi = 1;
	if (dx < 0) {
		xi = -1;
		dx = -dx;
	}

	int D = (dx << 1) - dy; // Use bit shifting instead of multiplication by 2
	int incrE = dx << 1; // increment when moving east
	int incrNE = (dx - dy) << 1; // increment when moving north-eastW
	int x = x0;

	for (int y = y0; y <= y1; y++) {
		plotPixel(x, y, DEFAULT_COLOR);
		if (D > 0) {
			x = x + xi;
			D += incrNE;
		}
		D += incrE;
	}
}

int sys_lineto(void) {
	int hdc, x, y;
	if (argint(0, &hdc) < 0) {
		return -1;
	}
	if (argint(1, &x) < 0) {
		return -1;
	}
	if (argint(2, &y) < 0) {
		return -1;
	}

	int abs_x = clip(abs(x - current_x), SCREEN_WIDTH);
	int abs_y = clip(abs(y - current_y), SCREEN_HEIGHT);

	if(abs_y < abs_x) {
		if (current_x > x) {
			plotLineLow(x, y, current_x, current_y);
		}
		else {
			plotLineLow(current_x, current_y, x, y);
		}
	}
	else {
		if (current_y > y) {
			plotLineHigh(x, y, current_x, current_y);
		}
		else {
			plotLineHigh(current_x, current_y, x, y);
		}
	}
	current_x = x;
	current_y = y;

	return 0;
}

void set_palette_color(int index, int r, int g, int b) {
    outb(PALETTE_INDEX, index);
    outb(PALETTE_DATA, r >> 2);  // VGA expects 6-bit values
    outb(PALETTE_DATA, g >> 2);
    outb(PALETTE_DATA, b >> 2);
}

int sys_setpencolour(void) {
	int index, r, g, b;
    if (argint(0, &index) < 0 || index < 16 || index > 255) {
        return -1;
    }
    if (argint(1, &r) < 0 || argint(2, &g) < 0 || argint(3, &b) < 0) {
        return -1;
    }

	r = clip(r, 64);
	g = clip(g, 64);
	b = clip(b, 64);

	set_palette_color(index, r, g, b);
	current_color = index;
	return 0;
}

int sys_selectpen(void) {
    int index, hdc;
    if (argint(0, &index) < 0 || index < 16 || index > 255) {
        return -1;
    }
	if (argint(1, &hdc) < 0) {
		return -1;
	}

	int previous_color = current_color;
	outb(PALETTE_INDEX, index);
	current_color = index;

	return previous_color;
}

int sys_fillrect(void) {
	int hdc;
	struct rect *r;
	if (argint(0, &hdc) < 0) {
		return -1;
	}
	if (argptr(1, (void*)&r, sizeof(*r)) < 0) {
		return -1;
	}

	r->top = clip(r->top, SCREEN_HEIGHT);
    r->left = clip(r->left, SCREEN_WIDTH);
    r->bottom = clip(r->bottom, SCREEN_HEIGHT);
    r->right = clip(r->right, SCREEN_WIDTH);

	for (int y = r->top; y < r->bottom; y++) {
        for (int x = r->left; x < r->right; x++) {
            plotPixel(x, y, current_color);
        }
    }

	return 0;
}
