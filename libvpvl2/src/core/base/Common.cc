/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#if defined(VPVL2_LINK_GLOG) && !defined(VPVL2_OS_WINDOWS)
#include <sys/fcntl.h>
#include <limits.h>
#endif

namespace {

using namespace vpvl2;

#if defined(VPVL2_LINK_GLOG) && !defined(VPVL2_OS_WINDOWS)

static char g_crashHandlePath[PATH_MAX];

static void HandleFailure(const char *data, int size)
{
    int fd = ::open(g_crashHandlePath, O_WRONLY | O_APPEND | O_CREAT);
    if (fd != -1) {
        ::write(fd, data, size);
        ::close(fd);
    }
}

static void InstallFailureHandler(const char *logdir)
{
    google::InstallFailureSignalHandler();
    google::InstallFailureWriter(HandleFailure);
    static const char kFailureLogFilename[] = "/failure.log";
    internal::snprintf(g_crashHandlePath, sizeof(g_crashHandlePath), "%s/%s", logdir, kFailureLogFilename);
}

#else
#define InstallFailureHandler(logdir)
#endif

}

namespace vpvl2
{

bool isLibraryVersionCorrect(int version) VPVL2_DECL_NOEXCEPT
{
    return internal::kCurrentVersion == version;
}

const char *libraryVersionString() VPVL2_DECL_NOEXCEPT
{
    return internal::kCurrentVersionString;
}

void installLogger(const char *argv0, const char *logdir, int vlog)
{
    VPVL2_CHECK(argv0);
    google::InitGoogleLogging(argv0);
    InstallFailureHandler(logdir);
    FLAGS_v = vlog;
    if (logdir) {
        FLAGS_stop_logging_if_full_disk = true;
        FLAGS_log_dir = logdir;
#ifndef NDEBUG
        FLAGS_alsologtostderr = true;
#endif
    }
    else {
        google::LogToStderr();
        FLAGS_logtostderr = true;
        FLAGS_colorlogtostderr = true;
    }
}

void uninstallLogger()
{
    google::ShutdownGoogleLogging();
}

} /* namespace vpvl2 */
