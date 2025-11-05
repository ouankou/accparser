#pragma acc data copy(a[0:n])
#pragma acc serial loop
#pragma acc data copy(a[0:n], b[0:n], c[0:n])
#pragma acc serial
#pragma acc loop
#pragma acc loop
