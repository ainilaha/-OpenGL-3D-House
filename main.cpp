#include <iostream>
#include "window.h"


using namespace std;

int main()
{
    Window window(1024, 768,"Window");
    window.Clear(0.1f, 0.2f, 0.2f, 1.0f);
    window.RenderWorld();

    return 0;
}
