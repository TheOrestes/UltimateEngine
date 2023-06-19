
#include "EngineHeader.h"

class GameApplication : public EngineApplication
{
public:
	GameApplication() {}
	~GameApplication() {}
};

int main(int argc, char** argv)
{
	GameApplication* game = new GameApplication();
	game->Run();

	delete game;
	return 0;
}