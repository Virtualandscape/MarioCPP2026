// Include the Game class which provides the main application lifecycle (initialization, run loop, shutdown).
#include "mario/game/MarioGame.hpp"

// Program entry point.
// Creates the game object, runs the main loop, performs shutdown, and returns an exit code.
int main()
{
    // Construct the game instance. The constructor should initialize resources.
    mario::Game game;

    // Run the main game loop. This should block until the game exits (e.g., window closed or game over).
    game.run();

    // Perform cleanup and release resources before exiting the process.
    game.shutdown();

    // Return zero to indicate successful execution to the operating system.
    return 0;
}
