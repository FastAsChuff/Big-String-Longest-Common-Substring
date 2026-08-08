//======================================================================================================================================
// maximalcommonsubstrings.c: Find the Maximal Common Substrings of length >= M.
//======================================================================================================================================
// A Maximal Common Substring between two strings s1 and s2 is a common substring which extended at either end with contiguous characters from s2 no longer is a common substring. This program finds all maximal common substring(s) of length >= M_i between the first input text file and other text files given in the argument list. Duplicate Maximal Common Substrings are suppressed from the output for brevity.
// Typical RAM requirement approx. 80 * sizeof(filename1) + max_{i=2 to n} sizeof(filenamei).
// Author: Simon Goater May 2026.
// Usage:- ./maximalcommonsubstrings.bin C filename1 filename2 M_2 ... filenamen M_n
//
// COPYRIGHT NOTICE: Copying and distributing without modification, but with conspicuous attribution for any legal purpose is permitted.
//======================================================================================================================================

#include <math.h>
#include <openssl/evp.h> //sudo apt-get install libssl-dev
#include "/home/simon/20260505codedump/mylcsubstringfns.c"

#define OSSL_HASH_DIGEST_NAME "SHA256"
#define OSSL_HASH_DIGEST_LEN 32
#define HASH_TREE_DIGEST_LEN OSSL_HASH_DIGEST_LEN
#include "/home/simon/hashtree.c"

#define SUFFIXTREETEXTIFYBINARYFILES false
#define SUFFIXTREEINDEXINTEGERFORMATTYPE "%lu"

// gcc maximalcommonsubstrings.c -o maximalcommonsubstrings.bin -O3 -Wall -mssse3 -lm

typedef struct {
  uint64_t count; // Number of Maximal Common Substrings.
  uint64_t arraysize; 
  uint64_t M;
  double C;
  double score;
  SUFFIXTREEINDEXINTEGERTYPE *array; // 
} mcs_s;

void addmcss(mcs_s *mcs, uint64_t offset, SUFFIXTREEINDEXINTEGERTYPE substringlen) {
  if (2*mcs->count == mcs->arraysize) {
    if (mcs->arraysize == 0) {
      mcs->array = aligned_alloc(alignof(SUFFIXTREEINDEXINTEGERTYPE), sizeof(SUFFIXTREEINDEXINTEGERTYPE)*50);
      assert(mcs->array);
      mcs->arraysize = 50;
    } else {
      mcs->arraysize *= 2;
      assert((mcs->array = mysuffixtreerealloc(alignof(SUFFIXTREEINDEXINTEGERTYPE), mcs->array, sizeof(SUFFIXTREEINDEXINTEGERTYPE)*mcs->arraysize)));
    }
  }
  mcs->array[2*mcs->count] = offset;
  mcs->array[2*mcs->count + 1] = substringlen;
  mcs->count++;
}
  

void printmcss(mcs_s *mcs, SUFFIXTREEALPHABETTYPE *cmptext, uint64_t outputsubstringmaxlen) {
  mcs->score = 0.0;
  for (uint64_t i=0; i<mcs->count; i++) {
    mcs->score += pow(mcs->array[2*i + 1], mcs->C);
    if (mcs->array[2*i + 1] <= outputsubstringmaxlen) {
      char temp = cmptext[mcs->array[2*i] + mcs->array[2*i + 1]];
      cmptext[mcs->array[2*i] + mcs->array[2*i + 1]] = 0;
      printf("%lu. (" SUFFIXTREEINDEXINTEGERFORMATTYPE ", " SUFFIXTREEINDEXINTEGERFORMATTYPE ") '%s'\n", 1+i, mcs->array[2*i], mcs->array[2*i + 1], cmptext + mcs->array[2*i]);
      cmptext[mcs->array[2*i] + mcs->array[2*i + 1]] = temp;
    } else {
      char temp = cmptext[mcs->array[2*i] + outputsubstringmaxlen];
      cmptext[mcs->array[2*i] + outputsubstringmaxlen] = 0;
      printf("%lu. (" SUFFIXTREEINDEXINTEGERFORMATTYPE ", " SUFFIXTREEINDEXINTEGERFORMATTYPE ") '%s...'\n", 1+i, mcs->array[2*i], mcs->array[2*i + 1], cmptext + mcs->array[2*i]);
      cmptext[mcs->array[2*i] + outputsubstringmaxlen] = temp;
    }
  }
  printf("***** Matching Score = %g (%lu substrings)\n", mcs->score, mcs->count);
  fflush(stdout);
}

void gethash(char *text, uint64_t textlen, uint8_t *hash) {  
  EVP_MD_CTX *mdctx = EVP_MD_CTX_new();  
  const EVP_MD *md = EVP_get_digestbyname(OSSL_HASH_DIGEST_NAME);
  uint32_t md_len = 0;
  EVP_DigestInit_ex(mdctx, md, NULL);
  EVP_DigestUpdate(mdctx, text, textlen);
  EVP_DigestFinal_ex(mdctx, hash, &md_len);
  EVP_MD_CTX_free(mdctx);
}

void getmcss(mcs_s *mcs, mysuffixtree_s *mysuffixtree, SUFFIXTREEALPHABETTYPE *cmptext) {
  SUFFIXTREEINDEXINTEGERTYPE cmptextlen = strlen(cmptext);
  uint64_t lastmatch = (uint64_t)-1;
  hashtree_s hashtree;
  hashtree_new(&hashtree);
  uint8_t hash[HASH_TREE_DIGEST_LEN];
  for (uint64_t i=0; i<cmptextlen; i++) {
    SUFFIXTREEINDEXINTEGERTYPE substringlen = matchmysuffixtreestring(mysuffixtree, i+cmptext, cmptextlen-i);
    if ((substringlen >= mcs->M) && ((lastmatch == (uint64_t)-1) || (substringlen >= lastmatch))) {
      gethash(i+cmptext, substringlen, hash);
      if (!hashtree_select(&hashtree, hash)) {
        addmcss(mcs, i, substringlen);
        hashtree_insert(&hashtree, hash);
      }
      lastmatch = 1+substringlen;
    } 
    if (lastmatch < (uint64_t)-1) {
      if (i + lastmatch >= cmptextlen) break;
      lastmatch--;
    }
  }
  hashtree_free(hashtree.root);
}

char *filegetcontents(char *filename, uint64_t *filesizeret) {
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    printf("Could not open %s for reading.\n", filename);
    return NULL;
  }
  uint64_t filesize = 0;
  uint64_t chunksize = 0;
  char buffer[10000] = {0};
  uint32_t buffersize = sizeof(buffer);
  while ((chunksize = fread(buffer, 1, buffersize, fp))) filesize += chunksize;
  char *output = malloc(1+filesize);
  assert(output);
  fseek(fp, 0, SEEK_SET);
  chunksize = fread(output, 1, filesize, fp);
  if (chunksize != filesize) {
    fclose(fp);
    free(output);
    printf("Error reading %s.\n", filename);
    return NULL;
  }
  fclose(fp);
  output[filesize] = 0;
  *filesizeret = filesize;
  return output;
}

char *filegettext(char *filename, _Bool textualize) {
  uint64_t filesize = 0;
  char *contents = filegetcontents(filename, &filesize);
  if (contents == NULL) return NULL;
  for (uint64_t i=0; i<filesize; i++) {
    if (contents[i] == 0) {
      if (textualize) {
        contents[i] = ' ';
      } else {
        printf("File %s is not a valid 1 byte per character text file.\n", filename);
        free(contents);
        return NULL;
      }
    }
  }
  return contents;
}

int main(int argc, char* argv[]) {
  /*
  hashtree_s hashtree;
  hashtree_new(&hashtree);
  uint8_t hash[HASH_TREE_DIGEST_LEN];
  char *sometext = "Hi There!";
  gethash(sometext, strlen(sometext), hash);
  assert(!hashtree_select(&hashtree, hash));
  hashtree_insert(&hashtree, hash);
  assert(hashtree_select(&hashtree, hash));
  char *sometext2 = "Hello There!";
  gethash(sometext, strlen(sometext2), hash);
  assert(!hashtree_select(&hashtree, hash));
  hashtree_insert(&hashtree, hash);
  assert(hashtree_select(&hashtree, hash));
  hashtree_insert(&hashtree, hash);
  exit(0);
  */
  if ((argc < 5) || (argc & 0x1u)) {
    printf("A Maximal Common Substring between two strings s1 and s2 is a common substring which extended at either end with contiguous characters from s2 no longer is a common substring. This program finds all maximal common substring(s) of length >= M_i between the first input text file and other text files given in the argument list. Duplicate Maximal Common Substrings are suppressed from the output for brevity.\nA Matching Score is given for each file comparison for i=2 to n, as sum_{j = 1 to m_i} L_ij^C where L_ij are the maximal common substring lengths between filename1 and filenamei of length >= M_i.\nOutput for i = 2 to n is m_i tuples (b_ij, c_ij) '[substring_ij]' where b_ij is the file index offset of the substring in filenamei, and c_ij is the substring length, followed by the Matching Score of the file.\nTypical RAM requirement approx. 80 * sizeof(filename1) + max_{i=2 to n} sizeof(filenamei).\nAuthor: Simon Goater May 2026.\nUsage:- %s outputsubstringmaxlen C filename1 filename2 M_2 ... filenamen M_n\n1 <= C, M_i\n", argv[0]);
    exit(0);
  }
  mcs_s mcs;
  mcs.array = NULL;
  mcs.score = 0.0;
  mcs.arraysize = 0;
  mcs.count = 0;
  uint64_t outputsubstringmaxlen = atol(argv[1]);
  assert((SUFFIXTREEINDEXINTEGERTYPE)-1 >= outputsubstringmaxlen);
  mcs.C = atof(argv[2]);
  assert(mcs.C >= 1.0);
  char *filename1 = argv[3];
  SUFFIXTREEALPHABETTYPE *text = filegettext(filename1, SUFFIXTREETEXTIFYBINARYFILES);
  assert(text);
  uint64_t starttimems = mylcsubstring_gettimems();
  mysuffixtree_s mysuffixtree;
  newmysuffixtree(&mysuffixtree, text);
  uint64_t endtimems = mylcsubstring_gettimems();
  printf("Suffix tree of %s built in %lu ms.\n", filename1, endtimems - starttimems);
  uint32_t filenum = 2;
  while (argc > 2*filenum+1) {
    char *filename2 = argv[2*filenum];
    SUFFIXTREEALPHABETTYPE *cmptext = filegettext(filename2, SUFFIXTREETEXTIFYBINARYFILES);
    if (cmptext) {  
      mcs.M = atol(argv[2*filenum + 1]);
      printf("***** Comparing %s and %s with M = %lu.\n", filename1, filename2, mcs.M);
      starttimems = mylcsubstring_gettimems();
      if (mcs.M >= 1) {
        getmcss(&mcs, &mysuffixtree, cmptext);
        if (outputsubstringmaxlen) {
          printmcss(&mcs, cmptext, outputsubstringmaxlen);
        } else {
          printmcss(&mcs, cmptext, (SUFFIXTREEINDEXINTEGERTYPE)-1);
        }
        free(mcs.array);
        mcs.array = NULL;
        mcs.score = 0.0;
        mcs.arraysize = 0;
        mcs.count = 0;
      } else {
        printf("Invalid zero M value with %s.\n", filename2);
      }
      endtimems = mylcsubstring_gettimems();
      printf("%lu ms.\n", endtimems - starttimems);
      free(cmptext);
    }
    filenum++;
  }
  freemysuffixtree(mysuffixtree.root);
  free(text);
}
// Make raw hex file on Linux
// xxd -plain somefilename | tr -d '\n' >somefilename.hex
