#include "types.h"
#include "user.h"


void drawLines(int color, int offset) {
    int hdc = beginpaint(0);
    setpencolour(color, (color == 16) ? 0 : 0, (color == 16) ? 0 : 63, (color == 16) ? 63 : 0);
    for (int i = 20; i < 160; i += 20) {
        selectpen(hdc, color);
        moveto(hdc, i, i + offset);
        lineto(hdc, i, i + 20 + offset);
        lineto(hdc, i + 20, i + 20 + offset);
        lineto(hdc, i + 20, i + offset);
        lineto(hdc, i, i + offset);
    }
    endpaint(hdc);
}

int main(int argc, char* argv[]) {
    setvideomode(0x13);

    int pid = fork(); // Create a new process

    if (pid < 0) {
        // fork failed
        printf(1, "fork failed\n");
        exit();
    }

    if (pid == 0) {

        // Child process
        drawLines(17, 0);
    } else {
        // Parent process
        drawLines( 16, 20);

        // Wait for child process to finish
        wait();
    }

    getch();
    setvideomode(0x03);
    exit();
}