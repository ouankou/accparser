#pragma acc data copy(a[0:10*n]) copyout(results[0:10])
#pragma acc parallel loop gang private(temp)
#pragma acc loop worker reduction(||:temp)
#pragma acc loop worker
