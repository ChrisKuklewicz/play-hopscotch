//
//  hopscotch.h
//  Project T
//
//  This is written to help me recall how to write C++.
//  I have a chatty log to stderr so I can follow the internals.
//
//  The hash map below uses a simplied hopscotch algorithm.
//  http://en.wikipedia.org/wiki/Hopscotch_hashing
//  
//  This may also annoy kittens and crash your program if you use it.
//
//  Created by Chris Kuklewicz on 2012/05/27.
//  Copyright (c) 2012 Chris Kuklewicz. All rights reserved.
//

#ifndef Project_T_hopscotch_h
#define Project_T_hopscotch_h

#include <vector>
#include <list>
#include "boost/optional.hpp"
#include "boost/iterator/iterator_facade.hpp"
#include "boost/ptr_container/ptr_vector.hpp"
#include "city.h"

namespace hopscotch {
  
using std::cerr;
using std::endl;

// Key needs to mimic the std::string methods c_str() and size() for hashing.
// T needs to be a copyable value to get it into the map.  The map does
// not copy it afterwards. T may be a const type.
template <typename Key, typename T>
class hopscotch{

protected:  
  // This is the internal Node type.  Stores items of type T as a value.
  class Node {
  public:
    typedef typename boost::ptr_vector<boost::nullable<Node> >::size_type size_type;
    
    std::pair<const Key,T> kv;
    size_type home;
    
    Node(Key k, size_type h, T& i) : kv(make_pair(k,i)), home(h) {};
    ~Node() { cerr << "~Node " << kv.first << " : " << kv.second << endl; }
  };
  
  // I need secret access to ptr_vector to make it work for this
  class mypv : public boost::ptr_vector<boost::nullable<Node> > {
    public:
    typedef typename boost::ptr_vector<boost::nullable<Node> >::size_type size_type;
    typedef typename boost::ptr_vector<boost::nullable<Node> >::value_type value_type;
    typedef typename boost::ptr_vector<boost::nullable<Node> >::const_iterator const_iterator;    
    typedef typename boost::ptr_vector<boost::nullable<Node> >::iterator iterator;    
    typedef typename boost::ptr_vector<boost::nullable<Node> >::auto_type auto_type;    

    explicit mypv(size_type n) : boost::ptr_vector<boost::nullable<Node> >(n) {};

    // ptr_vector::c_array does not work with boost::nullable<> so this
    // mypv subclass has a subtlely different my_c_array that does work.
    // This is needed for resize.
    Node** const my_c_array()
    {
      if ( this->empty() )
        return 0;
      else
        return reinterpret_cast<Node** const>(&(this->begin().base()[0]));
    }

    // To avoid using my_c_array() in erase, I add reset_at.
    bool reset_at(size_type n) {
      if ( this->empty() || (this->size() <= n) )
        return false;
      Node* p = static_cast<Node*>(this->base()[n]);
      if (p == NULL) {
        return false;
      } else {
        this->base()[n] = NULL;
        delete p;
        return true;
      }
    }
    
    // To avoid using my_c_array() in hop, I add swap.
    bool swap(size_type a, size_type b) {
      if ( this->empty() || (this->size() <= a) || (this->size() <= b) )
        return false;
      void* val_a = this->base()[a];
      this->base()[a] = this->base()[b];
      this->base()[b] = val_a;
      return true;
    }
  };

  typedef typename mypv::iterator iterator_0;
  typedef typename mypv::const_iterator const_iterator_0;

public:
  
  typedef typename mypv::size_type size_type;
  typedef typename mypv::auto_type auto_type;
  typedef std::pair<const Key, T> key_value;
  typedef const std::pair<const Key, const T> const_key_value;
  
  struct hopscotch_dead : public std::exception {
    virtual const char* what() { return "hopscotch null pointer deferenced"; }
  };

  struct iterator_dead : public std::exception {
    virtual const char* what() { return "hopscotch iterator null pointer deferenced"; }
  };
  
  class const_iterator;  // for friend for casting iterator to const_iterator
  
  // Bi-directional iterator for the hopscotch hash map, using key_value
  class iterator : public boost::iterator_facade<iterator,
    key_value, boost::bidirectional_traversal_tag>
  {
  public:
    iterator() : v(0), x() {}
    iterator(mypv *v_in, iterator_0 x_in) : v(v_in), x(x_in) {
      if (v && x != v->end() && boost::is_null(x))
        this->operator++();
    }
  private:
    mypv *v;
    iterator_0 x;

    friend class boost::iterator_core_access;
    friend class const_iterator;  // use forward declaration, for casting iterator to const_iterator
    
    void increment() {
      if(v) { while (x != v->end()   && boost::is_null(++x)); }
    }
    void decrement() {
      if(v) { while (x != v->begin() && boost::is_null(--x)); }
    }
    bool equal(iterator const& rhs) const {
      return ((rhs.v == v) && (rhs.x == x));
    }
    key_value& dereference() const {
      return (x->kv);
    }
  };

  class const_iterator : public boost::iterator_facade<const_iterator,
    const_key_value, boost::bidirectional_traversal_tag>
  {
  public:
    const_iterator() : v(0), x() {}
    const_iterator(const iterator& from) : v(from.v), x(from.x) {}
    const_iterator(const const_iterator& from) : v(from.v), x(from.x) {}
    const_iterator(mypv *v_in, typename mypv::iterator x_in) : v(v_in), x(x_in) {
      if (v && x != v->end() && boost::is_null(x))
        this->operator++();
    }
    const_iterator(mypv *v_in, typename mypv::const_iterator x_in) : v(v_in), x(x_in) {
      if (v && x != v->end() && boost::is_null(x))
        this->operator++();
    }
  private:
    mypv *v;
    const_iterator_0 x;

    friend class boost::iterator_core_access;
    
    void increment() {
      if(v) { while (x != v->end()   && boost::is_null(++x)); }
    }
    void decrement() {
      if(v) { while (x != v->begin() && boost::is_null(--x)); }
    }
    bool equal(const_iterator const& rhs) const {
      return ((rhs.v == v) && (rhs.x == x));
    }
    const_key_value& dereference() const {
      // This cast changes "T" to "const T" which should be safe
      return reinterpret_cast<const std::pair<const Key, const T>& >(x->kv);
    }
  };
  
  typedef  boost::reverse_iterator< iterator >   reverse_iterator;  
  typedef  boost::reverse_iterator< const_iterator >   const_reverse_iterator;  
  
  // Iterator over non-empty values in this hash map that returns key-value pairs.
  iterator begin() { return iterator(&_store,_store.begin()); };
  iterator end()   { return iterator(&_store,_store.end()); };
  const_iterator begin() const { return const_iterator(&_store,_store.cbegin()); };
  const_iterator end() const   { return const_iterator(&_store,_store.cend()); };
  const_iterator cbegin() const { return const_iterator(&_store,_store.cbegin()); };
  const_iterator cend() const   { return const_iterator(&_store,_store.cend()); };
  reverse_iterator rbegin() { return reverse_iterator(this->end()); };
  reverse_iterator rend()   { return reverse_iterator(this->begin()); };
  const_reverse_iterator rbegin() const { return const_reverse_iterator(this->cend()); };
  const_reverse_iterator rend() const   { return const_reverse_iterator(this->cbegin()); };
  const_reverse_iterator crbegin() const { return const_reverse_iterator(this->cend()); };
  const_reverse_iterator crend() const   { return const_reverse_iterator(this->cbegin()); };

  // Very basic construction
  explicit hopscotch(size_type start_table_size = 32) : 
    _table_size(start_table_size), 
    _store(start_table_size) 
  {
    _store.resize(start_table_size, NULL);
    cerr << "_store.size() is " << _store.size() << endl;
  }
  // Default destruction, copy, and assignment should work
  
  size_type size() { return _size; }
  
  void add(const Key key, T item) {
    loc loc = locate(key);
    if (loc.second >= _table_size) {
      cerr << "need to resize! No hole at all for " << key << endl;
      resize();
      return add(key,item);
    }
    if (diff(loc.first, loc.second) >= HOP_LIM && _store.is_null(loc.second)) {
      cerr << "need to hop! hole too distant for " << key << endl;
      loc = hop(loc);
    }
    if (loc.second >= _table_size) {
      cerr << "need to resize! could not hop for " << key << endl;
      resize();
      return add(key,item);
    }
    if  (_store.is_null(loc.second)) {
      cerr << "inserting " << key << " : " << loc.first << " : " << item
          << " at " << loc.second << endl;
      _store.replace(loc.second, new Node(key, loc.first, item));
      _size += 1;
    } else {
      auto_type old = _store.replace(loc.second, new Node(key, loc.first, item));
      cerr << "replacing, delete " << old->kv.first << " : " << old->home << " : "
           << old->kv.second << endl;
      cerr << "           inserting " << key << " : " << loc.first << " : "
          << item << " at " << loc.second << endl;
    }
  }

  // The returned pointer is NULL if the key is not present.
  // A non-NULL returned pointer is owned by the hash map.
  // The pointer will become invalid if the key is overwritten,
  // the key is erased, or the hash map is deleted.
  T* lookup(const Key& key) {
    size_type n = locate(key).second;
    if (n < _table_size && !_store.is_null(n)) {
      return &(_store[n].kv.second);
    } else {
      return NULL;
    }
  }

  // The reference will become invalid if the key is overwritten,
  // the key is erased, or the hash map is deleted.
  T& operator[](const Key& key) {
    size_type n = locate(key).second;
    if (n < _table_size && !_store.is_null(n)) {
      return (_store[n].kv.second);
    } else {
      //throw hopscotch_dead();
      assert(false);
    }
  }

  // member tests whether the key is in the hash map.
  bool member(const Key& key) {
    size_type n = locate(key).second;
    return (n < _table_size && !_store.is_null(n));
  }
  
  // erase return true when the key was found and erased and false
  // when the key was not in the hash map.
  bool erase(const Key& key) {
    size_type n = locate(key).second;
    if (n < _table_size && !_store.is_null(n)) {
      cerr << "++ erasing " << key << " at " << n << endl;
      return _store.reset_at(n);
    } else {
      cerr << "++ not erasing " << key << endl;
      return false;
    }
  }

  // see is used for debugging and demonstration (with length 1 keys).
  void see() const {
    cerr << '{';
    for(const_iterator_0 it = _store.begin(); it != _store.end(); ++it) {
      if( !boost::is_null(it) ) {
        cerr << it->kv.first;
      } else {
        cerr << Key(".");
      }
    }
    cerr << "} {" << _size << ", " << _table_size << ", "
         << (double)_size/(double)_table_size << '}' << endl;
  }  

protected:
  
  // Double the size of the table, moving some old entries to the new space.
  // I claim that this cannot fail, no collisions arise.
  void resize() {
    const size_type old_table_size = _table_size;
    const size_type new_table_size = 2 * old_table_size;
    _store.resize(new_table_size, NULL);
    Node ** const a = _store.my_c_array();
    uint32_t full_hash;
    size_type off, index, target;
    Node **it = a;
    size_type probe = 0;
    for(; probe < old_table_size; ++it, ++probe) {
      if (*it) {
        off = diff((*it)->home, probe, old_table_size);

        full_hash = CityHash64((*it)->kv.first.c_str(), (*it)->kv.first.size());
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
          cerr << " happy " << probe << " : " << full_hash <<  ", " << off
               << ", " << target << endl;
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
  loc locate(const Key& key) const {
    const uint32_t full_hash = CityHash64(key.c_str(), key.size());
    const size_type index = full_hash % _table_size;
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
        const Node& n = _store[probe];
        if ((index == n.home) && (key == n.kv.first)) {
          cerr << "locate found old " << n.kv.first << " : " << n.home << " : "
               << n.kv.second << endl;
          return std::pair<size_type,size_type>(index,probe);
        }
      }
    }
    return std::pair<size_type,size_type>(index,hole);
  }

  // use diff(ideal bin, actual bin, old_table_size) to get the positive offset.
  size_type diff(size_type home, size_type probe, size_type table_size) const {
    return ((probe>=home) ? probe-home : (table_size+probe)-home);
  }

  // use diff(ideal bin, actual bin) to get the positive offset.
  size_type diff(size_type home, size_type probe) const {
    return ((probe>=home) ? probe-home : (_table_size+probe)-home);
  }
  
  // Move hole in loc.second starting more distant than HOP_LIM of loc.first
  // to be within HOP_LIM of loc.first.  This may fail in which case
  // the retured loc.second will be _table_size.
  loc hop(loc h) {
    size_type in_off = diff(h.first, h.second);
    assert(_store.is_null(h.second));
    assert(in_off >= HOP_LIM);
    do { // loop state h and in_off
      size_type off = in_off-(HOP_LIM-1);
      assert(1 <= off);
      size_type probe;
      size_type new_off;
      cerr << "searching for existing Node to Hop" << endl;
      for (; off < in_off; ++off) {
        probe = (h.first+off) % _table_size;
        assert(!_store.is_null(probe));
        {
          const Node& n = _store[probe];
          new_off = diff(n.home, h.second);
        }
        if (new_off < HOP_LIM) {
          {
            const Node& n = _store[probe];
            cerr << "found existing Node to Hop: " << n.kv.first << " : " <<  n.home
                 << " : " <<  n.kv.second << " moves " << probe << " -> " 
                 << h.second << endl;
          }
          _store.swap(probe, h.second);
          assert(!_store.is_null(h.second));
          assert(_store.is_null(probe));
          h.second = probe; // mutate loop state h
          break; // stop for-loop when (off < in_off)
        }
      }
      if (off >= in_off) {
        h.second = _table_size;
        break; // stop while-loop because hopping failed
      }
      in_off = diff(h.first, h.second); // mutate loop state in_off
    } while (in_off >= HOP_LIM);
    return h;
  }
  
  static const size_type HOP_LIM = 5; // 0 <= offset of entry from home < HOP_LIM
  static const size_type ADD_LIM = 8; // 0 <= offset of located hole < ADD_LIM
  size_type _table_size, _size;
  mypv _store;

  // Small alterantives to ptr_vector:
  // std::vector<std::list<key_value_type> > _store_traditional;
  // std::vector<boost::optional<key_value_type> > _store_optional;

};

}
#endif
