//Developed by AdeptusXIII(c). All Rights Reserved. UCC 1-308 "Without Prejudice".

#include "Engine/Engine.h"

int main()
{
    MEngine Engine;
    Engine.Init();
    //Engine.RunMainLoop();
    Engine.RunCommandLoop();

    return 0;
}
