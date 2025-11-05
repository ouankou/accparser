#pragma acc enter data copyin(hasDevice[0:1])
#pragma acc serial present(hasDevice[0:1])
#pragma acc data copyin(a[0:n])
#pragma acc serial copyout(b[0:n])
#pragma acc loop
#pragma acc enter data copyin(hasDevice[0:1])
#pragma acc serial present(hasDevice[0:1])
#pragma acc data copyin(a[0:n], b[0:n])
#pragma acc serial copyout(b[0:n])
#pragma acc loop
#pragma acc enter data copyin(hasDevice[0:1])
#pragma acc serial present(hasDevice[0:1])
#pragma acc data copyin(a[0:n], b[0:n])
#pragma acc serial copyout(b[0:n])
#pragma acc loop
#pragma acc update host(b[0:n])
