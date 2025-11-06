#pragma acc data copyin(a[0:n], b[0:n]) copy(totals[0:10], c[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc atomic capture
