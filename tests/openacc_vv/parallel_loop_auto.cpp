#pragma acc data copy(device[0:n])
#pragma acc parallel loop num_gangs(1) vector_length(1) num_workers(1) auto
