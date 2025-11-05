#pragma acc declare create(c[0:n])
#pragma acc parallel present(a[0:n], b[0:n], d[0:n])
#pragma acc loop
#pragma acc loop
#pragma acc declare create(c[0:n])
#pragma acc parallel present(a[0:n], b[0:n], d[0:n])
#pragma acc loop
#pragma acc loop
#pragma acc data copyin(a[0:n][0:n], b[0:n][0:n]) copyout(d[0:n][0:n])
#pragma acc data copyin(a[0:n][0:n], b[0:n][0:n]) copy(c[0:n][0:n]) copyout(d[0:n][0:n])
#pragma acc enter data copyin(devtest[0:1])
#pragma acc parallel present(devtest[0:1])
#pragma acc data copyin(a[0:n][0:n], b[0:n][0:n])
#pragma acc data copyin(c[x:1][0:n]) copyout(d[x:1][0:n])
