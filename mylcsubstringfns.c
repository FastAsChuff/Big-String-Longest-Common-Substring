//======================================================================================================================================
// mylcsubstringfns.c: Build suffix tree and find the 'Longest Common Substring'.
// Author: Simon Goater April 2026.
// COPYRIGHT NOTICE: Copying and distributing without modification, but with conspicuous attribution for any legal purpose is permitted.
//======================================================================================================================================
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/time.h>


#ifndef alignof
  #define alignof _Alignof
#endif

#define SUFFIXTREESTRLENFN strlen
#define SUFFIXTREEALPHABETTYPE char
#define SUFFIXTREEPREXPONENTIALLIMIT 1000
#define SUFFIXTREEALPHABETIXINTEGERTYPE uint8_t
#define SUFFIXTREEALPHABETMAXSIZE 255
#define SUFFIXTREEINDEXINTEGERTYPE uint64_t
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define MIN(a,b) ((a) > (b) ? (b) : (a))

typedef struct mysuffixtreenode_s {
  struct mysuffixtreenode_s **children;
  SUFFIXTREEINDEXINTEGERTYPE startix, len;
  SUFFIXTREEALPHABETIXINTEGERTYPE numchildren;
} mysuffixtreenode_s;

typedef struct {
  SUFFIXTREEALPHABETTYPE *text;
  SUFFIXTREEINDEXINTEGERTYPE textlength;
  struct mysuffixtreenode_s *root;
} mysuffixtree_s;

uint64_t mylcsubstring_gettimems(void) {
  struct timeval thistime;
  gettimeofday(&thistime, NULL);
  return thistime.tv_sec*1000ULL + thistime.tv_usec/1000ULL;
}

struct mysuffixtreenode_s *newmysuffixtreenode(void) {
  struct mysuffixtreenode_s *newnode = aligned_alloc(alignof(mysuffixtreenode_s), sizeof(mysuffixtreenode_s));
  assert(newnode);
  newnode->children = NULL;
  newnode->numchildren = 0;
  newnode->startix = 0;
  newnode->len = 0;
  return newnode;
}

struct mysuffixtreenode_s *findmysuffixtreenodechild(mysuffixtree_s *mysuffixtree, struct mysuffixtreenode_s *node, SUFFIXTREEALPHABETTYPE character) {
  for (SUFFIXTREEALPHABETIXINTEGERTYPE i=0; i<node->numchildren; i++) {
    if (character == mysuffixtree->text[node->children[i]->startix]) return node->children[i];
  }
  return NULL;
}

void *mysuffixtreerealloc(uint64_t alignment, void *ptr, uint64_t newsize) {
  // Because there's no aligned_realloc.
  assert((alignof(max_align_t) % alignment) == 0);
  return realloc(ptr, newsize);
}

void mysuffixtreenewchild(struct mysuffixtreenode_s *thisnode, SUFFIXTREEINDEXINTEGERTYPE startix, SUFFIXTREEINDEXINTEGERTYPE len) {
  assert(thisnode != NULL);
  struct mysuffixtreenode_s *childnode = newmysuffixtreenode();
  assert(childnode);
  childnode->startix = startix;
  childnode->len = len;
  assert(thisnode->numchildren < SUFFIXTREEALPHABETMAXSIZE);
  if (thisnode->numchildren) {
    assert((thisnode->children = mysuffixtreerealloc(alignof(struct mysuffixtreenode_s*), thisnode->children, (1+thisnode->numchildren)*sizeof(struct mysuffixtreenode_s*))));
  } else {
    assert((thisnode->children = aligned_alloc(alignof(struct mysuffixtreenode_s*), sizeof(struct mysuffixtreenode_s*))));
  }
  thisnode->children[thisnode->numchildren] = childnode;
  thisnode->numchildren++;
}

void mysuffixtreetwonewchildren(struct mysuffixtreenode_s *thisnode, SUFFIXTREEINDEXINTEGERTYPE startix1, SUFFIXTREEINDEXINTEGERTYPE len1, SUFFIXTREEINDEXINTEGERTYPE startix2, SUFFIXTREEINDEXINTEGERTYPE len2) {
  assert(thisnode != NULL);
  struct mysuffixtreenode_s *childnode1 = newmysuffixtreenode();
  assert(childnode1);
  struct mysuffixtreenode_s *childnode2 = newmysuffixtreenode();
  assert(childnode2);
  childnode1->startix = startix1;
  childnode1->len = len1;
  childnode2->startix = startix2;
  childnode2->len = len2;
  assert(thisnode->numchildren < SUFFIXTREEALPHABETMAXSIZE-1);
  if (thisnode->numchildren) {
    assert((thisnode->children = mysuffixtreerealloc(alignof(struct mysuffixtreenode_s*), thisnode->children, (2+thisnode->numchildren)*sizeof(struct mysuffixtreenode_s*))));
  } else {
    assert((thisnode->children = aligned_alloc(alignof(struct mysuffixtreenode_s*), 2*sizeof(struct mysuffixtreenode_s*))));
  }
  thisnode->children[thisnode->numchildren++] = childnode1;
  thisnode->children[thisnode->numchildren++] = childnode2;
}

_Bool insertmysuffixtreenode(mysuffixtree_s *mysuffixtree, struct mysuffixtreenode_s *thisnode, SUFFIXTREEINDEXINTEGERTYPE ix) {
  if (mysuffixtree->root == NULL) return false;
  SUFFIXTREEINDEXINTEGERTYPE textlen = mysuffixtree->textlength;
  while (true) {
    assert(ix < textlen);
    assert(mysuffixtree->text[ix] == mysuffixtree->text[thisnode->startix]);
    SUFFIXTREEINDEXINTEGERTYPE prefixlen = 0;
    while ((mysuffixtree->text[ix + prefixlen] == mysuffixtree->text[thisnode->startix + prefixlen]) 
      && (prefixlen < thisnode->len) && (ix+prefixlen < textlen)) prefixlen++;
    if (ix+prefixlen >= textlen) {
      return true;
    } else {
      if (prefixlen >= thisnode->len) {
        struct mysuffixtreenode_s *child = findmysuffixtreenodechild(mysuffixtree, thisnode, mysuffixtree->text[ix + thisnode->len]);
        if (child) {
          ix += thisnode->len;
          thisnode = child;
        } else {
          mysuffixtreenewchild(thisnode, ix + thisnode->len, textlen-(ix + thisnode->len));
          return true;
        }
      } else {
        // Reduce thisnode->len to prefixlen
        struct mysuffixtreenode_s **thisnodechildren = thisnode->children;
        SUFFIXTREEALPHABETIXINTEGERTYPE thisnodenumchildren = thisnode->numchildren;
        thisnode->numchildren = 0;
        thisnode->children = NULL;
        mysuffixtreetwonewchildren(thisnode, thisnode->startix + prefixlen, thisnode->len - prefixlen, ix + prefixlen, textlen-(ix + prefixlen));
        thisnode->len = prefixlen;
        thisnode->children[0]->numchildren = thisnodenumchildren;
        thisnode->children[0]->children = thisnodechildren;
        return true;
      }
    }
  }
}

_Bool insertmysuffixtree(mysuffixtree_s *mysuffixtree, SUFFIXTREEINDEXINTEGERTYPE ix) {
  if (mysuffixtree->root == NULL) return false;
  struct mysuffixtreenode_s *thisnode = mysuffixtree->root;
  struct mysuffixtreenode_s *child = findmysuffixtreenodechild(mysuffixtree, thisnode, mysuffixtree->text[ix]);
  if (child) {
    return insertmysuffixtreenode(mysuffixtree, child, ix);
  } else {
    mysuffixtreenewchild(thisnode, ix, mysuffixtree->textlength-ix);
    return true;
  }
}

_Bool newmysuffixtree(mysuffixtree_s *mysuffixtree, SUFFIXTREEALPHABETTYPE *text) {
  // Suggested initialisation...
  // SUFFIXTREEALPHABETTYPE *text = "some text";
  // mysuffixtree_s mysuffixtree;
  // newmysuffixtree(&mysuffixtree, text);
  mysuffixtree->root = newmysuffixtreenode();
  mysuffixtree->text = text;
  mysuffixtree->textlength = SUFFIXTREESTRLENFN(text);
  for (SUFFIXTREEINDEXINTEGERTYPE i=0; i<mysuffixtree->textlength; i++) {
    insertmysuffixtree(mysuffixtree, i);
  }
  return true;
}

SUFFIXTREEINDEXINTEGERTYPE matchmysuffixtreestring(mysuffixtree_s *mysuffixtree, SUFFIXTREEALPHABETTYPE *cmptext, SUFFIXTREEINDEXINTEGERTYPE cmptextlen) {
  // Returns maximum length of prefix of cmptext which is a substring in mysuffixtree->text.
  if (cmptextlen == 0) return 0;
  SUFFIXTREEINDEXINTEGERTYPE cmpix = 0;
  struct mysuffixtreenode_s *thisnode = findmysuffixtreenodechild(mysuffixtree, mysuffixtree->root, cmptext[0]);
  if (thisnode == NULL) return 0;
  SUFFIXTREEINDEXINTEGERTYPE textlen = mysuffixtree->textlength;
  while (true) {
    assert(cmpix < cmptextlen);
    assert(cmptext[cmpix] == mysuffixtree->text[thisnode->startix]);
    SUFFIXTREEINDEXINTEGERTYPE prefixlen = 0;
    while ((cmptext[cmpix + prefixlen] == mysuffixtree->text[thisnode->startix + prefixlen]) 
      && (prefixlen < thisnode->len) && (cmpix+prefixlen < cmptextlen) && (thisnode->startix + prefixlen < textlen)) prefixlen++;
    if (thisnode->startix + prefixlen >= textlen) return cmpix + prefixlen;
    if (cmpix+prefixlen >= cmptextlen) {
      return cmptextlen;
    } else {
      if (prefixlen >= thisnode->len) {
        struct mysuffixtreenode_s *child = findmysuffixtreenodechild(mysuffixtree, thisnode, cmptext[cmpix + thisnode->len]);
        if (child) {
          cmpix += thisnode->len;
          thisnode = child;
        } else {
          return cmpix + thisnode->len;
        }
      } else {
        return cmpix + prefixlen;
      }
    }
  }
}

void freemysuffixtree(struct mysuffixtreenode_s *thisnode) {
  if (thisnode->numchildren) {
    for (SUFFIXTREEALPHABETIXINTEGERTYPE i=0; i<thisnode->numchildren; i++) {
      freemysuffixtree(thisnode->children[i]); // WARNING! - Highly Contrived Stackoverflow potential!
    }
    free(thisnode->children);
  }
  free(thisnode);
}

SUFFIXTREEINDEXINTEGERTYPE lcsubstringsexp(mysuffixtree_s *mysuffixtree, SUFFIXTREEALPHABETTYPE *cmptext, SUFFIXTREEINDEXINTEGERTYPE *lcsubstring) { 
  if (cmptext == NULL) return 0;
  SUFFIXTREEINDEXINTEGERTYPE cmptextlen = SUFFIXTREESTRLENFN(cmptext);
  if (cmptextlen == 0) return 0;
  SUFFIXTREEINDEXINTEGERTYPE match, longestmatch = 0;
  SUFFIXTREEINDEXINTEGERTYPE longestmatchcount = 0;
  SUFFIXTREEINDEXINTEGERTYPE idiff;
  SUFFIXTREEINDEXINTEGERTYPE exponentialmodeidiff, exponentialmodeiprev, exponentialmodeprevmatch;
  for (SUFFIXTREEINDEXINTEGERTYPE i=cmptextlen-1; ; ) {
    match = matchmysuffixtreestring(mysuffixtree, cmptext + i, cmptextlen - i);
    if (match > SUFFIXTREEPREXPONENTIALLIMIT) {
      exponentialmodeiprev = i;
      exponentialmodeprevmatch = match;
      exponentialmodeidiff = 1;
      while (match >= exponentialmodeprevmatch + exponentialmodeiprev - i) {
        exponentialmodeiprev = i;
        exponentialmodeprevmatch = match;
        if (i == 0) break;
        exponentialmodeidiff *= 2;
        if (i >= exponentialmodeidiff) {
          i -= exponentialmodeidiff;
        } else {
          i = 0;
        }
        match = matchmysuffixtreestring(mysuffixtree, cmptext + i, cmptextlen - i);
      }
      i = exponentialmodeiprev;
      match = exponentialmodeprevmatch;
    } 
    if (match > longestmatch) {
      longestmatch = match;
      lcsubstring[0] = i;
      lcsubstring[1] = i+match-1;
      longestmatchcount = 1;
    } else {
      if (match && (match == longestmatch)) longestmatchcount++;
    }
    idiff = (longestmatch > match ? longestmatch - match : 0);
    idiff = MAX(1, idiff);
    if (i < idiff) break;
    i -= idiff;
  }
  return longestmatchcount;
}

//======================================================================================================================================

