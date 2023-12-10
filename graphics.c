#include "graphics.h"
#include "defs.h"
#include "memlayout.h"
#include "x86.h"

struct device_context contexts[MAX_CONTEXTS];

void clear320x200x256() {
    char* video_memory = (char*)VIDEO_MEMORY;
    for (int i = 0; i < 320 * 200; i++) {
        video_memory[i] = 0; // Black color
    }
}

int clip(int num, int max) {
	if (num < 0) {
		return 0;
	} else if (num >= max) {
		return max - 1;
	} else {
		return num;
	}
}

void plotPixel(int x, int y, uint color) {
	char* video_memory = (char*)VIDEO_MEMORY;
	video_memory[y * 320 + x] = color;
}

int sys_setpixel(void) {
	int hdc, x, y;
	if (argint(0, &hdc) < 0 || hdc >= MAX_CONTEXTS || !contexts[hdc].used) {
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

	uint current_color = contexts[hdc].current_color;

	plotPixel(x, y, current_color);
	contexts[hdc].current_x = x;
	contexts[hdc].current_y = y;
	return 0;
}


int sys_moveto(void) {
	int hdc, x, y;
	if (argint(0, &hdc) < 0 || hdc >= MAX_CONTEXTS || !contexts[hdc].used) {
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

	contexts[hdc].current_x = x;
	contexts[hdc].current_y = y;

	return 0;
}

static inline int abs(int n) {
    return (n >= 0) ? n : -n;
}

void plotLineLow(int current_colour, int x0, int y0, int x1, int y1) {
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
		plotPixel(x, y, current_colour);
		if (D > 0) {
			y = y + yi;
			D += incrNE;
		}
		D += incrE;
	}
}

void plotLineHigh(int current_colour, int x0, int y0, int x1, int y1) {
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
		plotPixel(x, y, current_colour);
		if (D > 0) {
			x = x + xi;
			D += incrNE;
		}
		D += incrE;
	}
}

int sys_lineto(void) {
	int hdc, x, y;
	if (argint(0, &hdc) < 0 || hdc >= MAX_CONTEXTS || !contexts[hdc].used) {
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

	int current_x = contexts[hdc].current_x;
	int current_y = contexts[hdc].current_y;
	uint current_color = contexts[hdc].current_color;

	int abs_x = abs(x - current_x);
	int abs_y = abs(y - current_y);

	if(abs_y < abs_x) {
		if (current_x > x) {
			plotLineLow(current_color, x, y, current_x, current_y);
		}
		else {
			plotLineLow(current_color, current_x, current_y, x, y);
		}
	}
	else {
		if (current_y > y) {
			plotLineHigh(current_color, x, y, current_x, current_y);
		}
		else {
			plotLineHigh(current_color, current_x, current_y, x, y);
		}
	}
	
	contexts[hdc].current_x = x;
	contexts[hdc].current_y = y;

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

	return 0;
}

int sys_selectpen(void) {
    int index, hdc;
	if (argint(0, &hdc) < 0) {
		return -1;
	}
    if (argint(1, &index) < 0 || index > 255) {
        return -1;
    }


	uint previous_color = contexts[hdc].current_color;
	contexts[hdc].current_color = index;

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

    char* video_memory = (char*)VIDEO_MEMORY;
    int width = r->right - r->left;

	uint current_color = contexts[hdc].current_color;

    for (int y = r->top; y < r->bottom; y++) {
        // Calculate the memory address of the start of the row
        char* row = video_memory + y * SCREEN_WIDTH + r->left;

        // Set the entire row to the current color
        memset(row, current_color, width);
    }

	return 0;
}

int sys_beginpaint(int hwnd) {
    // For now, ignore hwnd as windows are not implemented

    // Find an available device context
    for (int i = 0; i < MAX_CONTEXTS; i++) {
        if (!contexts[i].used) {
            contexts[i].used = 1;  // Mark as used
            contexts[i].current_x = 0;  // Initialize values
            contexts[i].current_y = 0;
            contexts[i].current_color = DEFAULT_COLOR;
            return i;  // Return the index as the hdc
        }
    }
    return -1;  // No available context
}

int sys_endpaint(int hdc) {
    if (hdc < 0 || hdc >= MAX_CONTEXTS) {
        return -1;  // Invalid hdc
    }

    contexts[hdc].used = 0;  // Mark the context as free
    return 0;
}
