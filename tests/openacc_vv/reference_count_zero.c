#pragma acc data copy(a[0:n], b[0:n], c[0:n])
#pragma acc parallel loop
#pragma acc exit data copyout(c[0:n])
