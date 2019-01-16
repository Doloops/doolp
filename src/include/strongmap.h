#ifndef __STRONGMAP_H
#define __STRONGMAP_H

#include <map>
using namespace std;

// #include <glog.h>
#define __STRONGMAP_Bug(...) { fprintf ( stderr, "Bug : "); fprintf ( stderr, __VA_ARGS__); exit ( -1) ; }
#define __STRONGMAP_Log(...)

template<typename __key, typename __val, typename __less = less<__key> >
class strongmap
  {
    //  protected:
    private:
    typedef __key __key_type;
    typedef __val __val_type;
    typedef __less __less_type;
    typedef map<__key_type,__val_type,__less_type> __map;

    __map map1;
    public:
   
    strongmap() {}
    typedef typename __map::const_iterator citerator;
    typedef typename __map::iterator iterator;
    iterator begin () 
    { return map1.begin(); }
    iterator end ()
    { return map1.end(); }

    /*
      inline citerator begin () const
      { return map1.begin (); }
      inline citerator end () const
      { return map1.end (); }
    */
    inline __val& operator[] ( const __key &key )
    {
      __STRONGMAP_Bug ( "strongmap forbids using []\n" );
      return map1.end()->second;
    }
    inline __val get ( const __key &key ) const
    {
      citerator it = map1.find ( key );
      if ( it == map1.end () )
	{
	  return (__val) NULL;
	}
      return it->second;
    }
    inline bool has ( const __key &key ) const
    {
      citerator it = map1.find ( key );
      if ( it == map1.end () )
	{
	  return false;
	}
      return true;
    }
    inline bool put ( const __key &key, const __val &val )
    {
      if ( ((void*)get ( key )) != NULL )
	__STRONGMAP_Bug ( "Not a unique put, already has a value.\n" );
      map1.insert ( make_pair ( key, val ) );
      
      return true;
    }
    inline bool remove ( const __key &key )
    {
      iterator it = map1.find ( key );
      if ( it == map1.end () )
	{
	  __STRONGMAP_Bug ( "In remove : key not found.\n" );
	}
      map1.erase ( it );
      return true;
    }
    inline unsigned int count ( const __key &key )
    {
      return (unsigned int) map1.count ( key );
    }
    inline unsigned int size ( )
    {
      return (unsigned int ) map1.size ();
    }
  };

template<typename __key1, typename __key2, typename __less1, typename __less2>
  class strongdoublemapkey
{
 public:
  __key1 key1;
  __key2 key2;
  
  strongdoublemapkey ( __key1 _key1, __key2 _key2 )
    {
      key1 = _key1;
      key2 = _key2;
    }
  inline bool operator< ( const strongdoublemapkey & h2 ) const
    {
      if ( __less1() ( key1, h2.key1 ) )
	return true;
      else if ( __less1() ( h2.key1, key1 ) )
	return false;
      if ( __less2() ( key2,  h2.key2 ) )
	return true;
      return false;
    }
};

template<typename __key1, typename __key2, typename __val, typename __less1 = less<__key1>, typename __less2 = less<__key2> >
class strongdoublemap // : public strongmap<__key1, void *, __less1>
  {
    protected:
    typedef strongdoublemapkey<__key1,__key2, __less1, __less2> __doublekey;
    typedef map<__doublekey, __val, less<__doublekey> > __map1;
    
    __map1 map1;
    public:
    strongdoublemap() {}
    typedef typename __map1::const_iterator citerator;
    typedef typename __map1::iterator __iter;
    inline citerator begin () const
    { return map1.begin (); }
    inline citerator end () const
    { return map1.end (); }

    class iterator
    {
      friend class strongdoublemap<__key1,__key2,__val,__less1,__less2>;
      protected:
      __iter iter;
      public:
      __key1 first() { return iter->first.key1; }
      __key2 second() { return iter->first.key2; }
      __val third() { return iter->second; }
      bool operator==(const iterator& __x) const
      { return iter == __x.iter; }
      bool operator!=(const iterator& __x) const
      { return iter != __x.iter; }
      iterator& operator++()
      { iter++; return *this; }
      iterator operator++(int)
      { iterator __tmp = *this; iter++;
	return __tmp; }
    };
    protected:
    iterator __begin, __end;
    public:

    iterator begin () 
    { __begin.iter = map1.begin(); return __begin; }
    iterator end ()
    { __end.iter = map1.end(); return __end; }

    inline __val get ( const __key1& key1, const __key2& key2 ) const
    {
      __doublekey key ( key1, key2 );
      citerator it = map1.find ( key );
      if ( it == map1.end () )
	{
	  return (__val) NULL;
	}
      return it->second;
    }
    inline bool has ( const __key1& key1, const __key2& key2 ) const
    {
      __doublekey key ( key1, key2 );
      citerator it = map1.find ( key );
      if ( it == map1.end () )
	{
	  return false;
	}
      return true;
    }
    
    inline bool put ( const __key1& key1, const __key2& key2, const __val val ) 
    {
      if ( ((void*)get ( key1, key2 )) != NULL )
	__STRONGMAP_Bug ( "Not a unique put, already has a value.\n" );
      __doublekey key ( key1, key2 );
      map1.insert ( make_pair ( key, val ) );
      return true;
    }
    inline bool remove ( const __key1& key1, const __key2& key2 )
    {
      __doublekey key ( key1, key2 );
      map1.erase ( map1.find ( key ) );
      return true;
    }
    inline unsigned int count ( const __key1& key1, const __key2& key2 )
    {
      __doublekey key ( key1, key2 );
      return (unsigned int) map1.count ( key );
    }
    inline unsigned int size ( )
    {
      return (unsigned int ) map1.size ();
    }
    inline void clear ( )
    {
      map1.clear ();
    }
  };



template<typename __key1, typename __key2, typename __key3, typename __less1, typename __less2, typename __less3>
  class strongtriplemapkey
{
 public:
  __key1 key1;
  __key2 key2;
  __key3 key3;
  strongtriplemapkey ( __key1 _key1, __key2 _key2, __key3 _key3 )
    {
      key1 = _key1;
      key2 = _key2;
      key3 = _key3;
    }
  inline bool operator< ( const strongtriplemapkey & h2 ) const
    {
      if ( __less1() ( key1, h2.key1 ) )
	return true;
      else if ( __less1() ( h2.key1, key1 ) )
	return false;
      if ( __less2() ( key2,  h2.key2 ) )
	return true;
      else if ( __less2() ( h2.key2, key2 ) )
	return false;
      if ( __less3() ( key3,  h2.key3 ) )
	return true;
      return false;
    }
};

template<typename __key1, typename __key2, typename __key3, typename __val, typename __less1 = less<__key1>, typename __less2  = less<__key2>, typename __less3 = less<__key3> >
class strongtriplemap // : public strongmap<__key1, void *, __less1>
  {
    protected:
    typedef strongtriplemapkey<__key1,__key2,__key3,__less1,__less2,__less3> __triplekey;
    typedef map<__triplekey, __val, less<__triplekey> > __map1;
    typedef typename __map1::iterator __iter;
  
    __map1 map1;
    public:
    class iterator
    {
      friend class strongtriplemap<__key1,__key2,__key3,__val,__less1,__less2,__less3>;
    protected:
      __iter iter;
    public:
      __key1 first() { return iter->first.key1; }
      __key2 second() { return iter->first.key2; }
      __key3 third() { return iter->first.key3; }
      __val fourth() { return iter->second; }
      __val value() { return iter->second; }
      bool operator==(const iterator& __x) const
      { return iter == __x.iter; }
      bool operator!=(const iterator& __x) const
      { return iter != __x.iter; }
      iterator& operator++()
	{ iter++; return *this; }
      iterator operator++(int)
      { iterator __tmp = *this; iter++;
	return __tmp; }
    };
    protected:
    iterator __begin, __end;
    public:

    iterator begin () 
    { __begin.iter = map1.begin(); return __begin; }
    iterator end ()
    { __end.iter = map1.end(); return __end; }

    strongtriplemap() {}
    typedef typename __map1::const_iterator citerator;
    inline citerator cbegin () const
    { return map1.begin (); }
    inline citerator cend () const
    { return map1.end (); }
    inline __val get ( const __key1& key1, const __key2& key2, const __key3& key3 ) const
    {
      __triplekey key ( key1, key2, key3 );
      citerator it = map1.find ( key );
      if ( it == map1.end () )
	{
	  return (__val) NULL;
	}
      return it->second;
    }
    inline bool put ( const __key1& key1, const __key2& key2, const __key3& key3, const __val val ) 
    {
      if ( (void*) get ( key1, key2, key3 ) != NULL )
	__STRONGMAP_Bug ( "Not a unique put, already has a value.\n" );
      __triplekey key ( key1, key2, key3 );
      map1.insert ( make_pair ( key, val ) );
      return true;
    }
    inline bool remove ( const __key1& key1, const __key2& key2, const __key3& key3 )
    {
      __triplekey key ( key1, key2, key3 );
      map1.erase ( map1.find ( key ) );
      return true;
    }
    bool remove ( iterator & iter )
    {
      //      if ( map1.find ( iter.iter ) == map1.end() )
      //	return false;
      // iter.iter = 
      map1.erase ( iter.iter );
      return true;
    }
    inline unsigned int count ( const __key1& key1, const __key2& key2, const __key3& key3 )
    {
      __triplekey key ( key1, key2, key3 );
      return (unsigned int) map1.count ( key );
    }
    inline unsigned int size ( )
    {
      return (unsigned int ) map1.size ();
    }
  };

template<typename __key1, typename __key2, typename __key3, typename __val, typename __less1 = less<__key1>, typename __less2  = less<__key2>, typename __less3 = less<__key3> >
class strongtriplemap2 // : public strongmap<__key1, void *, __less1>
  {
    protected:
    typedef strongmap<__key3,__val> __map_type_3;
    typedef strongmap<__key2, __map_type_3 * > __map_type_2;
    typedef strongmap<__key1, __map_type_2 * > __map_type;
    __map_type __map;
    public:
    strongtriplemap2() {}
    inline __val get ( const __key1& key1, const __key2& key2, const __key3& key3 ) const
    {
      __map_type_2 * m2 = __map.get(key1);
      if ( m2 == NULL )
	return ( (__val) NULL );
      __map_type_3 * m3 = m2->get ( key2 );
      if ( m3 == NULL )
	return ( (__val) NULL );
      return m3->get ( key3 );
    }
    inline bool put ( const __key1& key1, const __key2& key2, const __key3& key3, const __val val ) 
    {
      __map_type_2 * m2 = __map.get(key1);
      if ( m2 == NULL )
	{
	  m2 = new __map_type_2();
	  __map.put ( key1, m2 );
	}
      __map_type_3 * m3 = m2->get(key2);
      if ( m3 == NULL )
	{
	  m3 = new __map_type_3();
	  m2->put ( key2, m3 );
	}
      return m3->put ( key3, val );
    }
  };
#endif // __STRONGMAP_H
