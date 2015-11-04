#include "app.h"
// #include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <iostream>
// #include "Shader.h"



int main(void)
{
    std::cout << "start." << std::endl;
    App a = App(800, 600, "test-app");
    a.run();
}
