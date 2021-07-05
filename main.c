/*
 * Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include "utils.h"
#include "egl.h"
#include "kms.h"
#include "eglgears.h"

/*
 * Example code demonstrating how to connect EGL to DRM KMS using
 * EGLStreams.
 */

extern EGLStreamKHR eglStream;

int main(void)
{
    EGLDisplay eglDpy;
    EGLDeviceEXT eglDevice;
    int drmFd, width, height;
    uint32_t planeID = 0;
    EGLSurface eglSurface;

    GetEglExtensionFunctionPointers();

    eglDevice = GetEglDevice();

    drmFd = GetDrmFd(eglDevice);

    SetMode(drmFd, &planeID, &width, &height);

    eglDpy = GetEglDisplay(eglDevice, drmFd);

    eglSurface = SetUpEgl(eglDpy, planeID, width, height);

    InitGears(width, height);

    EGLBoolean acquireOk = 1;
    while(1) {
        DrawGears();
	if (acquireOk) { // Protection against EGL_RESOURCE_BUSY_EXT leading to a deadlock
        	eglSwapBuffers(eglDpy, eglSurface);
	}

	
	struct timespec cur_time;                                                                                                                                                                                                       
        clock_gettime(CLOCK_MONOTONIC, &cur_time);
        long last_ms = cur_time.tv_sec * 1000 + cur_time.tv_nsec / 1000000;
	EGLAttrib acquire_attribs[] = { EGL_NONE };
	acquireOk = pEglStreamConsumerAcquireAttribNV(eglDpy, eglStream, acquire_attribs);
        clock_gettime(CLOCK_MONOTONIC, &cur_time);
	long cur_ms = cur_time.tv_sec * 1000 + cur_time.tv_nsec / 1000000;
	long delta = cur_ms - last_ms;
	printf("eglStreamConsumerAcquireAttribNV exec time: %ld ms\n", delta);
	if (delta < 16) {
		usleep((16 - delta) * 1000); // bogus vsync to 60hz
	}
        PrintFps();
    }

    return 0;
}
