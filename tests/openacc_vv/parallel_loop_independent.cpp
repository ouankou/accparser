#pragma acc data copy(a[0:n]) copyout(b[0:n])
#pragma acc parallel loop independent
