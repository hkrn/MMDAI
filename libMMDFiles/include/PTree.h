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

/* PTreeNode: data pointer tree */
typedef struct _PTreeNode {
   union {
      void *data;    /* data pointer */
      int thres_bit; /* or thershold bit */
   } value;
   struct _PTreeNode *left0;  /* left child node */
   struct _PTreeNode *right1; /* right child node */
} PTreeNode;

/* PTreeNodeList: list of PTreeNode */
typedef struct _PTreeNodeList {
   PTreeNode *list; /* list of allocated nodes */
   int current;     /* current index */
   int size;        /* size of list */
   struct _PTreeNodeList *next;  /* pointer to next stocker */
} PTreeNodeList;

/* PTree: Pointer tree */
class PTree
{
private:

   PTreeNodeList *m_stocker; /* node stocker */
   PTreeNode *m_root;   /* root index node */

   /* newNode: allocate a new node */
   PTreeNode *newNode();

   /* initialize: initialize PTree */
   void initialize();

   /* clear: free PTree */
   void clear();

public:

   /* PTree: constructor */
   PTree();

   /* PTree: destructor */
   ~PTree();

   /* release: free PTree */
   void release();

   /* add: add an entry to the tree */
   void add(const char *str, void *data, const char *matchStr);

   /* findNearest: return the nearest entry */
   void *findNearest(const char *str);
};
