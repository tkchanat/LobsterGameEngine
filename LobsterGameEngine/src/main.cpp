#include "pch.h"
#include "Application.h"

//	Entry point of the program.
//	Please move to Application.cpp to see the main game loop.
int main()
{
    Lobster::Application app;
    app.Initialize();
    app.Run();
    app.Shutdown();
    return 0;
}
