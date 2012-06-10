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

using std::cerr;
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

typedef hopscotch::hopscotch<std::string,int> H;
void test() {
  H *h = new H();
  assert(h->begin() == h->end());
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
  h->erase(std::string("f")); // again
  
  h->add(std::string("f"),3); 
  h->add(std::string("p"),3); 
  h->add(std::string("q"),3); 
  h->add(std::string("C"),3);

  {
    H::iterator it = h->begin();
    const H::iterator end = h->end();
    for(int count=1; it != end; ++it, ++count) {
      cerr << count << " : " << (*it).first << " : " << (*it).second << endl;
      it->second = count;
    }
  }
  assert(h->lookup(std::string("not present")) == NULL);
  cerr << "Set first, Q, to -1" << endl;
  h->begin()->second = (-1);
  cerr << "Set last, N, to -2" << endl;
  (--(h->end()))->second = (-2);
  cerr << "Set \"a\" to 1000" << endl;
  (*h)[std::string("a")] = 1000;
  {
    H::const_iterator it = h->begin();
    const H::const_iterator end = h->end();
    for(int count=1; it != end; ++it, ++count) {
      cerr << count << " : " << (*it).first << " : " << (*it).second << endl;
    }
  }
  {
    H::const_reverse_iterator it = h->rbegin();  
    const H::const_reverse_iterator end = h->rend();
    for(int count=1; it != end; ++it, ++count) {
      cerr << count << " : " << (*it).first << " : " << (*it).second << endl;
    }
  }
  assert(!h->member(std::string("this key is not in the map")));
  cerr << "call delete on map" << endl;
  delete h;
  h=NULL;
}

int main(int argc, const char * argv[]) {
  // insert code here...
  cerr << "Hello, World!\n" << endl;
  
  test();
  return 0;
}

