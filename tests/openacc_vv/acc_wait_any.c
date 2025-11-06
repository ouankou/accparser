#pragma acc data copy(a[0:n], b[0:n], c[0:n])
#pragma acc parallel loop async(i)
#pragma acc parallel loop async(2)
