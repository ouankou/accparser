#pragma acc data copyin(a[0:n]) copyout(b[0:n])
#pragma acc serial loop auto
#pragma acc data copy(a[0:n])
#pragma acc serial loop auto
