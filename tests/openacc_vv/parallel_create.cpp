#pragma acc data copyin(a[0:n]) copy(c[0:n])
#pragma acc parallel create(b[0:n])
#pragma acc loop
#pragma acc loop
