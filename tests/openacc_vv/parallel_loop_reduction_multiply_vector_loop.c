#pragma acc data copyin(a[0:10*multiplicitive_n], b[0:10*multiplicitive_n]) copyout(c[0:10])
#pragma acc parallel loop private(temp)
#pragma acc loop vector reduction(*:temp)
#pragma acc data copyin(a[0:25*m_n], b[0:25*m_n]) copyout(c[0:25*m_n], totals[0:25])
#pragma acc parallel loop gang private(reduced)
#pragma acc loop worker reduction(*:reduced)
#pragma acc loop worker
