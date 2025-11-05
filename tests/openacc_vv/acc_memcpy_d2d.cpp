#pragma acc enter data create(a[0:n])
#pragma acc enter data create(b[0:n])
#pragma acc update device(a[0:n])
#pragma acc update device(b[0:n])
#pragma acc data present(a)
#pragma acc parallel loop
#pragma acc parallel loop
#pragma acc update host(b[0:n])
#pragma acc exit data delete(a[0:n], b[0:n])
