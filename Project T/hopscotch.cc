//
//  hopscotch.cc
//  Project T
//
//  Created by Chris Kuklewicz on 2012/05/27.
//  Copyright (c) 2012 Chris Kuklewicz. All rights reserved.
//

#include "hopscotch.h"

//namespace hopscotch {

// use diff(ideal bin, actual bin, old_table_size) to get the positive offset.
template<typename T>
typename hopscotch::hopscotch<T>::size_type hopscotch::hopscotch<T>::diff(size_type home, size_type probe, size_type table_size) {
  return ((probe>=home) ? probe-home : (table_size+probe)-home);
}

//}