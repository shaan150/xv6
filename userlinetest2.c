#include "types.h"
#include "user.h"


int main(int argc, char* argv[])
{
    setvideomode(0x13);

    int hdc = beginpaint(0);

    setpencolour(17, 0, 63, 0);
    for (int i = 20; i < 160; i+=20)
    {
        selectpen(hdc, 17);
        moveto(hdc, i, i);
        lineto(hdc, i, i + 20);
        lineto(hdc, i + 20, i + 20);
        lineto(hdc, i + 20, i);
        lineto(hdc, i, i);
    }

    endpaint(hdc);

    getch();
    setvideomode(0x03);
    exit();
}
