#pragma acc routine(test_routine_vector_loop_named) vector
#pragma acc routine(test_routine_vector_vector_named) vector
#pragma acc routine(test_routine_vector_seq_named) vector
#pragma acc routine vector
#pragma acc loop vector reduction(+:returned)
#pragma acc routine seq
#pragma acc loop seq reduction(+:returned)
#pragma acc routine vector
#pragma acc loop vector
#pragma acc routine vector
#pragma acc routine vector
#pragma acc loop vector
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
