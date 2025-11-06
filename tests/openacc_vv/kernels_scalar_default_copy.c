#pragma acc data copyin(b[0:n])
#pragma acc kernels num_gangs(1) vector_length(1)
#pragma acc loop
