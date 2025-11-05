#pragma acc data copyin(a[0:n], b[0:n]) copyout(c[0:n])
#pragma acc set default_async(1)
#pragma acc parallel async
#pragma acc loop
#pragma acc set default_async(2)
#pragma acc parallel async
#pragma acc loop
#pragma acc parallel async(1) wait(2)
#pragma acc loop
#pragma acc wait(1)
