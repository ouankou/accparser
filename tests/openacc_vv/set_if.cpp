#pragma acc set if(acc_get_device_type() == device_type) default_async(acc_async_default)
#pragma acc set if(acc_get_device_type() != device_type) default_async(acc_async_default)
