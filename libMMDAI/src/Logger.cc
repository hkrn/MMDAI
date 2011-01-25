/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn (libMMDAI)                         */
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
/* - Neither the name of the MMDAI project team nor the names of     */
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

/* headers */

#include <GLee.h>

#if defined(__APPLE__)
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <stdarg.h>

#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32_
#include <malloc.h>
#else
#include <string.h>
#endif

#include "Logger.h"
#include "TextRenderer.h"

#define LOG_MAXBUFLEN 1024
#define LOG_COLOR     1.0f,0.7f,0.0f,0.7f /* text color */
#define LOG_BGCOLOR   0.0f,0.0f,0.0f,0.8f /* background color */

/* Logger::initialize: initialize logger */
void Logger::initialize()
{
   m_textWidth = 0;
   m_textHeight = 0;
   m_textX = 0.0;
   m_textY = 0.0;
   m_textZ = 0.0;
   m_textScale = 0.0;

   m_textBufArray = NULL;
   m_displayListIdArray = NULL;
   m_length = NULL;
   m_updated = NULL;
}

/* Logger::clear: free logger */
void Logger::clear()
{
   int i;

   if (m_textBufArray) {
      for (i = 0; i < m_textHeight; i++)
         free(m_textBufArray[i]);
      free(m_textBufArray);
   }
   if (m_displayListIdArray) {
      for (i = 0; i < m_textHeight; i++)
         free(m_displayListIdArray[i]);
      free(m_displayListIdArray);
   }
   if (m_length)
      free(m_length);
   if (m_updated)
      free(m_updated);
   initialize();
}

/* Logger::Logger: constructor */
Logger::Logger()
{
   initialize();
}

/* Logger::~Logger: destructor */
Logger::~Logger()
{
   clear();
}

/* Logger::setup: initialize and setup logger with args */
void Logger::setup(int *size, float *pos, float scale)
{
   int i;
   int w = size[0];
   int h = size[1];
   float x = pos[0];
   float y = pos[1];
   float z = pos[2];

   if (w <= 0 || h <= 0 || scale <= 0.0)
     return;

   clear();

   m_textWidth = w;
   m_textHeight = h;
   m_textX = x;
   m_textY = y;
   m_textZ = z;
   m_textScale = scale;

   m_textBufArray = (char **) malloc(sizeof(char *) * m_textHeight);
   for (i = 0; i < m_textHeight; i++) {
      m_textBufArray[i] = (char *) malloc(sizeof(char) * m_textWidth);
      m_textBufArray[i][0] = L'\0';
   }

   m_displayListIdArray = (unsigned int **) malloc(sizeof(unsigned int *) * m_textHeight);
   for (i = 0; i < m_textHeight; i++)
      m_displayListIdArray[i] = (unsigned int *) malloc(sizeof(unsigned int) * m_textWidth);

   m_length = (int *) malloc(sizeof(int) * m_textHeight);
   for (i = 0; i < m_textHeight; i++)
      m_length[i] = -1;

   m_updated = (bool *) malloc(sizeof(bool) * m_textHeight);
   for (i = 0; i < m_textHeight; i++)
      m_updated[i] = false;

   m_textLine = 0;
}

/* Logger::log: store log text */
void Logger::log(const char *format, ...)
{
   char *p, *psave;
   char buff[LOG_MAXBUFLEN];
   va_list args;

   if (!m_textBufArray)
     return;

   va_start(args, format);
   if (m_textBufArray) {
      snprintf(buff, LOG_MAXBUFLEN - 1, format, args);
      printf("%s\n", buff);
      fflush(stdout);
      buff[LOG_MAXBUFLEN - 1] = '\0';
#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32)
      for (p = strtok_r(buff, "\n", &psave); p; p = strtok_r(NULL, "\n", &psave)) {
#else
      (void)psave;
      for (p = strtok(buff, "\n"); p; p = strtok(NULL, "\n")) {
#endif
         strncpy(m_textBufArray[m_textLine], p, m_textWidth - 1);
         m_textBufArray[m_textLine][m_textWidth - 1] = L'\0';
         m_updated[m_textLine] = true;
         if (++m_textLine >= m_textHeight)
            m_textLine = 0;
      }
   }
   va_end(args);
}

/* Logger::render: render log text */
void Logger::render(TextRenderer *text)
{
   int i, j;
   float x, y, z, w, h;

   if (text == NULL || !m_textBufArray)
     return;

   x = m_textX;
   y = m_textY;
   z = m_textZ;
   w = 0.5f * (float) (m_textWidth) * 0.85f + 1.0f;
   h = 1.0f * (float) (m_textHeight) * 0.85f + 1.0f;

   glPushMatrix();
   glDisable(GL_CULL_FACE);
   glDisable(GL_LIGHTING);
   glScalef(m_textScale, m_textScale, m_textScale);
   glNormal3f(0.0f, 1.0f, 0.0f);
   glColor4f(LOG_BGCOLOR);
   glBegin(GL_QUADS);
   glVertex3f(x    , y    , z);
   glVertex3f(x + w, y    , z);
   glVertex3f(x + w, y + h, z);
   glVertex3f(x    , y + h, z);
   glEnd();
   glTranslatef(x + 0.5f, y + h - 0.4f, z + 0.01f);
   for (i = 0; i < m_textHeight; i++) {
      glTranslatef(0.0f, -0.85f, 0.0f);
      j = m_textLine + i;
      if (j >= m_textHeight)
         j -= m_textHeight;
      if (m_textBufArray[j][0] != L'\0') {
         glColor4f(LOG_COLOR);
         glPushMatrix();
         if (m_updated[j]) {
            /* cache display list array */
            m_length[j] = text->getDisplayListArrayOfString(m_textBufArray[j], m_displayListIdArray[j], m_textWidth);
            m_updated[j] = false;
         }
         if (m_length[j] >= 0)
            text->renderDisplayListArrayOfString(m_displayListIdArray[j], m_length[j]);
         glPopMatrix();
      }
   }
   glEnable(GL_LIGHTING);
   glEnable(GL_CULL_FACE);
   glPopMatrix();
}
