#define CL_USE_DEPRECATED_OPENCL_1_1_APIS

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <SDKFile.hpp>
#include <SDKCommon.hpp>
#include <SDKApplication.hpp>

#define __NO_STD_STRING

#include <CL/cl.hpp>

#define BUF_LEN 512
#define BUF_SET 10
typedef struct apu_share
{
	int flag;
	int length;
	int total;
	int id;
	char buf[BUF_LEN];
}apu_share_t;

int main()
{
    cl_int err;

    // Platform info
    std::vector<cl::Platform> platforms;
    std::cout << "HelloCL!\nGetting Platform Information\n";
    err = cl::Platform::get(&platforms);
    if(err != CL_SUCCESS)
    {
        std::cout << "Platform::get() failed (" << err << ")" << std::endl;
        return SDK_FAILURE;
    }

    std::vector<cl::Platform>::iterator i;
    if(platforms.size() > 0)
    {
        for(i = platforms.begin(); i != platforms.end(); ++i)
        {
            if(!strcmp((*i).getInfo<CL_PLATFORM_VENDOR>(&err).c_str(), "Advanced Micro Devices, Inc."))
            {
                break;
            }
        }
    }
    if(err != CL_SUCCESS)
    {
        std::cout << "Platform::getInfo() failed (" << err << ")" << std::endl;
        return SDK_FAILURE;
    }

    /* 
     * If we could find our platform, use it. Otherwise pass a NULL and get whatever the
     * implementation thinks we should be using.
     */

    cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(*i)(), 0 };

    std::cout << "Creating a context AMD platform\n";
    cl::Context context(CL_DEVICE_TYPE_CPU, cps, NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        std::cout << "Context::Context() failed (" << err << ")\n";
        return SDK_FAILURE;
    }

    std::cout << "Getting device info\n";
    std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    if (err != CL_SUCCESS) {
        std::cout << "Context::getInfo() failed (" << err << ")\n";
        return SDK_FAILURE;
    }
    if (devices.size() == 0) {
        std::cout << "No device available\n";
        return SDK_FAILURE;
    }

    std::cout << "Loading and compiling CL source\n";
    streamsdk::SDKFile file;
    if (!file.open("test_kernel.cl")) {
         std::cout << "We couldn't load CL source code\n";
         return SDK_FAILURE;
    }
    cl::Program::Sources sources(1, std::make_pair(file.source().data(), file.source().size()));

    cl::Program program = cl::Program(context, sources, &err);
    if (err != CL_SUCCESS) {
        std::cout << "Program::Program() failed (" << err << ")\n";
        return SDK_FAILURE;
    }

    err = program.build(devices);
    if (err != CL_SUCCESS) {

        if(err == CL_BUILD_PROGRAM_FAILURE)
        {
            cl::string str = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);

            std::cout << " \n\t\t\tBUILD LOG\n";
            std::cout << " ************************************************\n";
            std::cout << str.c_str() << std::endl;
            std::cout << " ************************************************\n";
        }

        std::cout << "Program::build() failed (" << err << ")\n";
        return SDK_FAILURE;
    }

    cl::Kernel kernel(program, "hello", &err);
    if (err != CL_SUCCESS) {
        std::cout << "Kernel::Kernel() failed (" << err << ")\n";
        return SDK_FAILURE;
    }
    if (err != CL_SUCCESS) {
        std::cout << "Kernel::setArg() failed (" << err << ")\n";
        return SDK_FAILURE;
    }

    cl::CommandQueue queue(context, devices[0], 0, &err);
    if (err != CL_SUCCESS) {
        std::cout << "CommandQueue::CommandQueue() failed (" << err << ")\n";
        return SDK_FAILURE;
    }
	
	cl::Buffer in_out_buffer(context, CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, sizeof(apu_share_t), NULL, &err);
    if (err != CL_SUCCESS) {
        std::cout << "Buffer created failed (" << err << ")\n";
        return SDK_FAILURE;
    }
	
	apu_share_t *data = NULL;
	data = (apu_share_t*)queue.enqueueMapBuffer(in_out_buffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, sizeof(apu_share_t), NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        std::cout << "Buffer map failed (" << err << ")\n";
        return SDK_FAILURE;
    }
	
	data->flag = 0;
	data->length = BUF_LEN;
	data->total = -1;
	data->id = -1;
	memset(data->buf, BUF_SET, BUF_LEN);
	std::cout << "Graphic Operating, we set total:" << data->total << "\n";
	std::cout << "Graphic Operating, we set flag:" << data->flag << "\n";
	std::cout << "Graphic Operating, we set flag:" << data->length<< "\n";
	std::cout << "Graphic Operating, we set flag:" << data->id<< "\n";
	queue.enqueueUnmapMemObject(in_out_buffer, data);

	err = kernel.setArg(0, in_out_buffer);
    if (err != CL_SUCCESS) {
        std::cout << "Set arg 0 map failed (" << err << ")\n";
        return SDK_FAILURE;
    }

	cl::Event event;
    std::cout<<"Running CL program\n";
    err = queue.enqueueNDRangeKernel( kernel, cl::NullRange, cl::NDRange(1), cl::NullRange, NULL, &event);

    if (err != CL_SUCCESS) {
        std::cout << "CommandQueue::enqueueNDRangeKernel()" \
            " failed (" << err << ")\n";
       return SDK_FAILURE;
    }
	
	data = (apu_share_t*)queue.enqueueMapBuffer(in_out_buffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, sizeof(apu_share_t), NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        std::cout << "Buffer map failed (" << err << ")\n";
        return SDK_FAILURE;
    }
	
	data->flag = 1;
	volatile int flag = 0;
	while(1)
	{
		flag = data->flag;
		if(!flag)
			break;
	}
	std::cout << "After Graphic Operating, we get total:" << data->total << "\n";
	std::cout << "After Graphic Operating, we get flag:" << data->flag << "\n";
	std::cout << "After Graphic Operating, we get flag:" << data->length<< "\n";
	std::cout << "After Graphic Operating, we get flag:" << data->id<< "\n";
	queue.enqueueUnmapMemObject(in_out_buffer, (void*)data);

    err = queue.finish();
    if (err != CL_SUCCESS) {
        std::cout << "Event::wait() failed (" << err << ")\n";
    }

    std::cout<<"Done\nPassed!\n" << std::endl;
    return SDK_SUCCESS;
}
