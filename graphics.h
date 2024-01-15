#include "types.h"

#define VIDEO_MEMORY_PHYS 0xA0000
#define VIDEO_MEMORY P2V(VIDEO_MEMORY_PHYS)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define DEFAULT_COLOR 15  // White
#define PALETTE_INDEX 0x3C8
#define PALETTE_DATA  0x3C9
#define MAX_CONTEXTS  2 // Maximum number of device contexts
#define MAX_GRAPHICS_COMMANDS 1024

struct rect {
 int top; // y co-ordinate of top of rectangle
 int left; // x co-ordinate of left of rectangle
 int bottom; // y co-ordinate of bottom of rectangle
 int right; // x co-ordinate of right of rectangle
};

enum command_type {
    CMD_SETPIXEL,
    CMD_MOVETO,
    CMD_LINETO,
    CMD_SETPENCOLOR,
    CMD_SELECTPEN,
    CMD_FILLRECT
    // Add other command types as needed
};

struct setpixel_params {
    int hdc, x, y;
};

struct moveto_params {
    int hdc, x, y;
};

struct lineto_params {
    int hdc, x, y;
};

struct setpencolor_params {
    int index, r, g, b;
};

struct selectpen_params {
    int hdc, index;
};

struct fillrect_params {
    int hdc;
    struct rect rect;
};

// Define a union for the parameters of different command types
union command_params {
    struct setpixel_params setpixel;
    struct moveto_params moveto;
    struct lineto_params lineto;
    struct setpencolor_params setpencolor;
    struct selectpen_params selectpen;
    struct fillrect_params fillrect;
    // Add other parameter structs as needed
};

struct graphics_command {
    enum command_type type;
    union command_params params;
};

struct device_context {
    int current_x;
    int current_y;
    uint current_color;
    int used;
    struct graphics_command commands[MAX_GRAPHICS_COMMANDS];
    int cmd_count;
};

struct device_context contexts[MAX_CONTEXTS];

struct graphics_command global_commands[MAX_GRAPHICS_COMMANDS];