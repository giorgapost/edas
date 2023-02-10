/***************************************************************************//**
 * @file app_stack.c
 * @brief Implementation file of a stack with integer elements.
 * @author Georgios Apostolakis
 ******************************************************************************/

#include "app_stack.h"
#include "app_log.h"

///The space where the objects of the stack will be stored.
static int stack[STACK_MAXSIZE];

///This variable points at the top of the stack.
static int top = -1;

int peek() {
	if(!isempty())
		return stack[top];
	app_log_error("Error. The stack is empty.\n");
	return -1;
}

int pop() {
   int data;

   if(!isempty()) {
      data = stack[top];
      top = top - 1;
      return data;
   }
   app_log_error("Error. The stack is empty.\n");
   return -1;
}

void push(int data) {
   if(!isfull()) {
      top = top + 1;
      stack[top] = data;
   }
   else
	   app_log_error("Could not insert data because stack is full. Increase the capacity in 'stack.h' file and the problem will be fixed.\n");
}

void clear(){
	top = -1;
}

bool isempty() {
   if(top == -1)
      return true;
   else
      return false;
}

bool isfull() {
   if(top == STACK_MAXSIZE)
      return true;
   else
      return false;
}
