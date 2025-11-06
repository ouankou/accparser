#pragma acc routine(test_routine_gang_loop_named) gang
#pragma acc routine(test_routine_gang_gang_named) gang
#pragma acc routine(test_routine_gang_worker_named) gang
#pragma acc routine(test_routine_gang_vector_named) gang
#pragma acc routine(test_routine_gang_seq_named) gang
#pragma acc routine worker
#pragma acc loop worker reduction(+:returned)
#pragma acc routine vector
#pragma acc loop vector reduction(+:returned)
#pragma acc routine seq
#pragma acc loop seq reduction(+:returned)
#pragma acc routine gang
#pragma acc loop gang
#pragma acc loop worker reduction(+:privatized_return)
#pragma acc loop seq
#pragma acc routine gang
#pragma acc loop gang private(temp)
#pragma acc loop worker reduction(+:temp)
#pragma acc routine gang
#pragma acc routine gang
#pragma acc routine gang
#pragma acc routine gang
#pragma acc loop gang private(temp)
#pragma acc loop worker reduction(+:temp)
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
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
