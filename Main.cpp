
#include "bmp.h"
#include "MyWindow.h"
#include <iostream>

int main(int argc, char* argv[])
{
    // Create a window for rendering and UI
    int width = 0, height = 0;
    unsigned char *data = loadbmp("lena128.bmp", &width, &height);

    printf("width = %d, height = %d\n", width, height);

/*    for (int j = 0; j < height; j++){
        for (int i = 0; i < width; i++){

            printf("%d %d %d\n", data[(j*height+i)*3], data[(j*height+i)*3+1], data[(j*height+i)*3+2]);
        }
    }
*/




    MyWindow window;

    // Create a grid with 64 by 64 cells
    MyWorld *world = new MyWorld(width, 0.2, 0.0, 0.0, data);

    // Link the world to the window and start running the glut event loop
    window.setWorld(world);
    glutInit(&argc, argv);
    window.initWindow(640, 640, "Fluid");

    std::cout << "Left click+drag: add density" << std::endl;
    std::cout << "Right click+drag: add velocity" << std::endl;
    std::cout << "'v': visualization density/velocity" << std::endl;
    std::cout << "space bar: simulation on/off" << std::endl;

    glutMainLoop();
    return 0;
}
