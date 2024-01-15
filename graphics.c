#include "graphics.h"
#include "defs.h"
#include "memlayout.h"
#include "x86.h"

int global_cmd_count = 0;

void clear320x200x256() {
    char* video_memory = (char*)VIDEO_MEMORY;
    for (int i = 0; i < 320 * 200; i++) {
        video_memory[i] = 0; // Black color
    }
}

static inline int clip(int num, int max) {
	if (num < 0) {
		return 0;
	} else if (num >= max) {
		return max - 1;
	} else {
		return num;
	}
}

static inline void plotpixel(int x, int y, uint color) {
	char* video_memory = (char*)VIDEO_MEMORY;
	video_memory[y * 320 + x] = color;
}

static inline int add_command_to_buffer(int hdc, struct graphics_command cmd) {
	// Add the command to the device context's command buffer
	if (contexts[hdc].cmd_count < MAX_GRAPHICS_COMMANDS) {
		contexts[hdc].commands[contexts[hdc].cmd_count++] = cmd;
	} else {
		return -1;  // Command buffer is full
	}
	return 0;
}

static inline int checkhdc(int hdc) {
	if (hdc < 0 || hdc >= MAX_CONTEXTS || !contexts[hdc].used) {
		return -1;
	}

	return 0;
}

int sys_setpixel(void) {
	int hdc, x, y;

	if(argint(0, &hdc) < 0 || checkhdc(hdc) < 0) {
		return -1;
	}

	argint(1, &x);
	argint(2, &y);

	x = clip(x, SCREEN_WIDTH);
	y = clip(y, SCREEN_HEIGHT);
	
	struct graphics_command cmd;
	cmd.type = CMD_SETPIXEL;
	cmd.params.setpixel.hdc = hdc;
	cmd.params.setpixel.x = x;
	cmd.params.setpixel.y = y;

	if(add_command_to_buffer(hdc, cmd) < 0) {
		return -1;
	}

	return 0;
}


int sys_moveto(void) {
	int hdc, x, y;
	if(argint(0, &hdc) < 0 || checkhdc(hdc) < 0) {
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

	struct graphics_command cmd;
	cmd.type = CMD_MOVETO;
	cmd.params.moveto.hdc = hdc;
	cmd.params.moveto.x = x;
	cmd.params.moveto.y = y;

	if(add_command_to_buffer(hdc, cmd) < 0) {
		return -1;
	}

	return 0;
}

static inline int abs(int n) {
    return (n >= 0) ? n : -n;
}

static void plotLineLow(int current_colour, int x0, int y0, int x1, int y1) {
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
		plotpixel(x, y, current_colour);
		if (D > 0) {
			y = y + yi;
			D += incrNE;
		}
		D += incrE;
	}
}

static void plotLineHigh(int current_colour, int x0, int y0, int x1, int y1) {
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
		plotpixel(x, y, current_colour);
		if (D > 0) {
			x = x + xi;
			D += incrNE;
		}
		D += incrE;
	}
}

static void createline(uint current_colour, int x0, int y0, int x1, int y1) {

	// Calculate the absolute differences between the x and y coordinates.
	int abs_x = abs(x1 - x0);
	int abs_y = abs(y1 - y0);

	// Determine whether the line is closer to being horizontal or vertical.
	if(abs_y < abs_x) {
		// If the line is closer to being horizontal, call the plotLineLow function.
		if (x0 > x1) {
			plotLineLow(current_colour, x1, y1, x0, y0);
		}
		else {
			plotLineLow(current_colour, x0, y0, x1, y1);
		}
	}
	else {
		// If the line is closer to being vertical, call the plotLineHigh function.
		if (y0 > y1) {
			plotLineHigh(current_colour, x1, y1, x0, y0);
		}
		else {
			plotLineHigh(current_colour, x0, y0, x1, y1);
		}
	}
}

int sys_lineto(void) {
	int hdc, x, y;

	// Retrieve the hdc (handle to device context) from the first argument.
	// If the argument retrieval fails or the hdc is invalid, return -1.
	if(argint(0, &hdc) < 0 || checkhdc(hdc) < 0) {
		return -1;
	}

	// Retrieve the x-coordinate from the second argument.
	// If the argument retrieval fails, return -1.
	if (argint(1, &x) < 0) {
		return -1;
	}

	// Retrieve the y-coordinate from the third argument.
	// If the argument retrieval fails, return -1.
	if (argint(2, &y) < 0) {
		return -1;
	}

	// Clip the x-coordinate to ensure it is within the screen width.
	x = clip(x, SCREEN_WIDTH);

	// Clip the y-coordinate to ensure it is within the screen height.
	y = clip(y, SCREEN_HEIGHT);

	// Create a graphics command structure to store the command type (CMD_LINETO) and parameters.
	struct graphics_command cmd;
	cmd.type = CMD_LINETO;
	cmd.params.lineto.hdc = hdc;
	cmd.params.lineto.x = x;
	cmd.params.lineto.y = y;

	// Add the graphics command to the command buffer for the specified hdc.
	// If the addition fails, return -1.
	if(add_command_to_buffer(hdc, cmd) < 0) {
		return -1;
	}

	// Return 0 to indicate success.
	return 0;
}

static void set_palette_color(int index, int r, int g, int b) {
	outb(PALETTE_INDEX, index);
	outb(PALETTE_DATA, r >> 2);  // VGA expects 6-bit values
	outb(PALETTE_DATA, g >> 2);
	outb(PALETTE_DATA, b >> 2);
}


int sys_setpencolour(void) {
	int index, r, g, b;

	// Retrieve the index value from the first argument.
	// If the argument retrieval fails or the index is not within the valid range, return -1.
	if (argint(0, &index) < 0 || index < 16 || index > 255) {
		return -1;
	}

	// Retrieve the r, g, and b values from the second, third, and fourth arguments, respectively.
	// If any of the argument retrievals fail, return -1.
	if (argint(1, &r) < 0 || argint(2, &g) < 0 || argint(3, &b) < 0) {
		return -1;
	}

	// Clip the r, g, and b values to ensure they are within the valid range of 0 to 63.
	r = clip(r, 64);
	g = clip(g, 64);
	b = clip(b, 64);

	// Create a graphics command structure to store the command type and parameters.
	struct graphics_command cmd;
	cmd.type = CMD_SETPENCOLOR;
	cmd.params.setpencolor.index = index;
	cmd.params.setpencolor.r = r;
	cmd.params.setpencolor.g = g;
	cmd.params.setpencolor.b = b;

	// Check if the global command count is less than the maximum graphics commands.
	// If it is, add the command to the global commands array.
	// Otherwise, return -1 to indicate that the command buffer is full.
	if (global_cmd_count < MAX_GRAPHICS_COMMANDS) {
		global_commands[global_cmd_count++] = cmd;
	} else {
		return -1;  // Command buffer is full
	}

	return 0;
}


int sys_selectpen(void) {
    int index, hdc;
	if(argint(0, &hdc) < 0 || checkhdc(hdc) < 0) {
		return -1;
	}

    if (argint(1, &index) < 0 || index > 255) {
        return -1;
    }

	struct graphics_command cmd;
	cmd.type = CMD_SELECTPEN;
	cmd.params.selectpen.hdc = hdc;
	cmd.params.selectpen.index = index;

	if(add_command_to_buffer(hdc, cmd) < 0) {
		return -1;
	}

	uint previous_color = contexts[hdc].current_color;

	return previous_color;
}

static void createrect(uint current_colour, struct rect *r) {
	char* video_memory = (char*)VIDEO_MEMORY;
	int width = r->right - r->left;

	for (int y = r->top; y < r->bottom; y++) {
		// Calculate the memory address of the start of the row
		char* row = video_memory + y * SCREEN_WIDTH + r->left;

		// Set the entire row to the current color
		memset(row, current_colour, width);
	}
}

int sys_fillrect(void) {
	int hdc;
	struct rect *r;

	// Check and retrieve HDC parameter
	if (argint(0, &hdc) < 0 || checkhdc(hdc) < 0) {
		return -1;
	}

	// Check and retrieve rectangle parameter
	if (argptr(1, (void*)&r, sizeof(*r)) < 0) {
		return -1;
	}

	// Clip rectangle boundaries to fit within screen dimensions
	r->top = clip(r->top, SCREEN_HEIGHT);
	r->left = clip(r->left, SCREEN_WIDTH);
	r->bottom = clip(r->bottom, SCREEN_HEIGHT);
	r->right = clip(r->right, SCREEN_WIDTH);

	// Create graphics command structure and add it to the command buffer
	struct graphics_command cmd;
	cmd.type = CMD_FILLRECT;
	cmd.params.fillrect.hdc = hdc;
	cmd.params.fillrect.rect = *r;

	if (add_command_to_buffer(hdc, cmd) < 0) {
		return -1;
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
			contexts[i].cmd_count = 0;
			return i;  // Return the index as the hdc
		}
	}
	return -1;  // No available context
}

int sys_endpaint(void) {
	int hdc;

	// Retrieve the hdc parameter from the user
	if (argint(0, &hdc) < 0 || checkhdc(hdc) < 0) {
		return -1;
	}

	// Execute the global commands stored in the global command buffer
	for (int i = 0; i < global_cmd_count; i++) {
		struct graphics_command *cmd = &global_commands[i];

		switch (cmd->type) {
			case CMD_SETPENCOLOR:
				// Set the pen color using the set_palette_color function
				set_palette_color(cmd->params.setpencolor.index, cmd->params.setpencolor.r, cmd->params.setpencolor.g, cmd->params.setpencolor.b);
				break;
			default:
				return -1;  // Invalid command type
		}
	}

	// Clear the global command buffer
	global_cmd_count = 0;

	// Retrieve the device context associated with the given hdc
	struct device_context *ctx = &contexts[hdc];

	// Execute the commands stored in the device context's command buffer
	for (int i = 0; i < ctx->cmd_count; i++) {
		struct graphics_command *cmd = &ctx->commands[i];

		switch (cmd->type) {
			case CMD_SELECTPEN:
				// Select the pen color
				ctx->current_color = cmd->params.selectpen.index;
				break;

			case CMD_SETPIXEL:
				// Plot a pixel and update the current position
				plotpixel(cmd->params.setpixel.x, cmd->params.setpixel.y, ctx->current_color);
				ctx->current_x = cmd->params.setpixel.x;
				ctx->current_y = cmd->params.setpixel.y;
				break;

			case CMD_MOVETO:
				// Move the current position
				ctx->current_x = cmd->params.moveto.x;
				ctx->current_y = cmd->params.moveto.y;
				break;

			case CMD_LINETO:
				// Draw a line and update the current position
				createline(ctx->current_color, ctx->current_x, ctx->current_y, cmd->params.lineto.x, cmd->params.lineto.y);
				ctx->current_x = cmd->params.lineto.x;
				ctx->current_y = cmd->params.lineto.y;
				break;

			case CMD_FILLRECT:
				// Fill a rectangle with the current color
				createrect(ctx->current_color, &cmd->params.fillrect.rect);
				break;

			default:
				return -1;  // Invalid command type
		}
	}

	// Clear the command buffer in the device context
	ctx->cmd_count = 0;

	// Mark the device context as unused
	ctx->used = 0;

	return 0;
}