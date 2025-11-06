#pragma acc data copyin(a[0:10*n], b[0:10*n]) copy(d[0:10*n])
#pragma acc parallel firstprivate(c[0:n])
#pragma acc loop gang
#pragma acc loop worker
#pragma acc data copyin(a[0:10*n], b[0:10*n]) copy(d[0:10*n])
#pragma acc parallel firstprivate(c[0:n])
#pragma acc loop gang independent
#pragma acc loop worker independent
#pragma acc loop worker independent
