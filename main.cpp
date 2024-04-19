#include "Editor.h"
#include "TestRendering.h"
// Project DIR,use as the root of the project
#ifndef PROJECT_DIR
#define PROJECT_DIR ""
#endif

int main(int argc, char *argv[])
{
    GEditor.Init(argv[0], PROJECT_DIR);
	GEditor.LoadWorld(TestRendering());
    GEditor.Start();
}
