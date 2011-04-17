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

#ifndef VIMANAGER_H_
#define VIMANAGER_H_

#include <QTextStream>

#define VIMANAGER_MAXBUFLEN        65536
#define VIMANAGER_SEPARATOR1       '|'
#define VIMANAGER_SEPARATOR2       ','
#define VIMANAGER_COMMENT          '#'
#define VIMANAGER_STARTSTATE       0
#define VIMANAGER_EPSILON          "<eps>"
#define VIMANAGER_RECOG_EVENT_STOP "RECOG_EVENT_STOP"

/* VIManager_Arc: arc */
typedef struct _VIManager_Arc {
    char *input_event_type;
    char **input_event_args;
    int input_event_argc;
    char *output_command_type;
    char *output_command_args;
    struct _VIManager_State *next_state;
    struct _VIManager_Arc *next;
} VIManager_Arc;

/* VIManager_Arc_initialize: initialize arc */
void VIManager_Arc_initialize(VIManager_Arc * a, char *input_event_type, char **input_event_args, int input_event_argc, char *output_command_type, char *output_command_args, struct _VIManager_State * next_state);

/* VIManager_Arc_clear: free arc */
void VIManager_Arc_clear(VIManager_Arc * a);

/* VIManager_ALis: arc list */
typedef struct _VIManager_AList {
    VIManager_Arc *head;
} VIManager_AList;

/* VIManager_AList_initialize: initialize arc list */
void VIManager_AList_initialize(VIManager_AList * l);

/* VIManager_AList_clear: free arc list */
void VIManager_AList_clear(VIManager_AList * l);

/* VIManager_State: state */
typedef struct _VIManager_State {
    unsigned int number;
    struct _VIManager_AList arc_list;
    struct _VIManager_State *next;
} VIManager_State;

/* VIManager_State_initialize: initialize state */
void VIManager_State_initialize(VIManager_State * s, unsigned int number, VIManager_State * next);

/* VIManager_State_clear: free state */
void VIManager_State_clear(VIManager_State * s);

/* VIManager_SList: state list */
typedef struct _VIManager_SList {
    VIManager_State *head;
} VIManager_SList;

/* VIManager_SList_initialize: initialize state list */
void VIManager_SList_initialize(VIManager_SList * l);

/* VIManager_SList_clear: free state list */
void VIManager_SList_clear(VIManager_SList * l);

/* VIManager_SList_search_state: search state pointer */
VIManager_State *VIManager_SList_search_state(VIManager_SList * l, unsigned int n);

/* VIManager_SList_add_arc: add arc */
void VIManager_SList_add_arc(VIManager_SList *l, int index_s1, int index_s2, char *isymbol, char *osymbol);

/* VIManager: Voice Interaction Manager */
class VIManager
{
private:

    VIManager_SList m_stateList;     /* state list */
    VIManager_State *m_currentState; /* pointer of current state */

    /* initialize: initialize VIManager */
    void initialize();

    /* clear: free VIManager */
    void clear();

public:

    /* VIManager: constructor */
    VIManager();

    /* ~VIManager: destructor */
    ~VIManager();

    /* load: load FST */
    int load(QTextStream &stream);

    /* transition: state transition (if jumped, return 1) */
    int transition(const char *itype, const char* iargs, char *otype, char *oargs);
};

#endif
