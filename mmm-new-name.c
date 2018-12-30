// mmm-new-name.c
// Created by NJG on Saturday, December 29, 2018
//
// Test calling new PDQ_CreateOpenMultiserver() function
//
// Check resource queue length == number in PDQ_Report()
 
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "lib/PDQ_Lib.h" 

int main() {
	
    double aRate = 0.75;
    double sTime = 1.0;

    PDQ_Init("Bank Model");
    PDQ_CreateOpen("Customers", aRate);
    
	PDQ_CreateOpenMultiserver(4, "Tellers", MSO, FCFS);
    PDQ_SetDemand("Tellers", "Customers", sTime);
    
    PDQ_SetWUnit("Customers");
    PDQ_SetTUnit("Minute");
    
    PDQ_Solve(CANON);
    PDQ_Report();

    printf("Check queue length calculation in Report:\n"); 
    
    printf("Mean queue length: %8.4lf\n\n", 
      PDQ_GetQueueLength("Tellers", "Customers", TRANS)
    );
}

