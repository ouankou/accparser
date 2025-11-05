#pragma acc enter data create(a[0:10][0:n], b[0:10][0:n], c[0:10][0:n], d[0:10][0:n], e[0:10][0:n])
#pragma acc update device(a[x:1][0:n], b[x:1][0:n], d[x:1][0:n]) async(x)
#pragma acc parallel present(a[x:1][0:n], b[x:1][0:n], c[x:1][0:n]) async(x)
#pragma acc loop
#pragma acc parallel present(c[x:1][0:n], d[x:1][0:n], e[x:1][0:n]) async(x)
#pragma acc loop
#pragma acc update host(e[x:1][0:n]) async(x)
#pragma acc exit data delete(a[0:10][0:n], b[0:10][0:n], c[0:10][0:n], d[0:10][0:n], e[0:10][0:n])
#pragma acc data copyin(a[0:10*n], b[0:10*n], d[0:10*n]) copyout(c[0:10*n], e[0:10*n])
#pragma acc parallel present(a[0:10*n], b[0:10*n], c[0:10*n]) async(x)
#pragma acc loop
#pragma acc parallel present(c[0:10*n], d[0:10*n], e[0:10*n]) async(x)
#pragma acc loop
#pragma acc data copyin(a[0:10][0:n], b[0:10][0:n], d[0:10][0:n]) copyout(c[0:10][0:n], e[0:10][0:n])
#pragma acc parallel present(a[0:10][0:n], b[0:10][0:n], c[0:10][0:n]) async
#pragma acc loop
#pragma acc parallel present(c[0:10][0:n], d[0:10][0:n], e[0:10][0:n]) async
#pragma acc loop
