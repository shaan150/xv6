#include "graphics.h"
#include "types.h"
#include "user.h"


int main(int argc, char* argv[])
{
 setvideomode(0x13);
 setpencolour(18, 45, 0, 0);
 selectpen(0, 25 );
 struct rect rectangle = { 10, 10, 100, 100 };
 fillrect(0, &rectangle);
 getch();
 setvideomode(0x03);
 exit();

}
