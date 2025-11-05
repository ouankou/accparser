#pragma acc data copyin(a[0:multiplicitive_n], b[0:multiplicitive_n]) copy(multiplied_total)
#pragma acc serial loop reduction (*:multiplied_total)
