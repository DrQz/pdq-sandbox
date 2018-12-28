/*******************************************************************************
 *  Copyright (C) 1994 - 2019, Performance Dynamics Company                    
 *
 * NJG on Monday, December 18, 2017
 * This is prototype version, eventually to be integrated into PDQ_MServer.c
 * 
 * PDQ_MServer2.c
 * 
 * Created by NJG on Mon, Apr 2, 2007
 *
 * A collection of subroutines to solve multiserver queues.
 * (only one function so far)
 * 		- ErlangR returns the residence for an M/M/m queue.
 *      - 
 *
 *******************************************************************************/

 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include "PDQ_Lib.h"
 

double ErlangR(double arrivrate, double servtime, int servers) {
// Called from MVA_Canon.c 

	extern int   PDQ_DEBUG;
	extern char  s1[];
	extern       JOB_TYPE  *job;

	double		erlangs;
	double		erlangB;
	double		erlangC;
	double		rho;
	double		eb;
	double		wtE;
	double		rtE;
	int		    mm;
	char        *p = "ErlangR()";

	
	erlangs  = arrivrate * servtime;
	
	if (erlangs >= servers) {
		sprintf(s1, "%4.2lf Erlangs saturates %d servers", erlangs, servers);
	    errmsg(p, s1);
	}
	
	rho     = erlangs / servers;
	erlangB = erlangs / (1.0 + erlangs);
	for (mm = 2; mm <= servers; mm++) {
		eb  = erlangB;
		erlangB = eb * erlangs / (mm + (eb * erlangs));
	}
	
	erlangC  = erlangB / (1.0 - rho + (rho * erlangB));
	
	wtE = servtime * erlangC / (servers * (1.0 - rho));
	rtE = servtime + wtE;
	
	if (PDQ_DEBUG) {
		sprintf(s1, "Erlang R: %6.4lf", rtE);
		errmsg(p, s1);
	}
	
	return(rtE);

} // end of ErlangR



// FESC globals added by NJG on Monday, December 18, 2017
// Updated by NJG on Thursday, December 27, 2018

#define MAX_USERS 1200 // needs to be big to model threads 

/* 
int             N;      // number of customers
int             m;      // number of servers
float           D;      // service demand 
float           Z;      // think time
*/
double          R;      // residence time 
double          Q;      // no. customers 
double          X;      // mean thruput
double          U;      // total utilization
double          sm_x[MAX_USERS + 1];




void MMmNN(int m, int N, double D, double Z) {
	extern int        streams, nodes;
	extern            NODE_TYPE *node;
	extern            JOB_TYPE  *job;
	extern char       s1[], s2[], s3[], s4[];
	extern double     getjob_pop();

	int             i, j;
	int             c, k;
    int             n;
    float           pq[MAX_USERS + 1][MAX_USERS + 1]; 
    double          sumR[1] = {0.0}; //single class vector
    
	void            MMmNN_sub(int pop, int servers, float demand);

	if (N > MAX_USERS) {
        printf("N=%d cannot be greater than %d\n", N, MAX_USERS);
        exit(-1);		
	}
	
	for (i = 0; i <= N; i++) {
		for (j = 0; j <= N; j++) {
			pq[i][j] = 0;
		}
	}
	
	pq[0][0] = 1.0;
	

	// Solve the submodel
    MMmNN_sub(N, m, D);

	// Solve the composite model
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
        }

        // include j = 0 row
        pq[0][n] = 1.0;
        for (j = 1; j <= n; j++) {
            pq[0][n] -= pq[j][n];
        }
    }

    U = X * D;
    
    
    // collect queueing results
	for (k = 0; k < 1; k++) {
		for (c = 0; c < 1; c++) {
		
			switch (job[c].should_be_class) {
				case TERM:
					job[c].term->sys->thruput = X;
					job[c].term->sys->response = R;
					break;
				case BATCH:
					job[c].batch->sys->thruput = X;
					job[c].batch->sys->response = R;
					break;
				default:
					break;
			}
			
			node[k].qsize[c] = Q;
			node[k].resit[c] = R;
			sumR[c] += node[k].resit[c];
			node[k].utiliz[c] = U;
		}
	}



} // end of MMmNN 


void MMmNN_sub(int pop, int servers, float demand) {
// delay center with no waiting line
    int             i;
    
    for (i = 0; i <= pop; i++) {
        if (i <= servers) {
            sm_x[i] = (double) i / demand;
        } else {
            sm_x[i] = (double) servers / demand;
        }
    }
    
} // end of MMmNN_sub 





