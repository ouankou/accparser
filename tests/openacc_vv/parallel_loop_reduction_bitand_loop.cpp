#pragma acc data copyin(a[0:10 * n]) copy(b[0:10 * n], c[0:10])
#pragma acc parallel loop gang private(temp)
#pragma acc loop worker reduction(&:temp)
#pragma acc loop worker
#pragma acc data copyin(a[0:25*n]) copy(b[0:25*n], c[0:25])
#pragma acc parallel loop gang private(device)
#pragma acc loop worker reduction(&:device)
#pragma acc loop worker
