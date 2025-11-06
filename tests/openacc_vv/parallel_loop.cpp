#pragma acc data copy(a[0:n])
#pragma acc parallel loop
#pragma acc data copy(a[0:n], b[0:n], c[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc loop
