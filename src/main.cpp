#include "game.h"
#include <iostream>
#include <exception>

int main() {
    try {
        Game game;

        if (!game.initialize()) {
            std::cerr << "Failed to initialize game" << std::endl;
            return 1;
        }

        game.run();
        game.shutdown();

        std::cout << "Game exited successfully" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
