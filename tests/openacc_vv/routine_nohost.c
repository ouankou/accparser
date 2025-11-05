#pragma acc routine worker nohost
#pragma acc loop worker reduction(+:returned)
#pragma acc routine seq bind(f)
#pragma acc data copyin(a[0:n][0:n]) copy(b[0:n])
#pragma acc parallel
#pragma acc loop gang
