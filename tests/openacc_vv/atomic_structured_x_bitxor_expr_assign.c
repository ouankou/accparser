#pragma acc data copyin(a[0:n]) copy(totals[0:n/10 + 1]) copyout(b[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc atomic capture
