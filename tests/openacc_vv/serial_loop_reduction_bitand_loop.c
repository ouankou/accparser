#pragma acc data copyin(a[0:10 * n]) copy(b[0:10 * n], c[0:10])
#pragma acc serial
#pragma acc loop gang private(temp)
#pragma acc loop worker reduction(&:temp)
#pragma acc loop worker
