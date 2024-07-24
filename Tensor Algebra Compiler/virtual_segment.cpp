//
//  virtual_segment.cpp
//  Tensor Algebra Compiler
//
//

#include <stdio.h>

enum VirtualSegment {
    LOCAL = '%',
    STATIC = '@',
    ARGUMENT = '_',
    CONSTANT = '$',
    THIS = '#',
    THAT = '&',
    POINTER = '*',
    TEMP = '!'
};
