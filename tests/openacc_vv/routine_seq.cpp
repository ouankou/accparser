#pragma acc routine(test_routine_seq_loop_named) seq
#pragma acc routine(test_routine_seq_seq_named) seq
#pragma acc routine seq
#pragma acc loop seq reduction(+:returned)
#pragma acc routine seq
#pragma acc loop seq
#pragma acc routine seq
#pragma acc loop seq
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
