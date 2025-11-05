#pragma acc data copy(a[0:n], b[0:n])
#pragma acc parallel
#pragma acc loop independent
#pragma acc atomic capture
