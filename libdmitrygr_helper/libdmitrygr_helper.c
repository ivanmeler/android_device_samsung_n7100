#define LOG_TAG "libdmitrygr_helper"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <utils/Log.h>
#include <hardware/power.h>
#include <hardware/hardware.h>

/*
 * CURIOUS WHAT THE HELL IS GOING ON IN HERE? READ UP...
 *
 *
 * Problems:
 * 1. Nexus 10's GPS library was made to work with android L
 * 2. Android M changed a few things around that make it not work
 *   a. Sensor manager API changed in a few places
 *   b. BoringSSL replaced OpenSSL
 * 3. Due to these now-missing unresolved symbols GPS library will not load or run
 *
 * Curious data snippets
 *  Due to peculiarities of the ELF format, when a binary baz imports function foo() from libbar.so,
 *   nowhere in baz's ELF file does it say that foo() must from from libbar. In fact there are two
 *   separate records. One that says that libbar is "NEED"ed, and another that says that there is an
 *   import of function "foo". What that means is that if the process wer to also load libxyz, which
 *   also exported foo(), there is no way to be sure which foo() would get called. Why do we care?
 *   Well, consider out problems above. We need to provide functions and variables that existing
 *   libraries no longer do. How?
 *
 * A tricky but clever solution: INTERPOSITION library
 * 1. We'll edit the GPS library and replace one of its "NEED" record with one referencing a new library
 *    which we'll create. Need a library name? why not "lidmitry"?
 * 2. Make sure that lidmitry's NEED records include the library whose record we replaced in the GPS
 *    library, to make sure that the linker brings it in afterall and all symbols in it are found
 * 3. Implement libdmitry such that it provides the missing things and does them in such a way that the
 *    GPS library is happy.
 * 4. Complications exist:
 *   a. This would be impossible to do in C++, as the compiler would barf at us implementing random
 *      chunks of random classes we do not control. Luckily, the linker has no idea about C++, C, or other
 *      such things. C++ names get converted to special symbol names by the compiler, and thus the linker
 *      is neevr even aware of overloading or such things. This process is called mangling. So we'll just
 *      export the C++ functions we need with the proper mangled names, and code them in C. This means
 *      that we need to follow the proper calling conventions by hand, etc. With some dissasembling to see
 *      how GCC does it, we can duplicate it here, as I did.
 *   b. Not all missing things are functions. There are a few variables that need to be exported and are
 *      not present in M's code. We have to provide them. Luckily, just like with functions, as long as our
 *      mandled name matches, the linker will be happy to make the connection for us.
 *   c. Some cleanup may be needed on exit. Luckily, there is a way to register functions to be called
 *      upon library load and unload. I use that here to free some state that may be left over on exit.
 *
 * Result: GPS library works on M, with the help of libdmitry and a small binary patch to the GPS
 *         library itself (replacing one of the "NEED" records with a NEED record for "libdmitry"
 */




//library on-load and on-unload handlers (to help us set things up and tear them down)
    void libEvtLoading(void) __attribute__((constructor));
    void libEvtUnloading(void) __attribute__((destructor));

//what exists
uintptr_t _ZN7android6Parcel13writeString16EPKDsj(void* thisPtr, void *str, size_t len);



//android::Parcel::writeString16(unsigned short const*, unsigned int)
uintptr_t _ZN7android6Parcel13writeString16EPKtj(void* thisPtr, void *str, size_t len)
{
    return 	_ZN7android6Parcel13writeString16EPKDsj(thisPtr, str, len);
}



/*
 * FUNCTION: libEvtLoading()
 * USE:      Handle library loading
 * NOTES:    This is a good time to log the fact that we were loaded and plan to
 *           do our thing.
 */
void libEvtLoading(void)
{
    ALOGI("Nexus S RIL interposition library loaded. Your radio should work in M now.");
}

/*
 * FUNCTION: libEvtUnloading()
 * USE:      Handle library unloading
 * NOTES:    This is a good time to free whatever is unfreed and say goodbye
 */
void libEvtUnloading(void)
{
    ALOGI("Nexus S RIL interposition library unloading. Goodbye...");
}
