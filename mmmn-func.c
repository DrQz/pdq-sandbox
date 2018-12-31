/*
 * mmmn-func.c
 * 
 * C function MVA form of FESC code: mmmn-fesc.c
 * 
 * Prototype code for eventual inclusion in PDQ lib which will be called:
 * PDQ_CreateMultiClosed Current PDQ_CreateMultiNode(int servers, char *name,
 * int device, int sched) will be renamed PDQ_CreateMultiOpen()
 * 
 * Created by NJG on Sunday, December 17, 2017 
 * Revised by NJG on Sunday, December 16, 2018
 * 
 */

#include <string.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include "lib/PDQ_Lib.h"  

//*****************GLOBALS ******************
//Globals - these will be set by calling a PDQ_CreateClosed workload
int glob_N;    //number of customers
int glob_m;    //number of servers
float glob_D;  //service demand
float glob_Z;  //think time

double glob_R; //residence time
double glob_Q; //no.customers
double glob_X; //mean thruput
double glob_U; //total utilization

# define MAX_USERS 1200 // needs to be big to model threads
double sm_x[MAX_USERS + 1]; //submodel thruput must be double for accuracy

//Model attributes
char glob_wrkname[15];
char glob_devname[15];


//================  Run from shell script to pass in args ================

int main(int argc, char *argv[]) {

	const char  *pgm = NULL;  //gets argv[0]
	int          threads;
	float        stime;
	int          requests;
	float        think;

	pgm      = argv[0];
	stime    = atoi(*++argv);
	stime    = atof(*++argv);
	requests = atoi(*++argv);
	think    = atof(*++argv);


	//declare emulated PDQ functions
	void test_CreateClosed(char * name, int wtype, int users, float think);
	void test_CreateClosedMultiserver(int servers, char * name, int device, int sched);
	void test_SetDemand(char * nodename, char * workname, float servicetime);
	void test_SolveFESC();
	void test_Report();

	PDQ_Init("AWS-Tomcat Cloud Model");  //does nothing yet
	PDQ_CreateClosed("Requests", TERM, requests, think); 
	
	//Need these coz PDQ_CreateClosed not coupled yet
	glob_N = requests;
	glob_m = threads;
	glob_Z = think;
	//test_CreateClosed("Requests", TERM, requests, think);

	PDQ_CreateMultiserverClosed(350, "cloudFESC", MSC, FCFS); 
	//test_CreateClosedMultiserver(350, "cloudFESC", CEN, FCFS);

    PDQ_SetDemand("cloudFESC", "Requests", stime); 
	//test_SetDemand("cloudFESC", "Requests", stime);
	
	PDQ_SetWUnit("Reqs");
	
	PDQ_Solve(EXACT);
	//test_SolveFESC();
	
	PDQ_Report();
	//test_Report();

} //end main

//============================================

void test_CreateClosed(char * name, int wtype, int users, float think) {
  //Dummy function to emulate creating a CLOSED workload
  //Extension of CreateClosed() args:char *name, int TERM, float users, float think

  // 'users'	must be an int since it defines matrix dimensions
  if (users > MAX_USERS) {
    printf("N=%d cannot be greater than %d\n", users, MAX_USERS);
    exit(-1);
  }
  //wtype must be TERM or BATCH, but not used here
  glob_N = users;
  glob_Z = think;
  strcpy(glob_wrkname, name);

} //end CreateClosed



void test_CreateClosedMultiserver(int servers, char * name, int device, int sched) {

  //device and sched not used here
  glob_m = servers; 
  strcpy(glob_devname, name);

}


void test_SetDemand(char * nodename, char * workname, float servicetime) {
  //Dummy function to emulate setting service time of FESC

  glob_D = servicetime;

} //end CreateClosed


void test_SolveFESC() {
  //This code is on the same logical footing as ErlangR() in PDQ_MServer.c

  // local version of PDQ globals
  int m;
  int N;
  double Z;
  double D;
  double R;
  double Q;
  double X;
  double U;

  //internal counters
  int i;
  int j;
  int n;
  int ii, jj;
  int nn;

  //Subroutine declaration
  void SubModel(int pop, int servers, float demand);

  // In this M/M/m/N/N FESC the submodel is just a delay center
  // Hence, 'pq' is an array of 1 s and 0 s due to no waiting line in submodel
  // dimension	is MAX_USERS + 1, e.g., 10 + 1 = 0, 1, 2,..10
  float pq[MAX_USERS + 1][MAX_USERS + 1];
  
  extern JOB_TYPE *job;

  //Get the global params
  D = glob_D;
  //N = glob_N;
  N = job[0].term->pop;
  //Z = glob_Z;
  Z = job[0].term->think;
  m = glob_m;

  //init matrix to all zeros
  for (i = 0; i <= N; i++) {
    for (j = 0; j <= N; j++) {
      pq[i][j] = 0;
    }
  }

  pq[0][0] = 1.0;

  //Solve the submodel
  SubModel(N, m, D);

  //Solve the composite model...
  for (n = 1; n <= N; n++) {

    R = 0.0; //reset

    // response time at the FESC
    for (j = 1; j <= n; j++) {
      R += (j / sm_x[j]) * pq[j - 1][n - 1];
    }

    //mean thruput and mean qlength at the FESC * /
    X = n / (Z + R);
    Q = X * R;

    //compute qlength dsn at the FESC * /
    for (j = 1; j <= n; j++) {
      pq[j][n] = (X / sm_x[j]) * pq[j - 1][n - 1];

      //error
      if (pq[j][n] < 0 || pq[j][n] > 1) {
        //printf("ERROR: Prob[j=%d][n=%d]: %.4f\n\n", j, n, pq[j][n]);
        //exit(-1);
      }
    }

    //and the j = 0 case...
    pq[0][n] = 1.0;
    for (j = 1; j <= n; j++) {
      pq[0][n] -= pq[j][n];
    }
  }

  U = X * D;

  //Update globals
  glob_R = R;
  glob_Q = Q;
  glob_X = X;
  glob_U = U;

} //end MmmnFunc


void SubModel(int pop, int servers, float demand) {
  //This is just one of many possible submodel types
  //Delay center with no waiting line
  // Only valid for M/M/m/N/N

  int i;

  for (i = 0; i <= pop; i++) {
    if (i <= servers) {
      sm_x[i] = (double) i / demand;
    } else {
      sm_x[i] = (double) servers / demand;
    }
  }
}


void test_Report() {
  printf("\n");
  printf("  PDQ FESC node \'%s\'\n", glob_devname);
  printf("  ---------------------------------\n");
  printf("  No. of requests:  %14d\n", glob_N);
  printf("  No. of servers:   %14d\n", glob_m);
  printf("  Think time:       %14g\n", glob_Z);
  printf("  Service time:     %14.4f\n", glob_D);
  printf("  Service rate:     %14.4f\n", 1 / glob_D);
  printf("  Utilization:      %14.4f\n", glob_U);
  printf("  Per Server:       %14.4f\n", glob_U / glob_m);
  printf("  \n");
  printf("  No. in system:    %14.4f\n", glob_Q);
  printf("  No in service:    %14.4f\n", glob_U);
  printf("  No.  enqueued:    %14.4f\n", glob_Q - glob_U);
  printf("  Waiting time:     %14.4f\n", glob_R - glob_D);
  printf("  Throughput:       %14.4f\n", glob_X);
  printf("  Response time:    %14.4f\n", glob_R);
  printf("  Stretch factor:   %14.4f\n", glob_R / glob_D);
  printf("\n");
} //end PrintResults
