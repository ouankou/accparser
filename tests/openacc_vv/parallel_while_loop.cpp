#pragma acc data copy(a[0:10*n])
#pragma acc parallel
#pragma acc loop
#pragma acc loop reduction(+:avg)
