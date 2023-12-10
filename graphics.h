#include "types.h"

#define VIDEO_MEMORY_PHYS 0xA0000
#define VIDEO_MEMORY P2V(VIDEO_MEMORY_PHYS)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define DEFAULT_COLOR 15  // White
#define PALETTE_INDEX 0x3C8
#define PALETTE_DATA  0x3C9
#define MAX_CONTEXTS  2 // Maximum number of device contexts

struct rect {
 int top; // y co-ordinate of top of rectangle
 int left; // x co-ordinate of left of rectangle
 int bottom; // y co-ordinate of bottom of rectangle
 int right; // x co-ordinate of right of rectangle
};

struct device_context {
    int current_x;
    int current_y;
    uint current_color;
    int used; 
};




