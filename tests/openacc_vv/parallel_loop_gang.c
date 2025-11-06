#pragma acc data copyin(a[0:n], b[0:n]) copy(c[0:n])
#pragma acc parallel loop gang
