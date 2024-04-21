
#include "UltimateEnginePCH.h"
#include "EngineHeader.h"

class GameApplication : public EngineApplication
{
public:
	GameApplication() = default;
	~GameApplication() override = default;

private:

};

int main(int argc, char** argv)
{
	LOG_DEBUG("Game App Start...");

	GameApplication Game;
	Game.Initialize("The Ultimate Game Engine", 1920, 1080);
	Game.Run();

	LOG_DEBUG("Game App End...");

	return EXIT_SUCCESS;
} 