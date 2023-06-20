
#include "EngineHeader.h"

class GameApplication : public EngineApplication
{
public:
	GameApplication() {}
	~GameApplication() {}
};

int main(int argc, char** argv)
{
	LOG_ERROR("Game App Start...");

	GameApplication* game = new GameApplication();
	game->Run();

	LOG_ERROR("Game App End...");

	delete game;
	return 0;
} 