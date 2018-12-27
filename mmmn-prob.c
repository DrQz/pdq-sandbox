/*
 * mmmn-prob.c
 * 
 * Exact solution for M/M/m/N/N repairmen model.
 * 
 * Created by NJG: 17:45:47  12-19-94 
 * Updated by NJG: 16:45:35  02-26-96
 * $Id: repair.c,v 4.4 2009/04/03 02:19:40 pfeller Exp $
 * Updated by NJG: Monday, December 11, 2017 for testing PDQ fesc code
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>		/* needed for double types */


int main(int argc, char *argv[]) {
   long double     L;		/* mean number of broken machines in line */
   long double     Q;		/* mean number of broken machines in system */
   long double     R;		/* mean response time */
   long double     S;		/* mean service time */
   long double     U;		/* total mean utilization */
   long double     rho;		/* per server utilization */
   long double     W;		/* mean time waiting for repair */
   long double     X;		/* mean throughput */
   long double     Z;		/* mean time to failure (MTTF) */
   long double     p;		/* temp variable for prob calc. */
   long double     p0;		/* prob if zero breakdowns */

   int            m;		/* Number of servicemen */
   int            N;		/* Number of machines */
   int            k;		/* loop index */

#ifdef MAC
   argc = ccommand(&argv);
#endif

   if (argc != 5) { // counts pgm name
      printf("Usage: %s m S N Z\n", *argv);
      exit(1);
   }
   m = atoi(*++argv);
   S = atof(*++argv);
   N = atoi(*++argv);
   Z = atof(*++argv);

   p = p0 = 1;
   L = 0;

   for (k = 1; k <= N; k++) {
      p *= (N - k + 1) * S / Z;

      if (k <= m) {
	 p /= k;
      } else {
	 p /= m;
      }
      p0 += p;

      if (k > m) {
	 L += p * (k - m);
      }
   }				/* loop */

   p0 = 1.0 / p0;
   L *= p0;
   W = L * (S + Z) / (N - L);
   R = W + S;
   X = N / (R + Z);
   U = X * S;
   rho = U / m;
   Q = X * R;

   printf("\n");
   printf("  M/M/%d/%d/%d probability model\n", m, N, N);
   printf("  ----------------------------\n");
   printf("  Machine pop:      %14d\n", N);
   printf("  No. repairmen:    %14d\n", m);
   printf("  MT to failure:    %14.4Lg\n", Z);
   printf("  Service time:     %14.4Lf\n", S);
   printf("  Breakage rate:    %14.4Lg\n", 1 / Z);
   printf("  Service rate:     %14.4Lf\n", 1 / S);
   printf("  Utilization:      %14.4Lf\n", U);
   printf("  Per Server:       %14.4Lf\n", rho);
   printf("  \n");
   printf("  No. in system:    %14.4Lf\n", Q);
   printf("  No in service:    %14.4Lf\n", U);
   printf("  No.  enqueued:    %14.4Lf\n", Q - U);
   printf("  Waiting time:     %14.4Lf\n", R - S);
   printf("  Throughput:       %14.4Lf\n", X);
   printf("  Response time:    %14.4Lf\n", R);
   printf("  Stretch factor:   %14.4Lf\n", R / S);
   printf("  \n");

   return(0);
   
}  // end of main

