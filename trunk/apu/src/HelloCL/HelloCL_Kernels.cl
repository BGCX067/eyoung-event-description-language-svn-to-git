__kernel void hello(__global int *p)
{
    size_t i =  get_global_id(0);
    size_t j =  get_global_id(1);
}
