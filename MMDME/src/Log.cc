/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn (libMMDAI)                         */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAgent project team nor the names of  */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "MMDME/MMDME.h"

static MMDAILoggingHandler MMDAILogWriteNull, *g_handler = MMDAILogWriteNull;
static MMDAILoggingSJISHandler MMDAILogWriteNullSJIS, *g_handlerSJIS = MMDAILogWriteNullSJIS;

static void MMDAILogWriteNull(const char *file,
                              const int line,
                              const enum MMDAILogLevel level,
                              const char *format,
                              va_list ap)
{
    /* do nothing */
    (void) file;
    (void) line;
    (void) level;
    (void) format;
    (void) ap;
}

static void MMDAILogWriteNullSJIS(const char *file,
                                  const int line,
                                  const enum MMDAILogLevel level,
                                  const char *format, va_list ap)
{
    /* do nothing */
    (void) file;
    (void) line;
    (void) level;
    (void) format;
    (void) ap;
}

void MMDAILogSetHandler(MMDAILoggingHandler *handler)
{
    g_handler = handler;
}

void MMDAILogSetHandlerSJIS(MMDAILoggingHandler *handler)
{
    g_handlerSJIS = handler;
}

void MMDAILogWrite(const char *file,
                   const int line,
                   const enum MMDAILogLevel level,
                   const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    g_handler(file, line, level, format, ap);
    va_end(ap);
}

void MMDAILogWriteSJIS(const char *file,
                       const int line,
                       const enum MMDAILogLevel level,
                       const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    g_handlerSJIS(file, line, level, format, ap);
    va_end(ap);
}

