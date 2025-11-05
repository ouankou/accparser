#pragma acc data copyin(b[0:n]) copy(a[0:n])
#pragma acc parallel
#pragma acc loop
#pragma acc loop
#pragma acc atomic update
