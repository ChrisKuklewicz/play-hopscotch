//
//  hopscotch.h
//  Project T
//
//  Created by Chris Kuklewicz on 2012/05/27.
//  Copyright (c) 2012 Chris Kuklewicz. All rights reserved.
//

#ifndef Project_T_hopscotch_h
#define Project_T_hopscotch_h

#include <vector>
#include <list>
#include "boost/optional.hpp"
#include "boost/ptr_container/ptr_vector.hpp"
#include "city.h"

namespace hopscotch {
  
using std::cerr;
using std::endl;

template <typename T>
class hopscotch{

public:
  // This is the internal Node type.
  class Node {
  public:
    typedef typename boost::ptr_vector<boost::nullable<Node> >::size_type size_type;
    
    std::string key;
    size_type home;
    T item;
    
    Node(std::string k, size_type h, T& i) : key(k), home(h), item(i) {};
    ~Node() { cerr << "~Node " << key << " : " << item << endl; }
  };
  
protected:
  
  class mypv : public boost::ptr_vector<boost::nullable<Node> > {
    public:
    typedef typename boost::ptr_vector<boost::nullable<Node> >::size_type size_type;
    typedef typename boost::ptr_vector<boost::nullable<Node> >::value_type value_type;
    typedef typename boost::ptr_vector<boost::nullable<Node> >::iterator iterator;    
    typedef typename boost::ptr_vector<boost::nullable<Node> >::auto_type auto_type;    

    explicit mypv(size_type n) : boost::ptr_vector<boost::nullable<Node> >(n) {};

    // ptr_vector::c_array does not work with boost::nullable<> so this
    // mypv subclass has a subtlely different my_c_array that does work.
    value_type* my_c_array() // nothrow
    {
      if( this->empty() )
        return 0;
//      T** res = reinterpret_cast<T**>( &this->begin().base()[0] );      
      return reinterpret_cast<Node**>(&this->begin().base()[0]);
    }
  };
  
public:
  
  typedef typename mypv::size_type size_type;
  typedef typename mypv::auto_type auto_type;
  typedef typename mypv::iterator iterator;
  
  // Currently exposes an iterator to the internal Node type.
  // You must test with boost::is_null() before derefencing.
  iterator begin() {
    return _store.begin();
  };
  
  // Currently exposes an iterator to the internal Node type.
  iterator end() {
    return _store.end();
  };
  
    
  // Very basic construction
  explicit hopscotch(size_type start_table_size = 32) : _table_size(start_table_size), _store(start_table_size) {
    _store.resize(start_table_size, NULL);
    cerr << "_store.size() is " << _store.size() << endl;
  }
  // Default destruction
  ~hopscotch() {};
  
  void add(const std::string key, T item) {
    loc loc = locate(key);
    if (loc.second >= _table_size) {
      cerr << "need to resize! No hole at all for " << key << endl;
      resize();
      return add(key,item);
    }
    if (diff(loc.first, loc.second) > HOP_LIM && _store.is_null(loc.second)) {
      cerr << "need to hop! hole too distant for " << key << endl;
      loc = hop(loc);
    }
    if (loc.second >= _table_size) {
      cerr << "need to resize! could not hop for " << key << endl;
      resize();
      return add(key,item);
    }
    if  (_store.is_null(loc.second)) {
      cerr << "inserting " << key << " : " << loc.first << " : " << item << " at " << loc.second << endl;
      _store.replace(loc.second, new Node(key, loc.first, item));
      _size += 1;
    } else {
      auto_type old = _store.replace(loc.second, new Node(key, loc.first, item));
      cerr << "replacing, delete " << old->key << " : " << old->home << " : " << old->item << endl;
      cerr << "           inserting " << key << " : " << loc.first << " : " << item << " at " << loc.second << endl;
    }
  }

  // The returned pointer is NULL if the key is not present.
  // A non-NULL returned pointer is owned by the hash map.
  // The pointer will become invalid if the key is overwritten,
  // the key is erased, or the hash map is deleted.
  T* operator[](const std::string key) {
    loc h = locate(key);
    if (h.second >= _table_size || _store.is_null(h.second)) {
      return NULL;
    }
    return &(_store[h.second].item);
  }
  
  // member tests whether the key is in the hash map.
  bool member(const std::string key) {
    loc h = locate(key);
    return (h.second < _table_size && !_store.is_null(h.second));
  }
  
  // erase return true when the key was found and erased and false
  // when the key was not in the hash map.
  bool erase(const std::string key) {
    loc h = locate(key);
    if (h.second < _table_size && !_store.is_null(h.second)) {
      cerr << "++ erasing " << key << " at " << h.second << endl;
      Node *gone;
      {
        Node **a = _store.my_c_array();
        gone = a[h.second];
        a[h.second] = 0;
      }
      delete gone;
      return true;
    } else {
      return false;
    }
  }

  // see is used for debugging and demonstration (with length 1 keys).
  void see() {
    cerr << '{';
    for(iterator it = _store.begin(); it != _store.end(); ++it) {
      if( !boost::is_null(it) ) {
        cerr << it->key;
      } else {
        cerr << std::string(".");
      }
    }
    cerr << "} {" << _size << ", " << _table_size << ", " << (double)_size/(double)_table_size << '}' << endl;
  }  

protected:
  
  // Double the size of the table, moving some old entries to the new space.
  void resize() {
    const size_type old_table_size = _table_size;
    const size_type new_table_size = 2 * old_table_size;
    _store.resize(new_table_size, NULL);
    {
      Node **a = _store.my_c_array();
      Node **it = a;
      uint32_t full_hash;
      size_type probe, off, index, target;
      for(probe=0; probe < old_table_size; ++it, ++probe) {
        if (*it) {
          off = diff((*it)->home, probe, old_table_size);

          full_hash = CityHash64((*it)->key.c_str(), (*it)->key.size());
          index = full_hash % new_table_size;
          target = (index + off) % new_table_size;

          if (target != probe) {            
            cerr << "  moving " << probe << " -> " << target << endl;
            assert(a[target] == NULL);
            assert(a[probe] != NULL);
            a[target] = a[probe];
            a[probe] = NULL;
            assert(a[target] != NULL);
            assert(a[probe] == NULL);
            a[target]->home = index;
          } else {
            cerr << " happy " << probe << " : " << full_hash <<  ", " << off << ", " << target << endl;
            
          }
        }
      }
    }
    _table_size = new_table_size;
    see();
  }
  
  typedef std::pair<size_type,size_type> loc;

  // loc.first is the ideal (lowest) bucket index for the key
  // if the key is in the hash map then loc.second is index of its full bucket
  // if the key is not in the hash map then either
  //    1) loc.secord is the index of the nearest hole, up to ADD_LIM
  //    2) loc.second is _table_size and there is no hole up to ADD_LIM
  // Note that loc.second may wrap modulo _table_size so loc.second < loc.first,
  //   use diff(loc.first, loc.send) to get the positive offset.
  loc locate(const std::string key) {
    uint32_t full_hash = CityHash64(key.c_str(), key.size());
    size_type index = full_hash % _table_size;
    size_type hole = _table_size;
    size_type probe;
    const size_type end_off = (ADD_LIM <_table_size) ? ADD_LIM : _table_size;
    for (size_type off = 0; off < end_off; ++off) {
      probe = (index+off) % _table_size;
      if (_store.is_null(probe)) {
        if (hole >= _table_size) {
          hole = probe;
        }
      } else {
        Node& n = _store[probe];
        if ((index == n.home) && (key == n.key)) {
          cerr << "locate found old " << n.key << " : " << n.home << " : " << n.item << endl;
          return std::pair<size_type,size_type>(index,probe);
        }
      }
    }
    return std::pair<size_type,size_type>(index,hole);
  }
  
  // use diff(ideal bin, actual bin, old_table_size) to get the positive offset.
  size_type diff(size_type home, size_type probe, size_type table_size); /*{
    return ((probe>=home) ? probe-home : (table_size+probe)-home);
  }*/
  
  // use diff(ideal bin, actual bin) to get the positive offset.
  size_type diff(size_type home, size_type probe) {
    return ((probe>=home) ? probe-home : (_table_size+probe)-home);
  }
  
  // Move hole in loc.second to be in HOP_LIM of loc.first
  loc hop(loc h) {
    assert(_store.is_null(h.second));
    size_type in_off = diff(h.first, h.second);
    assert(in_off > HOP_LIM);
    do {
      size_type off = in_off-(HOP_LIM-1);
      assert(1 <= off);
      size_type probe;
      size_type new_off;
      cerr << "searching for existing Node to Hop" << endl;
      for (; off < in_off; ++off) {
        probe = (h.first+off) % _table_size;
        assert(!_store.is_null(probe));
        {
          Node& n = _store[probe];
          new_off = diff(n.home, h.second);
        }
        if (new_off < HOP_LIM) {
          {
            Node& n = _store[probe];
            cerr << "found existing Node to Hop: " << n.key << " : " <<  n.home << " : " <<  n.item << " moves " << probe << " -> " << h.second << endl;
          }
          {
            // Must do this out of scope of Node& above
            Node **a = _store.my_c_array();
            a[h.second] = a[probe];
            a[probe] = NULL;
          }
          assert(!_store.is_null(h.second));
          assert(_store.is_null(probe));
          h.second = probe;
          break; // stop for-loop when (off < in_off)
        }
      }
      if (off >= in_off) {
        h.second = _table_size;
        break; // stop while-loop because hopping failed
      }
      in_off = diff(h.first, h.second);
    } while (in_off > HOP_LIM);
    return h;
  }
  
  static const size_type HOP_LIM = 4;
  static const size_type ADD_LIM = 8;
  size_type _table_size, _size;
  mypv _store;

  // Small alterantives to ptr_vector:
  // std::vector<std::list<key_value_type> > _store_traditional;
  // std::vector<boost::optional<key_value_type> > _store_optional;

};

}
#endif
