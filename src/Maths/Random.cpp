#include "Random.hpp"
#include <random>
#include <thread>

double Random::random(const double& min, const double& max) {
    // Initialisé une seule fois par thread avec une graine unique dérivée de l'ID du thread
    thread_local std::mt19937 generator(
        static_cast<unsigned int>(std::hash<std::thread::id>{}(std::this_thread::get_id()))
    );

    std::uniform_real_distribution<double> distribution(min, max);
    return distribution(generator);
}