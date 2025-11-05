#pragma acc data copyin(a[0:n])
#pragma acc parallel copyout(zero: b[0:n])
#pragma acc loop
#pragma acc enter data copyin(hasDevice[0:1])
#pragma acc parallel present(hasDevice[0:1])
#pragma acc data copyin(a[0:n])
#pragma acc parallel copyout(zero: b[0:n])
#pragma acc loop
#pragma acc data copyin(a[0:n]) copy(b[0:n])
#pragma acc parallel copyout(zero: b[0:n])
#pragma acc loop
