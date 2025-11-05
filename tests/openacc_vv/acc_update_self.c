#pragma acc data create(c[0:n]) copyin(a[0:n], b[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc data copyout(c[0:n]) copyin(a[0:n], b[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc parallel
#pragma acc loop
#pragma acc parallel
#pragma acc loop
