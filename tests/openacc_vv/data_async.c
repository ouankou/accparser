#pragma acc data copy(a[0:n], b[0:n], c[0:n]) async(1)
#pragma acc parallel
#pragma acc loop
#pragma acc data copy(a[0:n], b[0:n], c[0:n]) async(1)
#pragma acc parallel
#pragma acc loop
#pragma acc wait(1)
#pragma acc data copy(a[0:n], b[0:n], c[0:n]) async(0)
#pragma acc parallel loop async(1) wait(0)
#pragma acc wait(1) async(0)
#pragma acc wait(0)
