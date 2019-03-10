/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>

using namespace std;

class CirMgr;
//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//
class PatNode
{
  friend CirMgr;
public:
  PatNode(const vector<size_t>* const pat, unsigned id):_PatList(pat), _id(id) {};
  ~PatNode(){};
  bool operator== (const PatNode& h) const
  {

    bool eq = true;
    for (unsigned i = 0, n = _PatList->size(); i < n; ++i)
    {
// cout << "(*_PatList)[i] " << (*_PatList)[i] <<  "(*(h._PatList))[i]" << (*(h._PatList))[i] << endl;
      if ((*_PatList)[i] != (*(h._PatList))[i]) { eq = false; break; }
    }
    if (eq) return true;
    for (unsigned i = 0, n = _PatList->size(); i < n; ++i)
      if ((size_t)(~((*_PatList)[i])) != (*(h._PatList))[i]) return false;
// if (_id == 1465 && h.GateId()==0) cout << "1465 == 0"<< endl;
    return true;
  }

  const size_t operator() () const
  {
// cout << "size " << _PatList->size() << endl;
    unsigned count = 0;
    bool k = _PatList->front() % 2;
    size_t key = 0;
    for (unsigned i = 0, n = _PatList->size(); (i < 100) && (i < n); i += 10 )
      { key ^= (*_PatList)[i]; count++; }
// }
    if (count % 2 == 1)
      if (k) key = ~key;    //to make the invert case have the same key
    return key;
  }
  unsigned GateId() const { return _id; }

private:
  const vector<size_t> * const _PatList;
  const unsigned _id;
};


class StructNode
{
friend CirMgr;
public:
  StructNode(unsigned id ,unsigned a, unsigned b): _CirGateId(id),
    _key( a < b ? ( ((size_t)a << 32) | (size_t)b ) :  ( ((size_t)b << 32) | (size_t)a ) ) {};
  ~StructNode() {};
  const size_t& operator() () const { return _key; }
  bool operator== (const StructNode& h) const { return _key==h._key; }
  unsigned GateId() const { return _CirGateId; }

private:
  const unsigned _CirGateId;
  const size_t _key;
};

template <class Data>
class HashSet
{
public:
   HashSet(size_t b = 0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashSet() { reset(); }

   // TODO: implement the HashSet<Data>::iterator
   // o An iterator should be able to go through all the valid Data
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashSet<Data>;
   public:
      iterator (vector<Data>* b, size_t d, const HashSet<Data>* h)
         : _data(d), _buck(b), _hash(h), _LastBucket(_hash->_buckets + _hash->_numBuckets) {}
      const Data& operator * () const { return (*_buck)[_data]; }
      iterator& operator = (const iterator& i) { _data=i._data; _buck=i._buck; return *this; }
      iterator& operator ++ () 
      {
// cout << "_LastBucket " << _LastBucket - _hash->_buckets << endl;
         if (_data<(*_buck).size()-1) _data++;
         else
         {
             do
             {
               _buck++;
               if (_buck >= _LastBucket)
               {
                  *this=_hash->end();
// cout << "iterator at the end. _data= " << _data << " _buck at " << _buck-_hash->_buckets << endl;
                  return *this;
               }
             }
             while (_buck->empty());
             _data=0;
         }
// cout << "_data= " << _data << " _buck at " << _buck-_hash->_buckets << endl;
// cout << "end() at " << _hash->end()._data << " " << _hash->end()._buck-_hash->_buckets << endl;
         return *this;
      }
      bool operator != (const iterator& i) const
      { return !(_data==i._data && _buck==i._buck); }
   private:
      size_t _data;
      vector<Data>* _buck;
      const HashSet* const _hash;
      vector<Data>* _LastBucket;
   };

   void init(size_t b) { _numBuckets = b; _buckets = new vector<Data>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<Data>& operator [] (size_t i) { return _buckets[i]; }
   const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const 
   {
      size_t s=0;
      while (_buckets[s].empty())
      {
         s++;
         if (s > _numBuckets) return iterator(0,0,this);
      }
// cout << "begin at " << 0 << " " << s << endl;
      return iterator(_buckets+s,0,this);
   }
   // Pass the end
   iterator end() const
   {
      size_t s=_numBuckets-1;
      while (_buckets[s].empty())
      {
         if (s == 0) return iterator(0,0,this);
         s--;
      }
// cout << "end at " << _buckets[s].size() << " " << s << endl;
      return iterator(_buckets+s,_buckets[s].size(),this);
   }
   // return true if no valid data
   bool empty() const 
   {
      for (size_t i = 0; i < _numBuckets; ++i)
         if (!_buckets[i].empty()) return false;
      return true;
   }
   // number of valid data
   size_t size() const
   {
      size_t s=0;
      for (int i = 0; i < _numBuckets; ++i)
         s+=_buckets[i].size();
      return s;
   }

   // check if d is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const Data& d) const
   {
      vector<Data>& v=_buckets[bucketNum(d)];
      for (size_t i = 0, n = v.size(); i < n; ++i)
         if (d==v[i]) return true;
      return false;
   }

   // query if d is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(Data& d) const
   {
      vector<Data>& v=_buckets[bucketNum(d)];
      for (size_t i = 0, n = v.size(); i < n; ++i)
         if (d==v[i]) { d=v[i]; return true; }
      return false;
   }

   // update the entry in hash that is equal to d (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const Data& d)
   {
      vector<Data>& v=_buckets[bucketNum(d)];
      for (size_t i = 0, n = v.size(); i < n; ++i)
         if (d==v[i]) { v[i]=d; return true; }
      v.push_back(d);
      return false;
   }

   // return true if inserted successfully (i.e. d is not in the hash)
   // return false is d is already in the hash ==> will not insert
   bool insert(const Data& d)
   {
      vector<Data>& v=_buckets[bucketNum(d)];
      for (size_t i = 0, n = v.size(); i < n; ++i)
         if (d==v[i]) return false;  
      v.push_back(d);
      return true;
   }

   // return true if removed successfully (i.e. d is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const Data& d)
   {
      vector<Data>& v=_buckets[bucketNum(d)];
      for (size_t i = 0, n = v.size(); i < n; ++i)
         if (d==v[i]) { v[i]=v.back(); v.pop_back(); return true; }  
      return false;
   }

   size_t NumOfBuck() const { return _numBuckets; }

   Data* Find(const Data& d) const
   {
      vector<Data>& v=_buckets[bucketNum(d)];
      for (size_t i = 0, n = v.size(); i < n; ++i)
         if (d==v[i]) { return &v[i]; }
      v.push_back(d);
      return 0;
   }
private:
   // Do not add any extra data member
   size_t            _numBuckets;
   vector<Data>*     _buckets;

   size_t bucketNum(const Data& d) const {
      return (d() % _numBuckets); }
};

#endif // MY_HASH_SET_H
