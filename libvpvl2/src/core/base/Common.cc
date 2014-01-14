/**

 Copyright (c) 2010-2014  hkrn

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

#if defined(VPVL2_ENABLE_QT)
#include <QLoggingCategory>
#elif defined(VPVL2_LINK_GLOG) && !defined(VPVL2_OS_WINDOWS)
#include <sys/fcntl.h>
#include <limits.h>
#endif

namespace {

using namespace vpvl2;

#if defined(VPVL2_LINK_GLOG) && !defined(VPVL2_OS_WINDOWS) && !defined(VPVL2_ENABLE_QT)

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
static bool FLAGS_alsologtostderr = false;
static bool FLAGS_colorlogtostderr = false;
static bool FLAGS_logtostderr = false;
static bool FLAGS_stop_logging_if_full_disk = false;
static int FLAGS_v = 0;
static const char *FLAGS_log_dir = 0;
namespace google {
static inline void InstallFailureSignalHandler() {}
static inline void InitGoogleLogging(const char * /* argv0 */) {
    FLAGS_alsologtostderr = false;
    FLAGS_colorlogtostderr = false;
    FLAGS_logtostderr = false;
    FLAGS_stop_logging_if_full_disk = false;
    FLAGS_v = 0;
    FLAGS_log_dir = 0;
}
static inline void LogToStderr() {}
static inline void ShutdownGoogleLogging() {}
} /* namespace google */
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

const char *libraryCommitRevisionString() VPVL2_DECL_NOEXCEPT
{
    return internal::kCurrentCommitRevisionString;
}

void installLogger(const char *argv0, const char *logdir, int vlog)
{
#if defined(VPVL2_ENABLE_QT)
    Q_UNUSED(argv0);
    Q_UNUSED(logdir);
    for (int i = INFO; i <= ERROR; i++) {
        QLoggingCategory &category = findLoggingBasicCategory(i);
        category.setEnabled(QtDebugMsg, true);
    }
    for (int i = 1; i <= 3; i++) {
        QLoggingCategory &category = findLoggingVerboseCategory(i);
        category.setEnabled(QtDebugMsg, vlog >= i);
    }
#else
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
#endif
}

void uninstallLogger()
{
    google::ShutdownGoogleLogging();
}

#if defined(VPVL2_ENABLE_QT)
#undef ERROR
QLoggingCategory &findLoggingBasicCategory(int level) {
    switch (level) {
    case INFO: {
        static QLoggingCategory info("vpvl2.logging.info");
        return info;
    }
    case WARNING: {
        static QLoggingCategory warning("vpvl2.logging.warning");
        return warning;
    }
    case ERROR: {
        static QLoggingCategory error("vpvl2.logging.error");
        return error;
    }
    default:
        static QLoggingCategory unknown("vpvl2.logging.unknown");
        return unknown;
    }
}

QLoggingCategory &findLoggingVerboseCategory(int level) {
    switch (level) {
    case 1: {
        static QLoggingCategory level1("vpvl2.logging.verbose.level1");
        return level1;
    }
    case 2: {
        static QLoggingCategory level2("vpvl2.logging.verbose.level2");
        return level2;
    }
    case 3: {
        static QLoggingCategory level3("vpvl2.logging.verbose.level3");
        return level3;
    }
    default:
        static QLoggingCategory unknown("vpvl2.logging.verbose.unknown");
        return unknown;
    }
}
#endif

} /* namespace vpvl2 */
