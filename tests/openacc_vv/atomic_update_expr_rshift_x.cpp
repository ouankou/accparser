#pragma acc data copyin(a[0:3 * n]) copy(b[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc loop independent
#pragma acc atomic update
