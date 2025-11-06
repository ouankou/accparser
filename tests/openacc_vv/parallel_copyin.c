#pragma acc enter data copyin(hasDevice[0:1])
#pragma acc parallel present(hasDevice[0:1])
#pragma acc parallel copyin(a[0:n])
#pragma acc loop
#pragma acc data copy(b[0:n])
#pragma acc parallel copyin(a[0:n])
#pragma acc loop
