#include<cmath>
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<cstring>
#include <iostream>

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


const char* STATS_PATH = "../stats/";

// customized variables
FILE *log_evolution_details;
char *evolution_filename;

// customized functions
void open_stats_for_evolution(void) {
    //initialize and open output files
    evolution_filename = new char [CHAR_LEN];
    snprintf(evolution_filename, CHAR_LEN, "%sevols.%s.csv", STATS_PATH, problem_instance);

    //for evolutionary details for each generation
    if ((log_evolution_details = fopen(evolution_filename, "a")) == nullptr) {
        cout << "Failed to open stats file " << evolution_filename << endl;
        exit(2);
    }
    else {
        fprintf(log_evolution_details, "iterations,vns_cnt,current_best,global_best,evaluations,progress,hours,minutes,seconds\n");
    }
    //initialize and open output files
}

void flush_row_into_file(const char* row) {
    fprintf(log_evolution_details, "%s\n", row);
}



void close_stats_for_evolution(void) {

    fclose(log_evolution_details);

}

void free_stats(){

  //free memory
  delete[] perf_of_trials;

}

