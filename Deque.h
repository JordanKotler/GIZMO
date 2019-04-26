/*
 * Deque.h
 *
 * Implementation of a generic, dynamic (array based) double-ended queue.
 * 
 * Author: Benedict Greenberg (https://github.com/nebbles)
 * Date:   7 February 2019
 * 
 * Based on the work of https://playground.arduino.cc/Code/QueueArray
 *
 * ╔══════════════════════════════════════════════════════════════════════╗
 * ║ Copyright (C) 2019  Benedict Greenberg (https://github.com/nebbles)  ║
 * ║                                                                      ║
 * ║ This program is free software: you can redistribute it and/or modify ║
 * ║ it under the terms of the GNU General Public License as published by ║
 * ║ the Free Software Foundation, either version 3 of the License, or    ║
 * ║ (at your option) any later version.                                  ║
 * ║                                                                      ║
 * ║ This program is distributed in the hope that it will be useful,      ║
 * ║ but WITHOUT ANY WARRANTY; without even the implied warranty of       ║
 * ║ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         ║
 * ║ GNU General Public License for more details.                         ║
 * ║                                                                      ║
 * ║ You should have received a copy of the GNU General Public License    ║
 * ║ along with this program. If not, see <http://www.gnu.org/licenses/>. ║
 * ╚══════════════════════════════════════════════════════════════════════╝
 * 
 * Version History
 * ---------------
 *
 * 2019-02-07  Benedict Greenberg <bsdgreenberg@gmail.com>
 *   - Initial release
 *
 */

#ifndef Deque_h
#define Deque_h

template <typename T>
class Deque
{
public:
  Deque(int initialSize = 2, int limit = 0);     // constructor
  ~Deque();                                      // destructor
  bool pushHead(const T item);                   // adds item to head/front of deque, returns True if successful
  bool pushTail(const T item);                   // adds item to tail/back of deque, returns True if successful
  T popHead();                                   // removes item from head/front of deque
  T popTail();                                   // removes item to tail/back of deque
  T peekHead() const;                            // returns item from head/front of deque without removing
  T peekTail() const;                            // returns item from tail/back of deque without removing
  bool isEmpty() const;                          // checks if deque is empty
  bool isFull() const;                           // checks if deque is full
  int count() const;                             // returns the number of items in the deque
  bool isLimited() const;                        // returns True if deque has user-defined limit
  bool setLimit(int newLimit);                   // limits size, 0 disables limit, returns True if successful
  bool resize(int newSize);                      // resize the size of the deque, return True if successful
  T &operator[](int index);                      // returns the index of deque, 0 being head/front
  void setFaultHandler(void (*f)(const char *)); // sets user-defined function which is called when error occurs

private:
  T *contents;                           // the array of the deque
  int size;                              // the size of the deque
  int items;                             // number of items in deque
  int maxItems;                          // total number of items allowed
  int head;                              // the head/front of the deque
  int tail;                              // the tail/back of the deque
  void (*faultHandler)(const char *);    // user-defined error function
  void error(const char *message) const; // error report method in case of error
};

template <typename T>
Deque<T>::Deque(int initialSize, int limit)
{
  if ((initialSize > limit) && (limit > 0))
  {
    initialSize = limit;
  }

  size = 0;
  items = 0;
  head = 0;
  tail = 0;
  faultHandler = NULL;
  maxItems = limit;

  // allocate enough memory for the array.
  contents = (T *)malloc(sizeof(T) * initialSize);

  // if there is a memory allocation error.
  if (contents == NULL)
    error("Error: insufficient memory to initialize Deque.");

  size = initialSize;
}

template <typename T>
Deque<T>::~Deque()
{
  free(contents); // deallocate the array of the queue.
  contents = NULL;
  faultHandler = NULL;
  size = 0;
  items = 0;
  head = 0;
  tail = 0;
}

template <typename T>
bool Deque<T>::pushHead(const T item)
{
  if (isFull())
  {
    bool ok = resize(size * 2);
    if (!ok)
      return false;
  }
  int head = ((head - 1) + size) % size; // reverse wrap around i.e. counterclockwise
  contents[head] = item;                 // store item
  items++;                               // add to total items stored
  return true;
}

template <typename T>
bool Deque<T>::pushTail(const T item)
{
  if (isFull())
  {
    bool ok = resize(size * 2);
    if (!ok)
      return false;
  }
  contents[tail] = item;    // store item
  tail = (tail + 1) % size; // increment tail with wrap around
  items++;                  // add to total items stored
  return true;
}

template <typename T>
T Deque<T>::popHead()
{
  if (isEmpty())
  {
    error("QUEUE: can't pop item from queue: queue is empty.");
    return T();
  }
  T item = contents[head];               // get item
  head = (head + 1) % size;              // increment head with wrap around
  items--;                               // decrease item count
  if (!isEmpty() && (items <= size / 4)) // shrink size of array if necessary
  {
    resize(size / 2);
  }
  return item; // return the item
}

template <typename T>
T Deque<T>::popTail()
{
  if (isEmpty())
  {
    error("QUEUE: can't pop item from queue: queue is empty.");
    return T();
  }
  int tail = ((tail - 1) + size) % size; // reverse wrap around i.e. counterclockwise
  T item = contents[tail];               // get item
  items--;                               // decrease item count
  if (!isEmpty() && (items <= size / 4)) // shrink size of array if necessary
  {
    resize(size / 2);
  }
  return item; // return the item
}

template <typename T>
T Deque<T>::peekHead() const
{
  if (isEmpty())
  {
    error("QUEUE: can't get the head item of queue: queue is empty.");
    return T();
  }
  return contents[head];
}

template <typename T>
T Deque<T>::peekTail() const
{
  if (isEmpty())
  {
    error("QUEUE: can't get the tail item of queue: queue is empty.");
    return T();
  }
  int t = ((tail - 1) + size) % size; // reverse wrap around
  return contents[t];
}

template <typename T>
bool Deque<T>::isEmpty() const
{
  return items == 0;
}

template <typename T>
bool Deque<T>::isFull() const
{
  return items == size;
}

template <typename T>
int Deque<T>::count() const
{
  return items;
}

template <typename T>
bool Deque<T>::isLimited() const
{
  return maxItems > 0;
}

template <typename T>
bool Deque<T>::setLimit(int newLimit)
{
  if (newLimit == 0) // disables the limit
  {
    maxItems = newLimit;
    return true;
  }
  if (newLimit < items) // is not allowed
  {
    error("Error: the new Deque limit is less than the number of items");
    return false;
  }
  int oldLimit = maxItems; // store old limit temporarily
  maxItems = newLimit;     // apply new limit
  if (newLimit < size)     // reduce memory footprint
  {
    bool ok = resize(newLimit);
    if (!ok)
      maxItems = oldLimit; // return to old limit since mem reduction failed
    return false;
  }
  return true;
}

template <typename T>
bool Deque<T>::resize(int newSize)
{
  if (newSize <= 0)
  {
    error("Error: new size for Deque array is not valid");
    return false;
  }

  if ((newSize > maxItems) && (maxItems > 0))
  {
    newSize = maxItems; // limit to maximum items allowed to save space
  }

  if (size == newSize)
  {
    error("Error: new size for Deque array is already at max limit");
    return false;
  }

  T *temp = (T *)malloc(sizeof(T) * newSize); // allocate enough memory for the temporary array

  if (temp == NULL) // if there is a memory allocation error
  {
    error("Error: insufficient memory to initialize temporary array for Deque");
    return false;
  }

  // copy the items from the old deque to the new one.
  for (int i = 0; i < items; i++)
  {
    temp[i] = contents[(head + i) % size];
  }

  free(contents);  // deallocate the old array
  contents = temp; // copy the pointer of the new array
  head = 0;        // set the head of the new array
  tail = items;    // set the tail of the new array
  size = newSize;  // set the new size of the array
  return true;     // resizing was successful
}

template <typename T>
T &Deque<T>::operator[](int index)
{
  if ((index < 0) || (index > items - 1)) // invalid indices
  {
    error("Error: invalid index for Deque");
    return contents[-1];
  }
  else
  {
    int i = (head + index) % size; // increment tail with wrap around
    return contents[i];
  }
}

template <typename T>
void Deque<T>::error(const char *message) const
{
  if (faultHandler)
  {
    faultHandler(message);
  }
}

template <typename T>
void Deque<T>::setFaultHandler(void (*f)(const char *))
{
  (faultHandler) = (f);
}

#endif