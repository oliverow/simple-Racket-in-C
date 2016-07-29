#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "dictionary.h"
#include "rexp.h"
  
struct enode *string_to_enode(const char *s,int *pos) {
  if ((s[(*pos)] >= 48 && s[(*pos)] <= 57) ||
      ((s[(*pos)] == 45) && 
       (s[(*pos)+1] >= 48 && s[(*pos)+1] <= 57))){
    struct enode *node = malloc(sizeof(struct enode));
    node->type = 'n';
    node->number = 0;
    bool negative = false;
    if (s[(*pos)] == 45) {
      negative = true;
      (*pos)++;
    }
    while(s[(*pos)] >= 48 && s[(*pos)] <= 57) {
      node->number = (node->number * 10) + (s[(*pos)] - 48);
      (*pos)++;
    }
    if (negative) {
      node->number *= -1;
    }
    node->left = NULL;
    node->right = NULL;
    return node;
  }
  if (s[(*pos)] == 42 || s[(*pos)] == 43 || 
      s[(*pos)] == 45 || s[(*pos)] == 47) {
    struct enode *node = malloc(sizeof(struct enode));
    node->type = s[(*pos)];
    (*pos)++;
    struct enode *left = string_to_enode(s,pos);
    struct enode *right = string_to_enode(s,pos);
    node->left = left;
    node->right = right;
    return node;
  } 
  if ((s[(*pos)] >= 65 && s[(*pos)] <= 90) ||
      (s[(*pos)] >= 97 && s[(*pos)] <= 122)) {
    struct enode *node = malloc(sizeof(struct enode));
    node->type = 'v';
    int counter = 0;
    while(((s[(*pos)] >= 65 && s[(*pos)] <= 90) ||
           (s[(*pos)] >= 97 && s[(*pos)] <= 122)) &&
          (counter < 21)){
      node->id[counter] = s[(*pos)];
      (*pos)++;
      counter++;
    }
    node->id[counter] = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
  } 
  (*pos)++;
  return string_to_enode(s,pos);
}

struct rexp *string_to_rexp(const char *s) {
  assert(s);
  int pos = 0;
  int *p = &pos;
  struct rexp *tree = malloc(sizeof(struct rexp));
  tree->root = string_to_enode(s,p);
  return tree;
}

int enode_eval(struct enode *n, const struct dictionary *constants) {
  if (n->type == '*') {
    int left = enode_eval(n->left,constants);
    int right = enode_eval(n->right,constants);
    int result = left * right;
    return result;
  }
  if (n->type == '+') {
    int left = enode_eval(n->left,constants);
    int right = enode_eval(n->right,constants);
    int result = left + right;
    return result;
  }
  if (n->type == '-') {
    int left = enode_eval(n->left,constants);
    int right = enode_eval(n->right,constants);
    int result = left - right;
    return result;
  } 
  if (n->type == '/') {
    int left = enode_eval(n->left,constants);
    int right = enode_eval(n->right,constants);
    int result = left / right;
    return result;
  }
  if (n->type == 'n') {
    int result = n->number;
    return result;
  }
  if (n->type == 'v') {
    int result = dict_lookup(n->id,constants);
    return result;
  }
  return 0;
}
  
int rexp_eval(const struct rexp *r, const struct dictionary *constants) {
  struct enode *first = r->root;
  return enode_eval(first,constants);
}

void node_destroy(struct enode *n) {
  if (n && n->left) {
    node_destroy(n->left);
  }
  if (n && n->right) {
    node_destroy(n->right);
  }
  free(n);
}

void rexp_destroy(struct rexp *r) {
  node_destroy(r->root);
  free(r);
}


void add_constant(const char *s, struct dictionary *constants) {
  int counter = 0;
  int first_pos = strstr(s,"define") - s + 6;
  //printf("%d\n",first_pos);
  while(s[first_pos] == 32) {
    first_pos++;
  }
  //printf("%d\n",first_pos);
  char *var = malloc(21*sizeof(char));
  while(s[first_pos+counter] != 32 && counter < 21) {
    //printf("%d\n",first_pos+counter);
    var[counter] = s[first_pos+counter];
    counter++;
  }
  var[counter] = 0;
  int eval_pos = first_pos + counter;
  struct enode *to_be_eval = string_to_enode(s,&eval_pos);
  int value = enode_eval(to_be_eval,constants);
  dict_add(var,value,constants);
  free(var);
  node_destroy(to_be_eval);
}
