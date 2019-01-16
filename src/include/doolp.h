#ifndef __DOOLP_H_
#define __DOOLP_H_

#include <doolp-config.h>

#include <glog.h>
#include <stdio.h>
#include <stdlib.h>

// Main Object : DoolpForge
#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpcall.h>

// DoolpForge Services
#include <doolp/doolp-doolpforgeservices.h>
#include <doolp/doolp-doolpobjectindexer.h>
#include <doolp/doolp-doolpnaming.h>
#include <doolp/doolp-doolppersistance.h>

// Defines DoolpObject Class
#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpobject-link.h>

// Defines DoolpStream Class
#include <doolp/doolp-doolpstream.h>

// Defines DoolpExceptions
#include <doolp/doolp-doolpexceptions.h>

/*
 * Defines Connection Architectures
 */
#ifdef __DOOLP_INCLUDES_FOR_DOOLPCONNECTIONS__
#include <doolp/doolp-doolpconnection.h>
#include <doolp/doolp-doolpconnection-tcp.h>
#include <doolp/doolp-doolpconnection-aggregate.h>
#include <doolp/doolp-doolpconnection-xml.h>
#endif

/*
 * Service implementations
 */
#ifdef __DOOLP_INCLUDES_FOR_DOOLPFORGESERVICES__
#include <doolp/doolp-doolpobjectindexerbyrule.h>
#include <doolp/doolp-doolpobjectindexerbyruleservice.h>
#include <doolp/doolp-doolpnamingcache.h>
#include <doolp/doolp-doolpnamingfile.h>
#include <doolp/doolp-doolppersistance-ftree.h>
#include <doolp/doolp-doolppubsub-service.h>
#include <doolp/doolp-doolppubsub-broker.h>
#endif

/*
 * For doolpCC-generated files
 */
#ifdef __DOOLP_INCLUDES_FOR_DOOLPCC__
#include <doolp/doolp-doolpobjectstaticinfo.h>
#include <doolp/doolp-doolpobjectdynamicinfo.h>
#include <doolp/doolp-doolpobject-slot-static.h>
#include <doolp/doolp-doolpconnection-tcp-server.h>
#endif // __DOOLP_INCLUDES_FOR_DOOLPCC__


#endif //  __DOOLP_H_
 
