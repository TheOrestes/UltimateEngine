
#include "UltimateEnginePCH.h"
#include "EngineHeader.h"

class GameApplication : public EngineApplication
{
public:
	GameApplication() {}
	virtual ~GameApplication() {}

private:

};

int main(int argc, char** argv)
{
	LOG_DEBUG("Game App Start...");

	GameApplication Game;
	Game.Initialize("The Ultimate Game Engine");
	Game.Run();

	LOG_DEBUG("Game App End...");

	return EXIT_SUCCESS;
} 