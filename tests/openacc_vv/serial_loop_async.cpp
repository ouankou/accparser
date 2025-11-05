#pragma acc data copyin(a[0:10*n], b[0:10*n], c[0:10*n], d[0:10*n]) copy(errors[0:10])
#pragma acc serial loop async(x)
#pragma acc serial loop async(x)
#pragma acc wait
