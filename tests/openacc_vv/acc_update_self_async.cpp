#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel
#pragma acc data create(c[0:n], d[0:n]) copyin(a[0:n], b[0:n])
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc parallel async(2)
#pragma acc loop
#pragma acc wait
#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel
#pragma acc data copyout(c[0:n]) copyin(a[0:n], b[0:n])
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc parallel async(2)
#pragma acc loop
#pragma acc parallel async(1)
#pragma acc loop
#pragma acc parallel async(2)
#pragma acc loop
#pragma acc parallel async(1) wait(2)
#pragma acc loop
#pragma acc wait(1)
