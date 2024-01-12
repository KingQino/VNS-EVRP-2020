#ifndef EVRP_STATS_HPP
#define EVRP_STATS_HPP

#include <filesystem>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#define MAX_TRIALS 	20 									//DO NOT CHANGE THE NUMBER
#define CHAR_LEN 100

void open_stats(void);									//creates the output file
void close_stats(void);									//stores the best values for each run
void get_mean(int r, double value);						//stores the observation from each run
void free_stats();										//free memory


namespace fs = std::filesystem;


struct PopulationMetrics {
    double min{};
    double max{};
    double avg{};
    double std{};
    std::size_t size{}; // population size
};

class StatsInterface {
public:
    static const std::string statsPath;

    static PopulationMetrics calculate_population_metrics(const std::vector<double>& data) ;
    static bool create_directories_if_not_exists(const std::string& directoryPath);
    static void stats_for_multiple_trials(const std::string& filePath, const std::vector<double>& data); // open a file, save the statistical info, and then close it

    virtual ~StatsInterface() = default;
};

#endif //EVRP_STATS_HPP
