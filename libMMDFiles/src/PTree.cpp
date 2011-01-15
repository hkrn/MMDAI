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

/* headers */

#include "MMDFiles.h"

/* testBit: test a bit */
static int testBit(const char *str, int slen, int bitplace)
{
   int maskptr;
   const unsigned char mbit[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

   if ((maskptr = bitplace >> 3) > slen)
      return 0;
   return(str[maskptr] & mbit[bitplace & 7]);
}

/* testBitMax: test a bit with max bit limit */
static int testBitMax(const char *str, int bitplace, int maxbitplace)
{
   const unsigned char mbit[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

   if (bitplace >= maxbitplace)
      return 0;
   return (str[bitplace >> 3] & mbit[bitplace & 7]);
}

/* getDiffPoint: return which bit differs first between two strings */
static int getDiffPoint(const char *str1, const char *str2)
{
   int p = 0;
   int bitloc;
   int slen1, slen2;

   while (str1[p] == str2[p])
      p++;
   bitloc = p * 8;
   slen1 = strlen( str1 );
   slen2 = strlen( str2 );
   while (testBit( str1, slen1, bitloc) == testBit(str2, slen2, bitloc))
      bitloc++;

   return bitloc;
}

/* PTree::newNode: allocate a new node */
PTreeNode * PTree::newNode()
{
   PTreeNodeList *newlist;
   PTreeNode *tmp;

   if (m_stocker == NULL || m_stocker->current == m_stocker->size) {
      newlist = (PTreeNodeList *) malloc(sizeof(PTreeNodeList));
      newlist->size = 200;
      newlist->list = (PTreeNode *) malloc(sizeof(PTreeNode) * newlist->size);
      newlist->current = 0;
      newlist->next = m_stocker;
      m_stocker = newlist;
   }
   tmp = &(m_stocker->list[m_stocker->current]);
   m_stocker->current++;

   tmp->left0 = NULL;
   tmp->right1 = NULL;

   return tmp;
}

/* PTree::initialize: initialize PTree */
void PTree::initialize()
{
   m_stocker = NULL;
   m_root = NULL;
}

/* PTree::clear: free PTree */
void PTree::clear()
{
   PTreeNodeList *tmp1, *tmp2;

   tmp1 = m_stocker;
   while (tmp1) {
      tmp2 = tmp1->next;
      free(tmp1->list);
      free(tmp1);
      tmp1 = tmp2;
   }
   initialize();
}

/* PTree::PTree: constructor */
PTree::PTree()
{
   initialize();
}

/* PTree::PTree: destructor */
PTree::~PTree()
{
   clear();
}

/* PTree::release: free PTree */
void PTree::release()
{
   clear();
}

/* PTree::add: add an entry to the tree */
void PTree::add(const char *str, void *data, const char *matchstr)
{
   int slen, bitloc;
   PTreeNode **p;
   PTreeNode *newleaf, *newbranch, *node;

   if (m_root == NULL) {
      m_root = newNode();
      m_root->value.data = data;
   } else {
      slen = strlen(str);
      bitloc = getDiffPoint(str, matchstr);

      p = &m_root;
      node = *p;
      while (node->value.thres_bit <= bitloc && (node->left0 != NULL || node->right1 != NULL)) {
         if (testBit(str, slen, node->value.thres_bit) != 0)
            p = &(node->right1);
         else
            p = &(node->left0);
         node = *p;
      }

      /* insert between [parent] and [node] */
      newleaf = newNode();
      newleaf->value.data = data;
      newbranch = newNode();
      newbranch->value.thres_bit = bitloc;
      *p = newbranch;
      if (testBit(str, slen, bitloc) == 0) {
         newbranch->left0 = newleaf;
         newbranch->right1 = node;
      } else {
         newbranch->left0 = node;
         newbranch->right1 = newleaf;
      }
   }
}

/* PTree::findNearest: return the nearest entry */
void * PTree::findNearest(const char *str)
{
   PTreeNode *n;
   PTreeNode *branch;
   int maxbitplace;

   if (m_root == NULL) return NULL;

   n = m_root;
   branch = NULL;
   maxbitplace = strlen(str) * 8 + 8;
   while (n->left0 != NULL || n->right1 != NULL) {
      branch = n;
      if (testBitMax(str, n->value.thres_bit, maxbitplace) != 0)
         n = n->right1;
      else
         n = n->left0;
   }
   return n->value.data;
}
