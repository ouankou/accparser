#pragma acc data copyin(a[0:multiplicitive_n], b[0:multiplicitive_n]) copy(multiplied_total)
#pragma acc parallel loop reduction(*:multiplied_total)
#pragma acc data copyin(a[0:10*m_n], b[0:10*m_n])
#pragma acc parallel loop reduction(*:multiplicitive_total)
