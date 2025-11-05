#pragma acc data copyin(a[0:10*n]) copy(b[0:10])
#pragma acc parallel loop private(temp)
#pragma acc loop vector reduction(||:temp)
