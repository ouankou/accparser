#pragma acc data copyin(a[0:n], b[0:n])
#pragma acc parallel loop present(a[0:n], b[0:n]) async(1)
#pragma acc update host(b[0:n]) wait(1)
