#ifndef __DOOLP_DOOLPNAMINGFILE_H
#define __DOOLP_DOOLPNAMINGFILE_H

#include <doolp/doolp-doolpnamingcache.h>

namespace Doolp
{
  class NamingFile : public NamingCache
  {
#define DoolpObject_CurrentObject NamingFile
    DoolpObject_stdFeatures ( );
    DoolpObject_Option ( forceObjectAs, Naming );

  protected:
    char * fileName;
    bool noflush;
  public:
    NamingFile ( char * fileName );
    virtual Object * getServiceAsObject();

    bool start () { return true; }
    bool stop () { return flush(); }
    bool readfile ();
    bool flush();
#undef DoolpObject_CurrentObject
  };

}; // NameSpace Doolp
#endif//  __DOOLP_DOOLPNAMINGFILE_H
