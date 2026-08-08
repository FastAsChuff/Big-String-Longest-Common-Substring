#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

typedef struct hashtreenode_s {
  struct hashtreenode_s *hashtree_left;
  struct hashtreenode_s *hashtree_right;
  uint8_t hash[HASH_TREE_DIGEST_LEN];
} hashtreenode_s;

typedef struct {
  struct hashtreenode_s *root;  
} hashtree_s;

hashtreenode_s *hashtree_newnode(uint8_t *hash) {
  hashtreenode_s *newnode =  malloc(sizeof(hashtreenode_s));
  assert(newnode);
  newnode->hashtree_left = NULL;
  newnode->hashtree_right = NULL;
  memcpy(&newnode->hash, hash, HASH_TREE_DIGEST_LEN);
  return newnode;
}

void hashtree_new(hashtree_s *hashtree) {
  hashtreenode_s node;
  assert(HASH_TREE_DIGEST_LEN == sizeof(node.hash));
  hashtree->root = NULL;
}

int hashtree_cmp(uint8_t *hash1, uint8_t *hash2) {
  return memcmp(hash1, hash2, HASH_TREE_DIGEST_LEN);
}

_Bool hashtree_select(hashtree_s *hashtree, uint8_t *hash) {
  if (hashtree->root == NULL) return false;
  hashtreenode_s *thisnode = hashtree->root;
  int cmp = hashtree_cmp(thisnode->hash, hash);  
  while (true) {
    if (cmp == 0) return true;
    if (cmp > 0) {
      if (thisnode->hashtree_left) {
        thisnode = thisnode->hashtree_left;
      } else return false;
    } else {
      if (thisnode->hashtree_right) {
        thisnode = thisnode->hashtree_right;
      } else return false;
    }
    cmp = hashtree_cmp(thisnode->hash, hash);
  }
}

_Bool hashtree_insert(hashtree_s *hashtree, uint8_t *hash) {
  // Don't insert hashes that already exist.
  //printf("Inserting %02x%02x%02x%02x%02x%02x%02x%02x...\n", hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7]);
  if (hashtree->root == NULL) {
    hashtree->root = hashtree_newnode(hash);
    return true;
  }
  hashtreenode_s *thisnode = hashtree->root;
  int cmp = hashtree_cmp(thisnode->hash, hash);
  hashtreenode_s *newnode = hashtree_newnode(hash);
  while (true) {
    if (cmp == 0) {
      fprintf(stderr, "Illegal Duplicate Hashtree Insert!\n");
      free(newnode);
      return false;
    }     
    if (cmp > 0) {
      if (thisnode->hashtree_left) {
        thisnode = thisnode->hashtree_left;
      } else {
        thisnode->hashtree_left = newnode;
        return true;
      }
    } else {
      if (thisnode->hashtree_right) {
        thisnode = thisnode->hashtree_right;
      } else {
        thisnode->hashtree_right = newnode;
        return true;
      }
    }
    cmp = hashtree_cmp(thisnode->hash, hash);
  }
}

void hashtree_free(hashtreenode_s *thisnode) {
  if (NULL == thisnode) return;
  if (thisnode->hashtree_left) hashtree_free(thisnode->hashtree_left);
  if (thisnode->hashtree_right) hashtree_free(thisnode->hashtree_right);
  free(thisnode);
}
