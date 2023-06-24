
#include "EngineHeader.h"

class GameApplication : public EngineApplication
{
public:
	GameApplication() {}
	~GameApplication() {}

private:

};

int main(int argc, char** argv)
{
	LOG_INFO("Game App Start...");

	GameApplication Game;
	Game.Initialize();
	Game.Run();
	Game.Destroy();

	LOG_INFO("Game App End...");

	return EXIT_SUCCESS;
} 