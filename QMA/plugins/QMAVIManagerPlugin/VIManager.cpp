/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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
#include "VIManager.h"

#include <QtCore>
#include <MMDME/Common.h>

/* get_token_from_string: get token from string */
static int get_token_from_string(char *str, int *index, char *buff)
{
    char c;
    int i = 0;

    c = str[(*index)];
    while (c == '\0' || c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        if (c == '\0') {
            buff[0] = '\0';
            return 0;
        }
        c = str[++(*index)];
    }

    while (c != '\0' && c != ' ' && c != '\t' && c != '\n' && c != '\r') {
        buff[i++] = c;
        c = str[++(*index)];
    }

    buff[i] = '\0';
    return i;
}

/* get_arg_from_string: get argument from string using separators */
static int get_arg_from_string(const char *str, int *index, char *buff)
{
    char c;
    int i = 0;

    c = str[(*index)];
    while (c == '\0' || c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        if (c == '\0') {
            buff[0] = '\0';
            return 0;
        }
        c = str[++(*index)];
    }

    while (c != '\0' && c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != VIMANAGER_SEPARATOR1 && c != VIMANAGER_SEPARATOR2) {
        buff[i++] = c;
        c = str[++(*index)];
    }
    if (c == VIMANAGER_SEPARATOR1 || c == VIMANAGER_SEPARATOR2)
        (*index)++;

    buff[i] = '\0';
    return i;
}

/* copy_string: copy string instead of strdup */
static char *copy_string(const char *str)
{
    char *buff = (char *) calloc(strlen(str) + 1, sizeof(char));
    strcpy(buff, str);
    return buff;
}

/* get_args: get event arguments */
static int get_args(const char *str, char ***args, int *argc)
{
    int i, j, len, idx = 0;
    char buff[VIMANAGER_MAXBUFLEN];

    if (str == NULL) {
        (*argc) = 0;
        (*args) = NULL;
        return 0;
    }

    len = strlen(str);
    if (len <= 0) {
        (*argc) = 0;
        (*args) = NULL;
        return 0;
    }

    /* get number of separator */
    (*argc) = 1;
    for (i = 0; i < len; i++)
        if (str[i] == VIMANAGER_SEPARATOR1 || str[i] == VIMANAGER_SEPARATOR2)
            (*argc)++;
    (*args) = (char **) calloc((*argc), sizeof(char *));
    for (i = 0; i < (*argc); i++)
        (*args)[i] = NULL;

    /* get event arguments */
    for (i = 0; i < (*argc); i++) {
        j = get_arg_from_string(str, &idx, buff);
        (*args)[i] = (char *) calloc(j + 1, sizeof(char));
        strcpy((*args)[i], buff);
    }

    return 1;
}

/* VIManager_Arc_initialize: initialize arc */
void VIManager_Arc_initialize(VIManager_Arc * a, char *input_event_type, char **input_event_args, int input_event_argc, char *output_command_type, char *output_command_args, VIManager_State * next_state)
{
    int i;

    if (input_event_type == NULL)
        a->input_event_type = NULL;
    else
        a->input_event_type = copy_string(input_event_type);
    if (input_event_argc <= 0) {
        a->input_event_args = NULL;
        a->input_event_argc = 0;
    } else {
        a->input_event_args = (char **) calloc(input_event_argc, sizeof(char *));
        for (i = 0; i < input_event_argc; i++)
            a->input_event_args[i] = copy_string(input_event_args[i]);
        a->input_event_argc = input_event_argc;
    }
    if (output_command_type == NULL)
        a->output_command_type = NULL;
    else
        a->output_command_type = copy_string(output_command_type);
    if (output_command_args == NULL)
        a->output_command_args = NULL;
    else
        a->output_command_args = copy_string(output_command_args);
    a->next_state = next_state;
    a->next = NULL;
}

/* VIManager_Arc_clear: free arc */
void VIManager_Arc_clear(VIManager_Arc * a)
{
    int i;

    if (a->input_event_type != NULL)
        free(a->input_event_type);
    if (a->input_event_args != NULL) {
        for (i = 0; i < a->input_event_argc; i++)
            free(a->input_event_args[i]);
        free(a->input_event_args);
    }
    if (a->output_command_type != NULL)
        free(a->output_command_type);
    if (a->output_command_args != NULL)
        free(a->output_command_args);
    VIManager_Arc_initialize(a, NULL, NULL, 0, NULL, NULL, NULL);
}

/* VIManager_AList_initialize: initialize arc list */
void VIManager_AList_initialize(VIManager_AList * l)
{
    l->head = NULL;
}

/* VIManager_AList_clear: free arc list */
void VIManager_AList_clear(VIManager_AList * l)
{
    VIManager_Arc *tmp1, *tmp2;

    for (tmp1 = l->head; tmp1 != NULL; tmp1 = tmp2) {
        tmp2 = tmp1->next;
        VIManager_Arc_clear(tmp1);
        free(tmp1);
    }
    l->head = NULL;
}

/* VIManager_State_initialize: initialize state */
void VIManager_State_initialize(VIManager_State * s, unsigned int number, VIManager_State * next)
{
    s->number = number;
    VIManager_AList_initialize(&s->arc_list);
    s->next = next;
}

/* VIManager_State_clear: free state */
void VIManager_State_clear(VIManager_State * s)
{
    VIManager_AList_clear(&s->arc_list);
    VIManager_State_initialize(s, 0, NULL);
}

/* VIManager_SList_initialize: initialize state list */
void VIManager_SList_initialize(VIManager_SList * l)
{
    l->head = NULL;
}

/* VIManager_SList_clear: free state list */
void VIManager_SList_clear(VIManager_SList * l)
{
    VIManager_State *tmp1, *tmp2;

    for (tmp1 = l->head; tmp1 != NULL; tmp1 = tmp2) {
        tmp2 = tmp1->next;
        VIManager_State_clear(tmp1);
        free(tmp1);
    }
    l->head = NULL;
}

/* VIManager_SList_search_state: search state pointer */
VIManager_State *VIManager_SList_search_state(VIManager_SList * l, unsigned int n)
{
    VIManager_State *tmp1, *tmp2, *result = NULL;

    if (l->head == NULL) {
        l->head = (VIManager_State *) calloc(1, sizeof(VIManager_State));
        VIManager_State_initialize(l->head, n, NULL);
        result = l->head;
    } else if (l->head->number == n) {
        result = l->head;
    } else if (l->head->number > n) {
        tmp1 = l->head;
        l->head = (VIManager_State *) calloc(1, sizeof(VIManager_State));
        VIManager_State_initialize(l->head, n, tmp1);
        result = l->head;
    } else {
        for (tmp1 = l->head; tmp1 != NULL; tmp1 = tmp1->next) {
            if (tmp1->next == NULL) {
                tmp1->next = (VIManager_State *) calloc(1, sizeof(VIManager_State));
                VIManager_State_initialize(tmp1->next, n, NULL);
                result = tmp1->next;
                break;
            } else if (tmp1->next->number == n) {
                result = tmp1->next;
                break;
            } else if (n < tmp1->next->number) {
                tmp2 = tmp1->next;
                tmp1->next = (VIManager_State *) calloc(1, sizeof(VIManager_State));
                VIManager_State_initialize(tmp1->next, n, tmp2);
                result = tmp1->next;
                break;
            }
        }
    }
    return result;
}

/* VIManager_SList_add_arc: add arc */
void VIManager_SList_add_arc(VIManager_SList * l, int index_s1, int index_s2, char *isymbol, char *osymbol)
{
    int i, idx;
    VIManager_State *s1, *s2;
    VIManager_Arc *a1, *a2;
    VIManager_AList *arc_list;

    char type[VIMANAGER_MAXBUFLEN];
    char **args = NULL;
    int argc = 0;
    char otype[VIMANAGER_MAXBUFLEN];
    char oargs[VIMANAGER_MAXBUFLEN];

    s1 = VIManager_SList_search_state(l, index_s1);
    s2 = VIManager_SList_search_state(l, index_s2);
    arc_list = &s1->arc_list;

    /* analysis input symbol */
    idx = 0;
    i = get_arg_from_string(isymbol, &idx, type);
    if (i <= 0)
        return;
    get_args(&isymbol[idx], &args, &argc);

    /* analysis output symbol */
    idx = 0;
    i = get_arg_from_string(osymbol, &idx, otype);
    if (i <= 0)
        return;
    get_token_from_string(osymbol, &idx, oargs);

    /* create */
    a1 = (VIManager_Arc *) calloc(1, sizeof(VIManager_Arc));
    VIManager_Arc_initialize(a1, type, args, argc, otype, oargs, s2);

    /* set */
    if (arc_list->head == NULL) {
        arc_list->head = a1;
    } else {
        for (a2 = arc_list->head; a2->next != NULL; a2 = a2->next){
        }
        a2->next = a1;
    }

    /* free buffer */
    if (argc > 0) {
        for (i = 0; i < argc; i++)
            free(args[i]);
        free(args);
    }
}

/* VIManager::initialize: initialize VIManager */
void VIManager::initialize()
{
    VIManager_SList_initialize(&m_stateList);
    m_currentState = NULL;
}

/* VIManager:clear: free VIManager */
void VIManager::clear()
{
    VIManager_SList_clear(&m_stateList);
    m_currentState = NULL;
}

/* VIManager::VIManager: constructor */
VIManager::VIManager()
{
    initialize();
}

/* VIManager::~VIManager: destructor */
VIManager::~VIManager()
{
    clear();
}

/* VIManager::load: load FST */
int VIManager::load(QTextStream &stream)
{
    /* unload */
    VIManager_SList_clear(&m_stateList);
    VIManager_SList_initialize(&m_stateList);
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");

    while (!stream.atEnd()) { /* string + \r + \n + \0 */
        QString line = stream.readLine().trimmed();
        /* check and load arc */
        if (!line.isEmpty() && line[0] != '#') {
            char buff[VIMANAGER_MAXBUFLEN];
            char buff_s1[VIMANAGER_MAXBUFLEN];
            char buff_s2[VIMANAGER_MAXBUFLEN];
            char buff_is[VIMANAGER_MAXBUFLEN];
            char buff_os[VIMANAGER_MAXBUFLEN];
            char buff_er[VIMANAGER_MAXBUFLEN];
            QByteArray bytes = line.replace(codec->toUnicode("\\"), "/").toUtf8();
            MMDAIStringCopySafe(buff, bytes.constData(), sizeof(buff));
            int idx = 0;
            int size_s1 = get_token_from_string(buff, &idx, buff_s1);
            int size_s2 = get_token_from_string(buff, &idx, buff_s2);
            int size_is = get_token_from_string(buff, &idx, buff_is);
            int size_os = get_token_from_string(buff, &idx, buff_os);
            int size_er = get_token_from_string(buff, &idx, buff_er);
            if (size_s1 > 0 && size_s2 > 0 && size_is > 0 && size_os > 0 && size_er == 0 && buff_s1[0] != VIMANAGER_COMMENT) {
                char *err_s1 = NULL, *err_s2 = NULL;
                unsigned int index_s1 = (unsigned int) strtoul(buff_s1, &err_s1, 10);
                unsigned int index_s2 = (unsigned int) strtoul(buff_s2, &err_s2, 10);
                if (buff_s1 + size_s1 == err_s1 && buff_s2 + size_s2 == err_s2)
                    VIManager_SList_add_arc(&m_stateList, index_s1, index_s2, buff_is, buff_os);
            }
        }
    }

    /* set current state to zero */
    m_currentState = VIManager_SList_search_state(&m_stateList, VIMANAGER_STARTSTATE);

    return 1;
}

/* VIManager::transition: state transition (if jumped, return 1) */
int VIManager::transition(const char *itype, const char *iargs, char *otype, char *oargs)
{
    int i, j;
    int jumped = 0;

    VIManager_Arc *arc;
    VIManager_AList *arc_list;

    char **args;
    int argc;

    strcpy(otype, VIMANAGER_EPSILON);
    strcpy(oargs, "");

    /* FST isn't loaded yet */
    if (m_currentState == NULL)
        return jumped;

    /* state don't have arc list */
    arc_list = &m_currentState->arc_list;
    if (arc_list->head == NULL)
        return jumped;

    /* get input event args */
    get_args(iargs, &args, &argc);

    /* matching */
    for (arc = arc_list->head; arc != NULL; arc = arc->next) {
        if (strcmp(itype, arc->input_event_type) == 0) {
            if (strcmp(itype, VIMANAGER_RECOG_EVENT_STOP) == 0) {
                /* for recognition event */
                for (i = 0; i < arc->input_event_argc; i++) {
                    jumped = 0;
                    for (j = 0; j < argc; j++) {
                        if (strcmp(arc->input_event_args[i], args[j]) == 0) {
                            jumped = 1;
                            break;
                        }
                    }
                    if (jumped == 0)
                        break;
                }
            } else if (argc == arc->input_event_argc) {
                /* for others */
                jumped = 1;
                for (i = 0; i < argc; i++) {
                    if (strcmp(args[i], arc->input_event_args[i]) != 0) {
                        jumped = 0;
                        break;
                    }
                }
            }
            if (jumped) { /* state transition */
                strcpy(otype, arc->output_command_type);
                strcpy(oargs, arc->output_command_args);
                m_currentState = arc->next_state;
                break;
            }
        }
    }

    if (argc > 0) {
        for (i = 0; i < argc; i++)
            free(args[i]);
        free(args);
    }

    return jumped;
}
