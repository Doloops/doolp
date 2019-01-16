/*
 * Template for CPP generation.
 */

#define _wr(...) fprintf ( fp_out, __VA_ARGS__ )
#define _wrParams(__sec__,__format__)				    \
  forIterSectionsOfType ( __p__, (__sec__), "Parameter" )	    \
    if ( (*__p__)->isVal ( "type", "DoolpConnection *" ) ) continue;	\
    _wr ( __format__, (*__p__)->getVal( "type" ), (*__p__)->name ); \
  endFor
#define _wrParamsWithIndexBefore(__sec__,__format__)				    \
  forIterSectionsOfType ( __p__, (__sec__), "Parameter" )	    \
    if ( (*__p__)->isVal ( "type", "DoolpConnection *" ) ) continue;	\
    _wr ( __format__, (*__p__)->getVal("index"), (*__p__)->getVal( "type" ), (*__p__)->name ); \
  endFor
#define _wrParamsWithIndexAfter(__sec__,__format__)				    \
  forIterSectionsOfType ( __p__, (__sec__), "Parameter" )	    \
    if ( (*__p__)->isVal ( "type", "DoolpConnection *" ) ) continue;	\
    _wr ( __format__, (*__p__)->getVal( "type" ), (*__p__)->name, (*__p__)->getVal("index") ); \
  endFor
#define _wrParamsWithSep(__sec__,__format__,__sep__)		    \
  forIterSectionsOfType ( __p__, (__sec__), "Parameter" )	    \
    if ( (*__p__)->isVal ( "type", "DoolpConnection *" ) ) continue;	\
    _wr ( __format__, (*__p__)->getVal( "type" ), (*__p__)->name ); \
  __p__ ++;							    \
  if ( __p__ != (__sec__)->sections.end () ) _wr ( __sep__ );	    \
  __p__ --;							    \
  endFor
#define _wrStreamsNoType(__sec__,__format__) \
  forIterSectionsOfType ( __p__, (__sec__), "Stream" )	    \
    _wr ( __format__, (*__p__)->name, (*__p__)->getVal("index") ); \
  endFor

#define _wrStreams(__sec__,__format__) \
  forIterSectionsOfType ( __p__, (__sec__), "Stream" )	    \
    _wr ( __format__, (*__p__)->getVal( "type" ), (*__p__)->name, (*__p__)->getVal("index") ); \
  endFor
#define _wrStreamsFullType(__sec__,__format__) \
  forIterSectionsOfType ( __p__, (__sec__), "Stream" )	    \
    _wr ( __format__, (*__p__)->getVal( "fullType" ), (*__p__)->name, (*__p__)->getVal("index") ); \
  endFor
/*
#define _wrStreamsFullTypeAfter(__sec__,__format__) \
  forIterSectionsOfType ( __p__, (__sec__), "Stream" )	    \
    _wr ( __format__, (*__p__)->name, (*__p__)->getVal( "fullType" ), (*__p__)->getVal("index") ); \
  endFor
*/
#define _DoolpObjectConstructor(obj)					\
  _wr ( "DoolpObject * __%s__constructor ( DoolpObjectId objId )\n"	\
	"{\n"								\
	"\tDoolpObject * obj = new %s ( objId );\n"			\
	"\t__DOOLP_Log ( \"New Object %s : %%p\\n\", obj );\n"		\
	"\treturn obj;\n"						\
	"}\n"								\
	, obj->name, obj->name, obj->name );				\
  _wr ( "DoolpNameId %s::getDoolpNameId ( )\n"				\
	"\t{ return %s; }\n", obj->name, obj->getVal ( "nameId" ) )
#define _DoolpObjectParamOffsetTable(obj) \
  _wr ( "int *%s::paramOffsetTable = NULL;\n", obj->name ); \
  _wr ( "void %s::__checkParamOffsetTable ()\n" \
	"{\n\tif ( paramOffsetTable ) return;\n" \
	, obj->name ); \
  _wr ( "\tparamOffsetTable = (int [])\n\t{\n" ); \
  _wrParamsWithIndexAfter(obj,"\t\t/* Type %s */(int)&%s - (int) this, %s,\n" ); \
  _wr ( "\t\t0, 0\n\t};\n}\n" );

#define _DoolpObjectSerializers(obj)		\
  _wr ( "bool %s::serialize ( DoolpConnection * conn )\n"	\
	"{\n", obj->name );					\
  _wr ( "\tconn->WriteObjectHead ( this );\n" ); \
  _wrParamsWithIndexBefore ( obj, "\tconn->setNextBlockIndex ( %s );\n" \
	      "\tconn->Write ( /* %s */ %s );\n" );	\
  _wr ( "\tconn->endSubSection ();\n" ); \
  _wr ( "\treturn true;\n"					\
	"}\n" );      					\
  /******************** Ser by paramId *****************/ \
  _wr ( "inline bool %s::serialize ( DoolpConnection * conn, const int paramId )\n"	\
	"{\n", obj->name );					\
  _wr (	"\tswitch ( paramId )\n\t{\n" ); \
  _wrParamsWithIndexBefore ( obj, "\t\tcase %s: " \
              "conn->setNextBlockIndex ( paramId );\n" \
	      "\t\t\tconn->Write ( /* %s */ %s ); break;\n" );	\
  _wr ( "\t\tdefault: Bug ( \"Index not handled : %%p\\n\", paramId);\n" ); \
  _wr ( "\t}\n\treturn true;\n"					\
	"}\n" );      					\
  /***************** Unserializer ***********************/ \
  _wr ( "bool %s::unserialize ( DoolpConnection * conn )\n"	\
	"{\n", obj->name );			\
  _wr ( "\tunsigned int blockIndex;\n" ); \
  _wr ( "\twhile ( ( blockIndex = conn->getNextBlockIndex () ) )\n" \
	"\tswitch ( blockIndex )\n\t{\n" );\
  _wrParamsWithIndexBefore ( obj, "\t\tcase %s: conn->Read ( /* %s */ &(%s) ); break;\n" );	\
  _wr ( "\t\tdefault: Bug ( \"Index not handled : %%p\\n\", blockIndex )\n" ); \
  _wr ( "\t}\n" ); \
  _wr ( "\treturn true;\n"					\
	"}\n" );



#define _DoolpObjectInfo(name, nameId, rpcNb)				\
  _wr ( "DoolpObjectInfo DoolpObject__%s =\n"				\
	"{\n"								\
	"\t\"%s\", // name\n"						\
	"\t%s, // nameId\n"						\
	"\t__%s__constructor, // constructor\n"				\
	"\t__%s__rpcTable, // rpcTable\n"				\
	/* "\t%d, // rpcNb\n"	*/					\
	"};\n",								\
	name, name, nameId, name, name /* rpcNb */ )

#define _DoolpRPC_prepare(_root, _func)				      \
  _wr ( "typedef struct %s_prepareStruct\n"			      \
	"{\n",							      \
	_root );						      \
  _wrParams ( _func, "\t%s %s;\n" );				      \
  _wrStreamsFullType ( _func, "\t%s %s; /* stream at index %s*/ \n" ); \
  _wr ( "\tunsigned int res;\n"					      \
	"};\n" );						      \
  _wr ( "int %s_prepare ( DoolpJob *job, DoolpConnection * conn, void ** buffer )\n" \
	"{\n"							      \
	"\t*buffer = malloc ( sizeof ( %s_prepareStruct ) ); \n"      \
	"#define __bf ((%s_prepareStruct*) *buffer)\n"		      \
	, _root, _root, _root );				      \
  _wrParams ( _func, "\tconn->Read(/* %s */ &(__bf->%s));\n" );	      \
  _wr ( "\tconn->readSubSectionEnd ();\n" ); \
  _wrStreamConstructions ( "(format not used)", _func ); \
  _wrStreamsNoType ( _func, "\t__bf->%s->bind ( conn, job, %s, DoolpStreamBindOption_RdWr );\n" ); \
  _wr ( "#undef __bf\n"						      \
	"\treturn sizeof ( %s_prepareStruct );\n"		      \
	"}\n", _root )

#define _DoolpRPC_run(_class, _root, _func)				\
    _wr ( "bool %s_run ( DoolpJob * job, DoolpConnection * conn,\n"	\
	  "\tDoolpObject * object, void * buffer,  int size )\n"	\
	  "{\n"								\
	  "#define __bf ((%s_prepareStruct*) buffer)\n"			\
	  , _root, _root );						\
    _wr ( "\t%s result = ( (%s *)object)->%s (\n"			\
	  , _func->getVal ( "returnType" ), _class			\
	  , _func->getVal("localName") );				\
    _wr ( "\t\t" ); \
    _wrStrippedParams ( "__bf->%s",_func ); \
   /* _wr ( "\t\t%s\n", _func->getVal ( "strippedParams" ) ); */ \
/*    _wrParamsWithSep ( _func, "\t\t  %s  __bf->%s", ",\n" );	*/	\
    _wr ( " );\n" );						\
    _wrStreamsNoType ( _func, "\t__bf->%s->unbind ( job ) /* index %s */;\n" ); \
    _wr ( "#undef __bf\n" );						\
    _wr ( "\tconn->startReply ( job );\n"				\
          "\tconn->startParamSubSection ( ); \n" \
	  "\tconn->Write ( result );\n"					\
          "\tconn->endSubSection ( ); \n"); \
  if ( ! Mode_stdRPC )							\
  {  _wr ( "\tobject->forge->serializeObjectsFromStepping ( conn, 0 );\n" ); } \
    _wr ( "\tconn->endMessage ();\n"					\
	  "\tfree (buffer);\n"						\
	  "\treturn true;\n" );						\
    _wr ( "}\n" )

// #define _wrForge							

#define _DoolpRPC_handleReply(_root, _func )				\
  _wr ( "bool %s_handleReply ( DoolpCall * call,\n"			\
	"\tDoolpConnection * conn )\n"					\
	"{\n"								\
	, _root );							\
  _wr ( "\t%s * result = (%s *) malloc ( sizeof (%s) );\n"		\
	,_func->getVal ( "returnType")					\
	,_func->getVal ( "returnType")					\
	,_func->getVal ( "returnType")					\
	);								\
  _wr ( "\tconn->Read ( result ); \n" );				\
  _wr ( "\tconn->readSubSectionEnd (); \n" );				\
  _wr ( "\tcall->result = (void*) result; \n" );			\
  _wr ( "\tcall->resultSize = sizeof ( %s ); \n",			\
	_func->getVal ( "returnType") );				\
  _wr ( "\treturn true;\n" );						\
  _wr ( "}\n" );

#define _wrForge							\
  if ( Mode_stdRPC )							\
    _wr ( "\t" ); else _wr ( "\tforge->" );			       
#define _DoolpRPC_strap_getResult(_class, _root, _func )	\
  _wr ( "%s %s__%s__getResult ( DoolpCall * call )\n"		\
	"{\n"							\
	"\t%s res = *((%s *) call->result);\n"			\
	"\tfree ( call->result);\n"				\
	"\tcall->result = NULL;\n"				\
	"\treturn res;\n"					\
	"}\n"							\
	, _func->getVal ( "returnType" )			\
	, _class, _func->name					\
	, _func->getVal ( "returnType" )			\
	, _func->getVal ( "returnType" )			\
	);							
  

#define _DoolpRPC_strap(_class, _root, _func )			\
  _DoolpRPC_strap_getResult (_class, _root, _func)		\
  _wr ( "%s %s::%s ( %s )\n"					\
	"{\n", _func->getVal ( "returnType" )			\
	, _class, _func->name					\
	,_func->getVal ( "signature" ) );			\
  if ( ! Mode_stdRPC )						\
    { _wr ( "\tDoolpConnection * conn;\n" ); }			\
  else if ( ! _func->getBool ( "hasDoolpConnection" ) )		\
    { _wr ( "\tDoolpConnection * conn = \n"			\
	    "\t\tgetDefaultConnection ();\n" ); }		\
  _wr ( "\tDoolpCall * call;\n" );				\
  _wrForge; _wr ( "newCall ( %s, objectId,\n"			\
		  "\t\t  &call, &conn );\n"			\
		  , _func->getVal ( "functionId" ) );		\
  _wr ( "\tconn->startParamSubSection ();\n" ); \
  _wrParams ( _func, "\tconn->Write ( /* %s */ %s );\n" );	\
  _wr ( "\tconn->endSubSection ();\n" ); \
  _wrStreamsNoType ( _func, "\t%s->bind ( conn, call, %s, DoolpStreamBindOption_RdWr );\n" ); \
  _wr ( "\tconn->endMessage ();\n" );				\
  _wrForge; _wr ( "addCall ( call );\n" ); 			\
  _wr ( "\tif ( call->async ) return (%s) NULL;\n"			\
	, _func->getVal ( "returnType") );				\
  _wrForge; _wr ( "waitCall ( call );\n" );				\
  _wrStreamsNoType ( _func, "\t%s->unbind ( call ); /* index %s*/\n" ); \
  _wr ( "\t%s res = %s__%s__getResult (call);\n"			\
	, _func->getVal ( "returnType" )				\
	, _class, _func->name );					\
  _wrForge; _wr ( "removeCall ( call );\n"				\
		  "\treturn res;\n"					\
		  "}\n" );

#define _DoolpRPC_head(_root, _func)					\
  if ( Mode_stdRPC )							\
    _wr ( "DoolpRPCId %s__rpcId = %s;\n",			\
	  _root, _func->getVal ( "functionId" ) );		\
  _wr ( "DoolpRPC %s =\n"						\
	"{\n"								\
	"\t%s, // rpcId\n"						\
	"\t\"%s\", // rpcName\n"					\
	"\t%s, // isStdRPC\n"						\
	, _root, _func->getVal ( "functionId" ), _func->name,		\
	Mode_stdRPC ? "true" : "false" )


#define _DoolpRPC( _root, _func )					\
  _DoolpRPC_head ( _root, _func );					\
  if ( _func->getBool ( "isImplementedLocally" ) )			\
    _wr ( "\t%s_prepare, // prepare\n"					\
	  "\t%s_run, // run\n"						\
	  , _root, _root );						\
  else _wr ( "\tNULL, NULL, // Not Implemented Locally\n" );		\
  _wr ( "\t%s_handleReply, // handleReply\n"				\
	"};\n",								\
	_root )								
