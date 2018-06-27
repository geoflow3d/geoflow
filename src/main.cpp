#include <iostream>

#include "app_povi.h"

int main(void)
{
    std::cout << "start." << std::endl;
    poviApp a(800, 600, "test-app");
    a.run();
}
