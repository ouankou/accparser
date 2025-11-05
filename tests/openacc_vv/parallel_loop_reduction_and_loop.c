#pragma acc data copy(a[0:10*n])
#pragma acc parallel loop gang private(temp)
#pragma acc loop worker reduction(&&:temp)
#pragma acc loop worker
#pragma acc data copy(a[0:25*n])
#pragma acc parallel loop gang private(device)
#pragma acc loop worker reduction(&&:device)
#pragma acc loop worker
