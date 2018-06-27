#include <iostream>

#include "app_povi.h"

int main(void)
{
    std::cout << "start." << std::endl;
    poviApp a(1280, 800, "test-app");
    a.run();
}
