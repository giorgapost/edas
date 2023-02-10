/***************************************************************************//**
 * @file app_stack.h
 * @brief Header file for a stack with integer elements.
 * @author Georgios Apostolakis
 ******************************************************************************/
#ifndef APP_STACK_H
#define APP_STACK_H

#include "rail_types.h"

/// The maximum capacity of the stack.
#define STACK_MAXSIZE 20

/** Returns the top element of the stack (without popping it).
 *
 * @date 20/12/2022
 * @return An integer which is the top element of the stack.
 */
int peek();

/** Pops an element out of the stack's top.
 *
 * @date 20/12/2022
 * @return The (integer) element that was popped.
 */
int pop();

/** Pushes an element on the top of the stack.
 *
 * @date 20/12/2022
 * @param data The integer element to be pushed on the top of the stack.
 */
void push(int data);

/** Deletes all elements of the stack.
 *
 * @date 20/12/2022
 */
void clear();

/** Returns whether there exist any elements inside the stack.
 *
 * @date 20/12/2022
 * @return True if the stack is not empty, false otherwise.
 */
bool isempty();

/** Returns whether there exist space for more elements to be pushed inside the
 *  stack.
 *
 * @date 20/2/2022
 * @return True if the stack is full, false otherwise.
 */
bool isfull();

#endif  // APP_STACK_H
