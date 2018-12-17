#! /bin/sh 

# This test is ONLY for the repairman model M/M/m/N/N
# Since the submodel is simply a delay center, w/o a 
# waiting line, all pq[][] elements are either 1 or 0.
servers=350
users=500
demand=0.45
think1=1e-9
think2=1e-9

./mmmn-prob $servers $demand $users $think1
./mmmn-fesc $servers $demand $users $think2

exit 0

