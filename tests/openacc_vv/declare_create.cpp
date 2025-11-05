#pragma acc declare create(mult_create)
#pragma acc routine vector
#pragma acc data present(a[0:n])
#pragma acc loop vector
#pragma acc update host(a[0:n])
#pragma acc declare create(scalar, a[0:n], n)
#pragma acc update device(n)
#pragma acc parallel loop present(a[0:n])
#pragma acc routine vector
#pragma acc loop vector
#pragma acc update host(a[0:n])
#pragma acc enter data create(a[0:n])
#pragma acc update device(a[0:n])
#pragma acc data copy(c[0:n]) present(a[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc update device(scalar)
#pragma acc data copy(local_a[0:n], c[0:n]) present(scalar)
#pragma acc parallel
#pragma acc loop
#pragma acc enter data create(a[0:n])
#pragma acc update device(a[0:n])
#pragma acc update host(a[0:n])
#pragma acc enter data create(a[0:n])
#pragma acc update device(a[0:n])
#pragma acc enter data create(a[0:n])
#pragma acc update device(a[0:n], mult_create)
#pragma acc data copy(c[0:n]) present(a[0:n], mult_create)
#pragma acc parallel
#pragma acc loop
#pragma acc enter data create(a[0:n])
#pragma acc update device(a[0:n])
