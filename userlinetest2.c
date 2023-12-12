#include "types.h"
#include "user.h"


int main(int argc, char* argv[])
{
    setvideomode(0x13);

    int hdc = beginpaint(0);

    for (int i = 0; i < 20; i++)
    {
        moveto(hdc, i * 10, i * 5);
        setpencolour(i + 20, i * 3, i * 2, i);
        selectpen(hdc, i + 20);
        lineto(hdc, i * 10 + 20, i * 5);
        lineto(hdc, i * 10 + 20, i * 5 + 20);
        lineto(hdc, i * 10, i * 5 + 20);
        lineto(hdc, i * 10, i * 5);
    }

    endpaint(hdc);

    getch();
    setvideomode(0x03);
    exit();
}
