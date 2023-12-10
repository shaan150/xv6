#include "graphics.h"
#include "types.h"
#include "user.h"

struct rect rectangle = { 0, 0, 0, 0 };

int main(int argc, char* argv[])
{
    setvideomode(0x13);
    int hdc = 0;

    for (int i = 0; i < 50; i++)
    {
        setpencolour(i + 20, i * 3, i * 2, i);
        selectpen(hdc, i + 5);
        rectangle.top = i + 15;
        rectangle.left = i + 15;
        rectangle.bottom = i + 40;
        rectangle.right = i + 40;
        fillrect(hdc, &rectangle);
    }


    getch();
    setvideomode(0x03);
    exit();

}
