#include "UltimateEnginePCH.h"
#include "EngineHeader.h"

class GameApplication : public EngineApplication
{
public:
	GameApplication() = default;
	~GameApplication() override = default;
};

int main(int argc, char** argv)
{
	LOG_INFO("Game App Start...");

	GameApplication Game;
	Game.Initialize("The Ultimate Game Engine", 960, 540);
	Game.Run();

	LOG_INFO("Ending Game App...");

	return EXIT_SUCCESS;
} 