#pragma acc data copyin(a[0:10*n], b[0:10*n], c[0:10*n], d[0:10*n]) copy(errors[0:10])
#pragma acc parallel loop async(x)
#pragma acc parallel loop async(x) reduction(+:errors[x])
#pragma acc wait
#pragma acc parallel loop copy(a[0:n]) async(0)
#pragma acc parallel loop copy(c[0:n]) async(0)
#pragma acc wait
