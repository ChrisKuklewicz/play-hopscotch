//
//  main.cpp
//  Project T : try to make a hash map
//
//  Created by Chris Kuklewicz on 2012/05/25.
//  Copyright (c) 2012 Chris Kuklewicz. All rights reserved.
//

#include <iostream>
#include "city.h"
#include "hopscotch.h"

using std::cout;
using std::endl;

/* 
  29=j=foo
  26=a=i=n
   1=g=k
  13=b=E
  31=f=p=q=C
   0=c
  22=z=D
  16=m=B
   9=t=A
   3=u=y
 */
void test() {
  hopscotch::hopscotch<int> *h = new hopscotch::hopscotch<int>();
  for (char c='a'; c<='z'; ++c) {
    std::string k(1,c);
    h->add(k,(int)c);
    h->see();
  }
  for (char c='A'; c<='Z'; ++c) {
    std::string k(1,c);
    h->add(k,(int)c);
    h->see();
  }
  
  h->add(std::string("f"),1); 
  h->add(std::string("p"),1); 
  h->add(std::string("q"),1); 
  h->add(std::string("C"),1); 
  h->add(std::string("f"),2); 
  h->add(std::string("p"),2); 
  h->add(std::string("q"),2); 
  h->add(std::string("C"),2);
  
  h->erase(std::string("f"));
  h->erase(std::string("C"));
  
  h->add(std::string("f"),3); 
  h->add(std::string("p"),3); 
  h->add(std::string("q"),3); 
  h->add(std::string("C"),3); 
  delete h;
}

int main(int argc, const char * argv[]) {
  // insert code here...
  cout << "Hello, World!\n" << endl;
  
  test();
  return 0;
}

