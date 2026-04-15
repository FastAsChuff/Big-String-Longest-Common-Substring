//======================================================================================================================================
// mylcsubstring2.c: Find the Longest Common Substring.
//======================================================================================================================================
// This program finds the longest substring(s) in the first input text file which is(are) common to other text files given in the argument list.
// Typical RAM requirement approx. 80 * sizeof(filename1) + max_{i=2 to n} sizeof(filenamei).
// Author: Simon Goater April 2026.
// Usage:- ./mylcsubstring2.bin filename1 filename2 ... filenamen
//
// COPYRIGHT NOTICE: Copying and distributing without modification, but with conspicuous attribution for any legal purpose is permitted.
//======================================================================================================================================

#include "/home/simon/mylcsubstringfns.c"

#define SUFFIXTREETEXTIFYBINARYFILES false
#define SUFFIXTREEPRINTSTRINGMAXSIZE 1000
#define SUFFIXTREEINDEXINTEGERFORMATTYPE "%lu"

// gcc mylcsubstring2.c -o mylcsubstring2.bin -O3 -Wall -mssse3

void printlcsubstring(SUFFIXTREEALPHABETTYPE *cmptext, SUFFIXTREEINDEXINTEGERTYPE *lcsubstring, SUFFIXTREEINDEXINTEGERTYPE numlcsubstrings, char *filename1, char *filename2, SUFFIXTREEINDEXINTEGERTYPE maxexampleprintlength) {
  if (numlcsubstrings == 0) {
    printf("There are 0 Longest Common Substrings in %s matching substrings in %s.\n", filename2, filename1);
    return;
  }
  if (numlcsubstrings > 1) {
    printf("There are " SUFFIXTREEINDEXINTEGERFORMATTYPE " Longest Common Substrings in %s matching substrings in %s.\nExample ", numlcsubstrings, filename2, filename1);
  }
  printf("Longest Common Substring is at [" SUFFIXTREEINDEXINTEGERFORMATTYPE ", " SUFFIXTREEINDEXINTEGERFORMATTYPE "] in %s\n", lcsubstring[0], lcsubstring[1], filename2);
  SUFFIXTREEINDEXINTEGERTYPE endprintix = 1+lcsubstring[1];
  char *cdots;
  if (endprintix > lcsubstring[0] + maxexampleprintlength) {
    endprintix = lcsubstring[0] + maxexampleprintlength;
    cdots = "...";
  } else {
    cdots = "";
  }
  SUFFIXTREEALPHABETTYPE temp = cmptext[endprintix];
  cmptext[endprintix] = 0;
  printf("\"%s%s\"\n", &cmptext[lcsubstring[0]], cdots);
  cmptext[endprintix] = temp;
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
  if (argc < 3) {
    printf("This program finds the longest substring(s) in the first input text file which is(are) common to other text files given in the argument list.\nTypical RAM requirement approx. 80 * sizeof(filename1) + max_{i=2 to n} sizeof(filenamei).\nAuthor: Simon Goater April 2026.\nUsage:- %s filename1 filename2 ... filenamen\n", argv[0]);
    exit(0);
  }
  char *filename1 = argv[1];
  SUFFIXTREEALPHABETTYPE *text = filegettext(filename1, SUFFIXTREETEXTIFYBINARYFILES);
  assert(text);
  uint64_t starttimems = mylcsubstring_gettimems();
  mysuffixtree_s mysuffixtree;
  newmysuffixtree(&mysuffixtree, text);
  uint64_t endtimems = mylcsubstring_gettimems();
  printf("Suffix tree of %s built in %lu ms.\n", filename1, endtimems - starttimems);
  SUFFIXTREEINDEXINTEGERTYPE lcsubstring[2] = {0};
  uint32_t filenum = 2;
  while (argc > filenum) {
    char *filename2 = argv[filenum];
    SUFFIXTREEALPHABETTYPE *cmptext = filegettext(filename2, SUFFIXTREETEXTIFYBINARYFILES);
    if (cmptext) {  
      starttimems = mylcsubstring_gettimems();
      SUFFIXTREEINDEXINTEGERTYPE numlcsubstrings = lcsubstringsexp(&mysuffixtree, cmptext, lcsubstring); 
      printlcsubstring(cmptext, lcsubstring, numlcsubstrings, filename1, filename2, SUFFIXTREEPRINTSTRINGMAXSIZE);
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
