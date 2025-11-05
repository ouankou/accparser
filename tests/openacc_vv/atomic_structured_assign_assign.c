#pragma acc data copyin(a[0:n]) copy(b[0:n]) copyout(c[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc atomic capture
