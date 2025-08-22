#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

#define MAX_COMMAND_LENGTH 256
#define HISTORY_SIZE 10
#define command "mini-shell>"
#define MAX_SIZE = 100
// a simple stack implementation for storing command history
typedef struct{
    int items[MAX_SIZE];
    int top;


}Stack;

void initStack(Stack *s){
    s->top = -1; // -1 indicates an empty stack
}
void isFull(Stack *s){
    return s->top == MAX_SIZE - 1;
}


int main(){
    printf("Welcome to the Text File Analyzer!\n");
    // there is going to be an input loop that will continue until the user decides to exit
    // it'll store all the input in a dynamically allocated array of strings
    //np strcpy(), no strlen() they should be reimplemented
    //Circular buffer will be used to store the input
    // it will keep memory usage constant while allowing for dynamic input and keeping track of input history



    return 0;
}
