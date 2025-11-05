#pragma acc declare copyin(a[0:n], b[0:n])
#pragma acc parallel present(c[0:n])
#pragma acc loop
#pragma acc declare copyin(a[0:n], b[0:n])
#pragma acc parallel present(c[0:n])
#pragma acc loop
#pragma acc data copy(c[0:n][0:n])
#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel present(devtest[0:1])
#pragma acc data copy(a[0:n][0:n], b[0:n][0:n], c[0:n][0:n])
#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel present(devtest[0:1])
#pragma acc data copy(c[0:n][0:n])
