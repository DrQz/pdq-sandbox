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

int main(void) {
	void MmmnFunc(char *name, float users, float think, int servers);

	// These params will come from setting up PDQ functions
	float users = 500;
	float think = 0;  // Z = zero 
	int servers = 350;
	
	MmmnFunc("newFunc", users, think, servers);

}


// This function well goes into PDQ_Build.c
// Solution code can go into PDQ_MServer.c (where the erlang code is also)
// Extension of CreateClosed func args: char *name, int TERM, float users, float think

void MmmnFunc(char *name, float users, float think, int servers) {

} // MmmnFunc


