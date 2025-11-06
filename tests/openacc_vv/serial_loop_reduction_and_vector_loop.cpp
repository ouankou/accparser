#pragma acc data copyin(a[0:10*n]) copy(b[0:10])
#pragma acc serial loop private(temp)
#pragma acc loop vector reduction(&&:temp)
