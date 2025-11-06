#pragma acc routine(test_routine_worker_loop_named) worker
#pragma acc routine(test_routine_worker_worker_named) worker
#pragma acc routine(test_routine_worker_vector_named) worker
#pragma acc routine(test_routine_worker_seq_named) worker
#pragma acc routine worker
#pragma acc loop worker reduction(+:returned)
#pragma acc routine vector
#pragma acc loop vector reduction(+:returned)
#pragma acc routine seq
#pragma acc loop seq reduction(+:returned)
#pragma acc routine worker
#pragma acc loop worker
#pragma acc routine worker
#pragma acc routine worker
#pragma acc routine worker
#pragma acc loop worker
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
