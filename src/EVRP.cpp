#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<cstring>
#include<math.h>
#include<fstream>
#include<sstream>
#include<limits.h>
#include <set>
#include <algorithm>

#include "EVRP.hpp"
#include "utils.hpp"

using namespace std;

char *problem_instance;          //Name of the instance
struct node *node_list;     //List of nodes with id and x and y coordinates
int *cust_demand;                //List with id and customer demands
bool *charging_station;
double **distances;              //Distance matrix
int problem_size;                //Problem dimension read
double energy_consumption;

int DEPOT;                       //depot id (usually 0)
int NUM_OF_CUSTOMERS;       //Number of customers (excluding depot)
int ACTUAL_PROBLEM_SIZE;        //Total number of customers, charging stations and depot
double OPTIMUM;
int NUM_OF_STATIONS;
double BATTERY_CAPACITY;    //maximum energy of vehicles
int MAX_CAPACITY;           //capacity of vehicles

vector<int> CUSTOMERS;           // vector of customers
vector<int> AFSs;                // vector of AFSs, including the depot
map<int, int> afsIdMap;          // mapping: AFS id in EVRP -> AFS id in fw planner
floydWarshall fw;

double evals;
double current_best;

namespace {

string trim(const string &value) {
    const size_t start = value.find_first_not_of(" \t\r\n");
    if (start == string::npos) {
        return "";
    }

    const size_t end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

string first_token(const string &line) {
    istringstream iss(line);
    string token;
    iss >> token;
    if (!token.empty() && token.back() == ':') {
        token.pop_back();
    }
    return token;
}

bool is_section_marker(const string &token) {
    return token == "NODE_COORD_SECTION" ||
           token == "DEMAND_SECTION" ||
           token == "STATIONS_COORD_SECTION" ||
           token == "DEPOT_SECTION" ||
           token == "EOF";
}

string value_after_colon(const string &line) {
    const size_t colon = line.find(':');
    if (colon == string::npos) {
        return "";
    }

    return trim(line.substr(colon + 1));
}

}

/****************************************************************/
/*Compute and return the euclidean distance of two objects      */
/****************************************************************/
double euclidean_distance(int i, int j) {
    double xd, yd;
    double r = 0.0;
    xd = node_list[i].x - node_list[j].x;
    yd = node_list[i].y - node_list[j].y;
    r = sqrt(xd * xd + yd * yd);
    return r;
}

/****************************************************************/
/*Compute the distance matrix of the problem instance           */
/****************************************************************/
void compute_distances(void) {
    int i, j;
    for (i = 0; i < ACTUAL_PROBLEM_SIZE; i++) {
        for (j = 0; j < ACTUAL_PROBLEM_SIZE; j++) {
            distances[i][j] = euclidean_distance(i, j);
        }
    }
}


/****************************************************************/
/*Generate and return a two-dimension array of type double      */
/****************************************************************/
double **generate_2D_matrix_double(int n, int m) {
    double **matrix;

    matrix = new double *[n];
    for (int i = 0; i < n; i++) {
        matrix[i] = new double[m];
    }
    //initialize the 2-d array
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            matrix[i][j] = 0.0;
        }
    }
    return matrix;
}


/****************************************************************/
/* Read the problem instance and generate the initial object    */
/* vector.                                                      */
/****************************************************************/
void read_problem(const char *filename) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        cout << "wrong problem instance file" << endl;
        exit(1);
    }

    vector<node> parsed_nodes;
    vector<pair<int, int>> demand_entries;
    vector<int> station_entries;
    string line;
    string buffered_line;
    bool has_buffered_line = false;
    int declared_dimension = 0;
    int declared_stations = 0;

    ACTUAL_PROBLEM_SIZE = 0;
    NUM_OF_CUSTOMERS = 0;
    NUM_OF_STATIONS = 0;
    problem_size = 0;
    DEPOT = -1;

    auto next_line = [&](string &out_line) -> bool {
        if (has_buffered_line) {
            out_line = buffered_line;
            has_buffered_line = false;
            buffered_line.clear();
            return true;
        }

        return static_cast<bool>(getline(fin, out_line));
    };

    while (next_line(line)) {
        const string token = first_token(line);
        if (token.empty()) {
            continue;
        }

        if (token == "DIMENSION") {
            istringstream iss(value_after_colon(line));
            if (!(iss >> declared_dimension)) {
                cout << "DIMENSION error" << endl;
                exit(0);
            }
        } else if (token == "EDGE_WEIGHT_TYPE" || token == "EDGE_WEIGHT_FORMAT") {
            const string weight_type = value_after_colon(line);
            if (weight_type.empty()) {
                cout << "EDGE_WEIGHT_TYPE error" << endl;
                exit(0);
            }
            if (weight_type != "EUC_2D") {
                cout << "not EUC_2D" << endl;
                exit(0);
            }
        } else if (token == "CAPACITY") {
            istringstream iss(value_after_colon(line));
            if (!(iss >> MAX_CAPACITY)) {
                cout << "CAPACITY error" << endl;
                exit(0);
            }
        } else if (token == "ENERGY_CAPACITY") {
            istringstream iss(value_after_colon(line));
            if (!(iss >> BATTERY_CAPACITY)) {
                cout << "ENERGY_CAPACITY error" << endl;
                exit(0);
            }
        } else if (token == "ENERGY_CONSUMPTION") {
            istringstream iss(value_after_colon(line));
            if (!(iss >> energy_consumption)) {
                cout << "ENERGY_CONSUMPTION error" << endl;
                exit(0);
            }
        } else if (token == "STATIONS") {
            istringstream iss(value_after_colon(line));
            if (!(iss >> declared_stations)) {
                cout << "STATIONS error" << endl;
                exit(0);
            }
        } else if (token == "OPTIMAL_VALUE") {
            istringstream iss(value_after_colon(line));
            if (!(iss >> OPTIMUM)) {
                cout << "OPTIMAL_VALUE error" << endl;
                exit(0);
            }
        } else if (token == "NODE_COORD_SECTION") {
            while (getline(fin, line)) {
                const string section_token = first_token(line);
                if (section_token.empty()) {
                    continue;
                }
                if (is_section_marker(section_token)) {
                    buffered_line = line;
                    has_buffered_line = true;
                    break;
                }

                istringstream iss(line);
                node parsed_node;
                if (!(iss >> parsed_node.id >> parsed_node.x >> parsed_node.y)) {
                    cout << "wrong problem instance file" << endl;
                    exit(1);
                }
                parsed_node.id -= 1;
                parsed_nodes.push_back(parsed_node);
            }
        } else if (token == "DEMAND_SECTION") {
            while (getline(fin, line)) {
                const string section_token = first_token(line);
                if (section_token.empty()) {
                    continue;
                }
                if (is_section_marker(section_token)) {
                    buffered_line = line;
                    has_buffered_line = true;
                    break;
                }

                istringstream iss(line);
                int node_id;
                int demand;
                if (!(iss >> node_id >> demand)) {
                    cout << "wrong problem instance file" << endl;
                    exit(1);
                }
                demand_entries.emplace_back(node_id, demand);
            }
        } else if (token == "STATIONS_COORD_SECTION") {
            while (getline(fin, line)) {
                const string section_token = first_token(line);
                if (section_token.empty()) {
                    continue;
                }
                if (is_section_marker(section_token)) {
                    buffered_line = line;
                    has_buffered_line = true;
                    break;
                }

                istringstream iss(line);
                int node_id;
                if (!(iss >> node_id)) {
                    cout << "wrong problem instance file" << endl;
                    exit(1);
                }
                station_entries.push_back(node_id);
            }
        } else if (token == "DEPOT_SECTION") {
            while (getline(fin, line)) {
                const string section_token = first_token(line);
                if (section_token.empty()) {
                    continue;
                }
                if (section_token == "EOF") {
                    break;
                }

                istringstream iss(line);
                int depot_id;
                if (!(iss >> depot_id)) {
                    cout << "wrong problem instance file" << endl;
                    exit(1);
                }
                if (depot_id == -1) {
                    break;
                }
                DEPOT = depot_id - 1;
            }
        }
    }

    fin.close();

    if (parsed_nodes.empty() || demand_entries.empty()) {
        cout << "wrong problem instance file" << endl;
        exit(1);
    }

    ACTUAL_PROBLEM_SIZE = static_cast<int>(parsed_nodes.size());
    problem_size = static_cast<int>(demand_entries.size());
    NUM_OF_CUSTOMERS = problem_size - 1;
    NUM_OF_STATIONS = station_entries.empty() ? (ACTUAL_PROBLEM_SIZE - problem_size)
                                              : static_cast<int>(station_entries.size());

    const bool dimension_matches = declared_dimension == 0 ||
                                   declared_dimension == problem_size ||
                                   declared_dimension == ACTUAL_PROBLEM_SIZE;
    if (!dimension_matches ||
        declared_stations != 0 && declared_stations != NUM_OF_STATIONS ||
        ACTUAL_PROBLEM_SIZE != problem_size + NUM_OF_STATIONS ||
        DEPOT < 0 || DEPOT >= ACTUAL_PROBLEM_SIZE) {
        cout << "wrong problem instance file" << endl;
        exit(1);
    }

    node_list = new node[ACTUAL_PROBLEM_SIZE];
    for (int i = 0; i < ACTUAL_PROBLEM_SIZE; i++) {
        node_list[i] = parsed_nodes[i];
    }

    cust_demand = new int[ACTUAL_PROBLEM_SIZE];
    charging_station = new bool[ACTUAL_PROBLEM_SIZE];
    for (int i = 0; i < ACTUAL_PROBLEM_SIZE; i++) {
        cust_demand[i] = 0;
        charging_station[i] = false;
    }

    set<int> demand_node_ids;
    for (const auto &entry : demand_entries) {
        const int node_id = entry.first - 1;
        if (node_id < 0 || node_id >= ACTUAL_PROBLEM_SIZE) {
            cout << "wrong problem instance file" << endl;
            exit(1);
        }
        demand_node_ids.insert(node_id);
        cust_demand[node_id] = entry.second;
    }

    if (station_entries.empty()) {
        for (int i = 0; i < ACTUAL_PROBLEM_SIZE; i++) {
            if (!demand_node_ids.count(i)) {
                charging_station[i] = true;
            }
        }
    } else {
        for (const int station_entry : station_entries) {
            const int node_id = station_entry - 1;
            if (node_id < 0 || node_id >= ACTUAL_PROBLEM_SIZE) {
                cout << "wrong problem instance file" << endl;
                exit(1);
            }
            charging_station[node_id] = true;
        }
    }

    charging_station[DEPOT] = true;
    distances = generate_2D_matrix_double(ACTUAL_PROBLEM_SIZE, ACTUAL_PROBLEM_SIZE);
    compute_distances();
}


/****************************************************************/
/* Returns the solution quality of the solution. Taken as input */
/* an array of node indeces and its length                      */
/****************************************************************/
double fitness_evaluation(int *routes, int size) {
    int i;
    double tour_length = 0.0;

    //the format of the solution that this method evaluates is the following
    //Node id:  0 - 5 - 6 - 8 - 0 - 1 - 2 - 3 - 4 - 0 - 7 - 0
    //Route id: 1 - 1 - 1 - 1 - 2 - 2 - 2 - 2 - 2 - 3 - 3 - 3
    //this solution consists of three routes:
    //Route 1: 0 - 5 - 6 - 8 - 0
    //Route 2: 0 - 1 - 2 - 3 - 4 - 0
    //Route 3: 0 - 7 - 0
    for (i = 0; i < size - 1; i++)
        tour_length += distances[routes[i]][routes[i + 1]];

    if (tour_length < current_best)
        current_best = tour_length;

    //adds complete evaluation to the overall fitness evaluation count
    evals++;

    return tour_length;
}

/****************************************************************/
/* Outputs the routes of the solution. Taken as input           */
/* an array of node indeces and its length                      */
/****************************************************************/
void print_solution(int *routes, int size) {
    int i;

    for (i = 0; i < size; i++) {
        cout << routes[i] << " , ";
    }
    cout << endl;
}


/****************************************************************/
/* Validates the routes of the solution. Taken as input         */
/* an array of node indeces and its length                      */
/****************************************************************/
bool check_solution(int *t, int size) {
    int i, from, to;
    double energy_temp = BATTERY_CAPACITY;
    double capacity_temp = MAX_CAPACITY;
    double distance_temp = 0.0;

    for (i = 0; i < size - 1; i++) {
        from = t[i];
        to = t[i + 1];
        capacity_temp -= get_customer_demand(to);
        energy_temp -= get_energy_consumption(from, to);
        distance_temp += get_distance(from, to);
        if (capacity_temp < 0.0) {
            cout << "error: capacity below 0 at customer " << to << endl;
            print_solution(t, size);
//            exit(1);
            return false;
        }
        if (energy_temp < 0.0) {
            cout << "error: energy below 0 from " << from << " to " << to << endl;
            cout << energy_temp << endl;
            print_solution(t, size);
//            exit(1);
            return false;
        }
        if (to == DEPOT) {
            capacity_temp = MAX_CAPACITY;
        }
        if (is_charging_station(to) == true || to == DEPOT) {
            energy_temp = BATTERY_CAPACITY;
        }
    }
    if (distance_temp != fitness_evaluation(t, size)) {
        cout << "error: check fitness evaluation" << endl;
        return false;
    }

    return true;
}


/****************************************************************/
/* Returns the distance between two points: from and to. Can be */
/* used to evaluate a part of the solution. The fitness         */
/* evaluation count will be proportional to the problem size    */
/****************************************************************/
double get_distance(int from, int to) {
    //adds partial evaluation to the overall fitness evaluation count
    //It can be used when local search is used and a whole evaluation is not necessary
    evals += (1.0 / ACTUAL_PROBLEM_SIZE);

    return distances[from][to];

}


/****************************************************************/
/* Returns the energy consumed when travelling between two      */
/* points: from and to.                                         */
/****************************************************************/
double get_energy_consumption(int from, int to) {

    return energy_consumption * distances[from][to];

}

/****************************************************************/
/* Returns the demand for a specific customer                   */
/* points: from and to.                                         */
/****************************************************************/
int get_customer_demand(int customer) {

    return cust_demand[customer];

}

/****************************************************************/
/* Returns true when a specific node is a charging station;     */
/* and false otherwise                                          */
/****************************************************************/
bool is_charging_station(int node) {

    bool flag = false;
    if (charging_station[node] == true)
        flag = true;
    else
        flag = false;
    return flag;

}

/****************************************************************/
/* Returns the best solution quality so far                     */
/****************************************************************/
double get_current_best() {

    return current_best;

}

/*******************************************************************/
/* Reset the best solution quality so far for a new indepedent run */
/*******************************************************************/
void init_current_best() {

    current_best = INT_MAX;

}

/****************************************************************/
/* Returns the current count of fitness evaluations             */
/****************************************************************/
double get_evals() {

    return evals;

}

/****************************************************************/
/* Reset the evaluation counter for a new indepedent run        */
/****************************************************************/
void init_evals() {

    evals = 0;

}



/****************************************************************/
/* Clear the allocated memory                                   */
/****************************************************************/
void free_EVRP() {

    int i;

    delete[] node_list;
    delete[] cust_demand;
    delete[] charging_station;

    for (i = 0; i < ACTUAL_PROBLEM_SIZE; i++) {
        delete[] distances[i];
    }

    delete[] distances;

}


/****************************************************************/
/* Initialize additional structures added to the EVRP.hpp       */
/****************************************************************/
void initMyStructures() {
    // init AFSs and CUSTOMERS vectors
    AFSs.clear();
    CUSTOMERS.clear();
    int afsId = 0;
    for (int i = 0; i < ACTUAL_PROBLEM_SIZE; i++) {
        if (is_charging_station(i)) {
            AFSs.push_back(i);
            afsIdMap.insert(pair<int, int>(i, afsId++));
        } else {
            CUSTOMERS.push_back(i);
        }
    }


    // Initialize instance of fw planner
    fw = floydWarshall(AFSs.size());
    fw.planPaths();

}

/****************************************************************/
/* Get energy to unit                                           */
/****************************************************************/
double get_energy_per_unit(){
    return energy_consumption;
}


/****************************************************************/
/* Overloaded fitness evaluation for vector representation      */
/****************************************************************/
double fitness_evaluation(vector<int> &tour) {
    double tour_length = 0;
    for (int i = 0; i < tour.size() - 1; i++) {
        tour_length += distances[tour[i]][tour[i + 1]];
    }

    if (tour_length < current_best)
        current_best = tour_length;

    evals++;

    return tour_length;
}

bool full_validity_check(vector<int> &tour) {
    cout << "VALIDITY CHECK\n";
    auto test1 = isValidTour(tour);
    cout << "My test: " << test1 << endl;

    best_sol = new solution;
    best_sol->tour = new int[4*NUM_OF_CUSTOMERS];
    best_sol->id = 1;
    best_sol->steps = 0;
    best_sol->tour_length = INT_MAX;
    for (int i = 0; i < tour.size(); i++) {
        best_sol->tour[i] = tour[i];
    }
    best_sol->steps = tour.size();
    auto test2 = check_solution(best_sol->tour, best_sol->steps);
    cout << "Provided test: " << test2 << endl;

    cout << "FITNESS\n";
    cout << "My fitness: " << fitness_evaluation(tour) << endl;
    cout << "Provided fitness: " << fitness_evaluation(best_sol->tour, best_sol->steps) << endl;

    std::set<int> customers;
    for (auto c:tour) {
        if (!is_charging_station(c)) {
            customers.insert(c);
        }
    }
    cout << "Customers served: " << customers.size() << "/" << NUM_OF_CUSTOMERS << endl;
    bool test3 = (customers.size() == NUM_OF_CUSTOMERS);

    return (test1 and test2 and test3);
}
