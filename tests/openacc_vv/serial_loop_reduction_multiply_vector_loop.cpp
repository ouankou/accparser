#pragma acc data copyin(a[0:10*multiplicitive_n], b[0:10*multiplicitive_n]) copyout(c[0:10])
#pragma acc serial loop private(temp)
#pragma acc loop vector reduction(*:temp)
