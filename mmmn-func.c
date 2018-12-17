/* 
	mmmn-func.c
	
	C function form of FESC code: mmmn-fesc.c
	
	Prototype code for eventual inclusion in PDQ lib
	which will be called: PDQ_CreateMultiClosed
	Current PDQ_CreateMultiNode(int servers, char *name, int device, int sched)
	will be renamed PDQ_CreateMultiOpen()
	
	Created by NJG on Sunday, December 17, 2017
	Revised by NJG on Sunday, December 16, 2018
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include "../pdq-qnm-pkg/lib/PDQ_Lib.h"  Not needed here


#define MAX_USERS 600  // 500 for AWS-Tomcat model


// Globals - these will be set by calling a PDQ_CreateClosed workload
int             glob_N;      // number of customers
int             glob_m;      // number of servers
float           glob_D;      // service demand 
float           glob_Z;      // think time

double          glob_R;      // residence time 
double          glob_Q;      // no. customers 
double          glob_X;      // mean thruput
double          glob_U;      // total utilization

double          sm_x[MAX_USERS + 1]; // submodel thruput

char            glob_wrkname[15];
char            glob_devname[15];


int main(void) {

	// IDs from PDQ lib types
	int CEN  = 4;
	int FCFS = 8;
	int TERM = 11;
	
	void CreateClosed(char *name, int wtype, float users, float think);
	void CreateMultiClosed(int servers, char *name, int device, int scheds);
	void SetDemand(char *nodename, char *workname, float servicetime);
	void SolveFESC();
	void PrintResults();

	CreateClosed("Requests", TERM, 500, 1e-9);
	CreateMultiClosed(350, "funcFESC", CEN, FCFS);
	SetDemand("funcFESC", "Requests", 0.4442); // do this first in this mock code
	SolveFESC();
	PrintResults();
	
} // main



void CreateClosed(char *name, int wtype, float users, float think) {
// Dummy function to emulate creating a CLOSED workload

    // wtype not used here
	glob_N = users;
	glob_Z = think;
	strcpy(glob_wrkname, name);
	
} // end CreateClosed



void CreateMultiClosed(int servers, char *name, int device, int sched) {
    
    // device and sched not used here
    glob_m = servers; // local param
    strcpy(glob_devname, name);

}



void SetDemand(char *nodename, char *workname, float servicetime) {
// Dummy function to emulate setting service time of FESC

	glob_D = servicetime;
	
} // end CreateClosed



// This function well goes into PDQ_Build.c
// Solution code can go into PDQ_MServer.c (where the erlang code is also)
// Extension of CreateClosed func args: char *name, int TERM, float users, float think

void SolveFESC() {
// This code has the same logical footing as ErlangR() in PDQ_MServer.c

    // local version of PDQ globals
    int             m;
    int             N;
    double          Z;
    double          D;
    double          R;
	double          Q;
	double          X;
	double          U;

	// internal vars
	int             i;
	int             j;
	int             n;
	int             ii, jj;
	int             nn;
	float           qlength;

	void            SubModel(int pop, int servers, float demand);


    // In this M/M/m/N/N fesc model the submodel is just a delay center
    // Hence, 'pq' is an array of 1s and 0s due to no waiting line in submodel
    // dimension is MAX_USERS + 1, e.g., 500 + 1 = 0, 1..500
    float           pq[MAX_USERS + 1][MAX_USERS + 1]; 
    
    // Get globals as inputs
    D = glob_D;
    N = glob_N;
    Z = glob_Z;
    m = glob_m;

	if (N > MAX_USERS) {
		printf("N=%d cannot be greater than %d\n", N, MAX_USERS);
		exit(-1);		
	}

	if (Z == 0) {
		printf("Z=%.2f can have any tiny value but not ZERO\n", Z);
		exit(-1);		
	}


	// init matrix to all zeros 
	for (i = 0; i <= N; i++) {
		for (j = 0; j <= N; j++) {
			pq[i][j] = 0;
		}
	}

	pq[0][0] = 1.0;

	// Solve the submodel
	SubModel(N, m, D);


	// Solve the composite model ...
    for (n = 1; n <= N; n++) {
    
        R = 0.0;                // reset

        // response time at the FESC
        for (j = 1; j <= n; j++) {
            R += (j / sm_x[j]) * pq[j - 1][n - 1];
        }

        // mean thruput and mean qlength at the FESC */
        X = n / (Z + R);
        Q = X * R;
        
        // compute qlength dsn at the FESC */
        for (j = 1; j <= n; j++) {
            pq[j][n] = (X / sm_x[j]) * pq[j - 1][n - 1];
            
            // error
            if (pq[j][n] < 0 || pq[j][n] > 1) {
            	//printf("ERROR: Prob[j=%d][n=%d]: %.4f\n\n", j, n, pq[j][n]);
            	//exit(-1);
            }
        }

        // and the j = 0 case ... 
        pq[0][n] = 1.0;
        for (j = 1; j <= n; j++) {
            pq[0][n] -= pq[j][n];
        }
    }

	U = X * D;

	// Update globals
	glob_R = R;
	glob_Q = Q;
	glob_X = X;
	glob_U = U;

} // end  of MmmnFunc



void SubModel(int pop, int servers, float demand) {
// Delay center with no waiting line
// Only valid for M/M/m/N/N

    int             i;

    for (i = 0; i <= pop; i++) {
        if (i <= servers) {
            sm_x[i] = (double) i / demand;
        } else {
            sm_x[i] = (double) servers / demand;
        }
    }
}



void PrintResults() {
	printf("\n");
	printf("  PDQ FESC function \'%s\'\n",  glob_devname);
	printf("  ---------------------------------\n");
	printf("  No. of servers:   %14d\n", glob_N);
 	printf("  No. of requests:  %14d\n", glob_m);
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
} // end of PrintResults

