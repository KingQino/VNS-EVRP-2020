#include<iostream>
#include<cstdlib>
#include <fstream>


#include "EVRP.hpp"
#include "heuristics.hpp"
#include "stats.hpp"
#include "perturbations.hpp"

using namespace std;


/*initialiazes a run for your heuristic*/
void start_run(int r) {

    srand(r); //random seed
    init_evals();
    init_current_best();
    SEED = r;
    cout << "Run: " << r << " with random seed " << r << endl;
}

/*gets an observation of the run for your heuristic*/
void end_run(int r) {

//    get_mean(r - 1, get_current_best()); //from stats.h
    cout << "End of run " << r << " with best solution quality " << get_current_best() << " total evaluations: "
         << get_evals() << endl;
    cout << " " << endl;
}

/*sets the termination conidition for your heuristic*/
bool termination_condition(void) {

    bool flag;
    if (get_evals() >= TERMINATION)
        flag = true;
    else
        flag = false;

    return flag;
}


/****************************************************************/
/*                Main Function                                 */
/****************************************************************/
//int main(int argc, char *argv[]) {
//    int run;
//    if (argc < 3) {
//        std::cerr << "Usage: " << argv[0] << " <problem_instance_filename> <seed>\n";
//        return 1; // Exit with an error code
//    }
//
//    std::string instanceName(argv[1]);
//    run = std::stoi(argv[2]);
//
//    problem_instance = argv[1];       //pass the .evrp filename as an argument
//    read_problem(problem_instance);   //Read EVRP from file from EVRP.h
//
//
//    string instancePrefix = instanceName.substr(0, instanceName.find_last_of('.'));
//    string directoryPath = StatsInterface::statsPath+ "/" + instancePrefix  + "/" + to_string(run);
//
//    start_run(run);
//    initialize_heuristic(); //heuristic.h
//    run_heuristic();  //heuristic.h
//    end_run(run);  //store the best solution quality for each run
//
//
//    free_stats();
//    free_heuristic();
//    free_EVRP();
//
//    return 0;
//}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <problem_instance_filename>\n";
        return 1; // Exit with an error code
    }

    std::string instanceName(argv[1]);

    vector<double> data;
    string instancePrefix = instanceName.substr(0, instanceName.find_last_of('.'));
    string directoryPrefix = StatsInterface::statsPath+ "/" + instancePrefix  + "/";
    for (int run = 1; run <= 20; ++run) {
        string directoryPath = directoryPrefix + to_string(run);

        // Construct the filename based on the run number
        string filename = "solution." + to_string(run) + "." + instancePrefix + ".txt";
        string filepath = directoryPath + "/" + filename;

        // Open the file
        ifstream infile(filepath);

        if (infile) {
            double value;
            // Read the first line from the file and convert it to double
            if (infile >> value) {
                // Push the value to the data vector
                data.push_back(value);
                cout << "Run " << run << ": Value read from file " << filename << ": " << value << endl;
            } else {
                cerr << "Error reading value from file: " << filename << endl;
            }
            infile.close();
        } else {
            cerr << "Error opening file: " << filename << endl;
        }
    }

    string statsFileName = directoryPrefix + "stats." + instancePrefix + ".txt";
    StatsInterface::stats_for_multiple_trials(directoryPrefix, data);


    return 0;
}