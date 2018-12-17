/* 
	mmmn-func.c
	
	C function form of FESC code: mmmn-fesc.c
	
	Prototype code for eventual inclusion in PDQ lib
	which will be called: PDQ_CreateMultiClosed
	Current PDQ_CreateMultiNode(int servers, char *name, int device, int sched)
	will be renamed PDQ_CreateMultiOpen()
	
	Created by NJG on Sunday, December 17, 2017
	Updated by NJG on Sunday, December 16, 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include "../pdq-qnm-pkg/lib/PDQ_Lib.h"  Not needed here

#define MAX_USERS 500  //needed for tomcat model


// Globals
int             N;      // number of customers
int             m;      // number of servers
float           D;      // service demand 
float           Z;      // think time

double          sm_x[MAX_USERS + 1]; // submodel thruput
double          R;      // residence time 
double          Q;      // no. customers 
double          X;      // mean thruput
double          U;      // total utilization


int main(void) {
	void MmmnFunc(char *name, float users, float think, int servers);

	// These params will come from setting up PDQ functions
	float users = 500;
	float think = 1e-9;  // Z = zero 
	int servers = 350;
	
	MmmnFunc("newFunc", users, think, servers);

}


// This function well goes into PDQ_Build.c
// Solution code can go into PDQ_MServer.c (where the erlang code is also)
// Extension of CreateClosed func args: char *name, int TERM, float users, float think

void MmmnFunc(char *name, float users, float think, int servers) {

	int             i;
	int             j;
    int             n;
    int             ii, jj;
    int             nn;
    float           qlength;
	
    void            sub_model(int pop, int servers, float demand);
    void            print_results();
    
    // In this M/M/m/N/N fesc model the submodel is just a delay center
    // Hence, 'pq' is an array of 1s and 0s due to no waiting line in submodel
    // dimension is MAX_USERS + 1, e.g., 500 + 1 = 0, 1..500
    float             pq[MAX_USERS + 1][MAX_USERS + 1]; 

    
    m = servers;
    D = 0.45;
    N = users;
    Z = think;

	if (N > 500) {
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
    sub_model(N, m, D);



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

    print_results();
    
    
} // MmmnFunc


void sub_model(int pop, int servers, float demand) {
// delay center with no waiting line
    int             i;

    for (i = 0; i <= pop; i++) {
        if (i <= servers) {
            sm_x[i] = (double) i / demand;
        } else {
            sm_x[i] = (double) servers / demand;
        }
    }
}



void print_results() {
	printf("\n");
	printf("  M/M/%d/%d/%d repairmen FESC model\n", m, N, N);
	printf("  ---------------------------------\n");
	printf("  Machine pop:      %14d\n", N);
 	printf("  No. repairmen:    %14d\n", m);
	printf("  MT to failure:    %14g\n", Z);
	printf("  Service time:     %14.4f\n", D);
	printf("  Breakage rate:    %14.4g\n", 1 / Z);
	printf("  Service rate:     %14.4f\n", 1 / D);
	printf("  Utilization:      %14.4f\n", U);
	printf("  Per Server:       %14.4f\n", U / m);
	printf("  \n");
	printf("  No. in system:    %14.4f\n", Q);
	printf("  No in service:    %14.4f\n", U);
	printf("  No.  enqueued:    %14.4f\n", Q - U);
	printf("  Waiting time:     %14.4f\n", R - D);
	printf("  Throughput:       %14.4f\n", X);
	printf("  Response time:    %14.4f\n", R);
	printf("  Stretch factor:   %14.4f\n", R / D);
}

