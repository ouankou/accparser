#pragma acc data create(b[0:n])
#pragma acc data copyin(a[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc data copyout(c[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc data present_or_create(b[0:n])
#pragma acc data copyin(a[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc data copyout(c[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc data pcreate(b[0:n])
#pragma acc data copyin(a[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc data copyout(c[0:n])
#pragma acc parallel
#pragma acc loop
