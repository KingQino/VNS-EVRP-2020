#include<cmath>
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<cstring>
#include <iostream>
#include <algorithm>


#include "EVRP.hpp"
#include "stats.hpp"

using namespace std;

//Used to output offline performance and population diversity

FILE *log_performance;
//output files
char *perf_filename;

double* perf_of_trials;

void open_stats(void){
    //Initialize
    perf_of_trials = new double[MAX_TRIALS];

    for(int i =0; i < MAX_TRIALS; i++){
        perf_of_trials[i] = 0.0;
    }


  //initialize and open output files
  perf_filename = new char[CHAR_LEN];
  sprintf(perf_filename, "stats.%s.txt",
	 problem_instance);

  //for performance
  if ((log_performance = fopen(perf_filename,"a")) == NULL) {
      std::cout << "Failed to open stats file " << perf_filename << std::endl;
      exit(2);
  }
  //initialize and open output files

}


void get_mean(int r, double value) {

  perf_of_trials[r] = value;

}


double mean(double* values, int size){
  int i;
  double m = 0.0;
  for (i = 0; i < size; i++){
      m += values[i];
  }
  m = m / (double)size;
  return m; //mean
}

double stdev(double* values, int size, double average){
  int i;
  double dev = 0.0;

  if (size <= 1)
    return 0.0;
  for (i = 0; i < size; i++){
    dev += ((double)values[i] - average) * ((double)values[i] - average);
  }
  return sqrt(dev / (double)(size - 1)); //standard deviation
}

double best_of_vector(double *values, int l ) {
  double min;
  int k;
  k = 0;
  min = values[k];
  for( k = 1 ; k < l ; k++ ) {
    if( values[k] < min ) {
      min = values[k];
    }
  }
  return min;
}


double worst_of_vector(double *values, int l ) {
  double max;
  int k;
  k = 0;
  max = values[k];
  for( k = 1 ; k < l ; k++ ) {
    if( values[k] > max ){
      max = values[k];
    }
  }
  return max;
}



void close_stats(void){
  int i,j;
  double perf_mean_value, perf_stdev_value;
 
  //For statistics
  for(i = 0; i < MAX_TRIALS; i++){
    //cout << i << " " << perf_of_trials[i] << endl;
    //cout << i << " " << time_of_trials[i] << endl;
    fprintf(log_performance, "%.2f", perf_of_trials[i]);
    fprintf(log_performance,"\n");

  }

  perf_mean_value = mean(perf_of_trials,MAX_TRIALS);
  perf_stdev_value = stdev(perf_of_trials,MAX_TRIALS,perf_mean_value);
  fprintf(log_performance,"Mean %f\t ",perf_mean_value);
  fprintf(log_performance,"\tStd Dev %f\t ",perf_stdev_value);
  fprintf(log_performance,"\n");
  fprintf(log_performance, "Min: %f\t ", best_of_vector(perf_of_trials,MAX_TRIALS));
  fprintf(log_performance,"\n");
  fprintf(log_performance, "Max: %f\t ", worst_of_vector(perf_of_trials,MAX_TRIALS));
  fprintf(log_performance,"\n");


  fclose(log_performance);
 

}


void free_stats(){

  //free memory
  delete[] perf_of_trials;

}

const std::string StatsInterface::statsPath = "../stats";

PopulationMetrics StatsInterface::calculate_population_metrics(const std::vector<double> &data) {
    PopulationMetrics metrics;

    metrics.size = data.size();

    if (data.empty()) {
        metrics.min = 0.0;
        metrics.max = 0.0;
        metrics.avg = 0.0;
        metrics.std = 0.0;
    } else {
        // Calculate min, max, mean, and standard deviation for feasible data
        metrics.min = *std::min_element(data.begin(), data.end());
        metrics.max = *std::max_element(data.begin(), data.end());

        double sum = 0.0;
        for (double value : data) {
            sum += value;
        }
        metrics.avg = sum / static_cast<double>(data.size());

        double sumSquaredDiff = 0.0;
        for (double value : data) {
            double diff = value - metrics.avg;
            sumSquaredDiff += diff * diff;
        }
        metrics.std = (data.size() == 1) ? 0.0 : std::sqrt(sumSquaredDiff / static_cast<double>(data.size() - 1));
    }

    return metrics;
}

bool StatsInterface::create_directories_if_not_exists(const string &directoryPath) {
    if (!fs::exists(directoryPath)) {
        try {
            fs::create_directories(directoryPath);
//            std::cout << "Directory created successfully: " << directoryPath << std::endl;
            return true;
        } catch (const std::exception& e) {
//            std::cerr << "Error creating directory: " << e.what() << std::endl;
            return false;
        }
    } else {
//        std::cout << "Directory already exists: " << directoryPath << std::endl;
        return true;
    }
}

void StatsInterface::stats_for_multiple_trials(const std::string& filePath, const std::vector<double>& data) {
    std::ofstream logStats;

    logStats.open(filePath);

    std::ostringstream oss;
    for (auto& perf:data) {
        oss << fixed << setprecision(2) << perf << endl;
    }
    PopulationMetrics metric = calculate_population_metrics(data);
    oss << "Mean " << metric.avg << "\t \tStd Dev " << metric.std << "\t " << endl;
    oss << "Min: " << metric.min << "\t " << endl;
    oss << "Max: " << metric.max << "\t " << endl;
    logStats << oss.str() << flush;

    logStats.close();
}
