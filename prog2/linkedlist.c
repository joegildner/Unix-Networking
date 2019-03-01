#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

 typedef struct node {
    char* word;
    struct node* next;
 } node;


 bool contains(node* head, char* thisWord) {
    bool contains = false;
    node* current = head;
    while (current != NULL) {
       if(!strcmp(current->word,thisWord)) contains = true;
       current = current->next;
    }
    return contains;
 }

 void push(node* head, char* word) {
    node* current = head;

    if(head == NULL){
      head = malloc(sizeof(node));
      head->word = "first";
      current->next = NULL;
      return;
    }

    while (current->next != NULL) {
      current = current->next;
    }

    current->next = malloc(sizeof(node));
    current->next->word = word;
    current->next->next = NULL;
 }


 int main(){
     node* n;
     push(n, "hello");
     push(n, "world");
     printf("%d\n",contains(n,"world"));
     printf("%d\n",contains(n,"first"));
  }
