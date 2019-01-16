// __DOOLP_VIRTUAL__
// __DOOLP_VIRTUAL_IMPL__

__DOOLP_VIRTUAL__ bool Write ( int i ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Write ( int * i ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Read ( int * i ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Read ( bool * b ) __DOOLP_VIRTUAL_IMPL__;

__DOOLP_VIRTUAL__ bool Write ( ObjectId id ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Read ( ObjectId * id ) __DOOLP_VIRTUAL_IMPL__;

__DOOLP_VIRTUAL__ bool Write ( char * s ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Read ( char ** s) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Write ( string * s ) __DOOLP_VIRTUAL_IMPL__;
//__DOOLP_VIRTUAL__ bool Read ( string * s) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Read ( string ** s) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Write ( string & s ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Read ( string * s) __DOOLP_VIRTUAL_IMPL__; // Not shure of this...

__DOOLP_VIRTUAL__ bool Write ( float f ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Read ( float * f ) __DOOLP_VIRTUAL_IMPL__;

__DOOLP_VIRTUAL__ bool Write ( FullContextId * id ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Read ( FullContextId ** id ) __DOOLP_VIRTUAL_IMPL__;

__DOOLP_VIRTUAL__ bool WriteObjectHead ( Object * obj ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Write ( Object * obj, ContextStepping fromStepping ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Read ( Object ** obj, bool canResolve, bool mustResolve ) __DOOLP_VIRTUAL_IMPL__;

__DOOLP_VIRTUAL__ bool Write ( Exception * e ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Read ( Exception ** e ) __DOOLP_VIRTUAL_IMPL__;

// Raw read of sections for Doolp::ObjectBuffer
__DOOLP_VIRTUAL__ bool WriteRaw ( void * buffer, int size ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool ReadRaw ( void ** buffer, int * size ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Write ( ObjectBufferParam * param ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool Read ( ObjectBufferParam ** param ) __DOOLP_VIRTUAL_IMPL__;


/*
 * Session-organized messaging
 */ 
__DOOLP_VIRTUAL__ bool startNewCall ( Call * call ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool startReply ( Job * job ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool startParamSubSection ( ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool readParamSubSection () __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool endSubSection ( ) __DOOLP_VIRTUAL_IMPL__;	
__DOOLP_VIRTUAL__ bool readSubSectionEnd ( ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool endMessage   ( ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool readMessageEnd ( ) __DOOLP_VIRTUAL_IMPL__;

__DOOLP_VIRTUAL__ bool startList ( unsigned int listIndex ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ unsigned int readList ( ) __DOOLP_VIRTUAL_IMPL__;

__DOOLP_VIRTUAL__ bool setNextBlockIndex ( unsigned int blockIndex ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ unsigned int getNextBlockIndex ( ) __DOOLP_VIRTUAL_IMPL__;

__DOOLP_VIRTUAL__ bool readObjectSubSection () __DOOLP_VIRTUAL_IMPL__;

__DOOLP_VIRTUAL__ bool setStream ( CallContext * callContext,
				   StreamIndex idx ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool readStreamSubSection () __DOOLP_VIRTUAL_IMPL__;  
__DOOLP_VIRTUAL__ bool readStream ( CallContext * callContext, StreamVirtual * stream, 
				    StreamIndex idx ) __DOOLP_VIRTUAL_IMPL__;  
__DOOLP_VIRTUAL__ bool leaveStream ( ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool endStream ( ) __DOOLP_VIRTUAL_IMPL__;
__DOOLP_VIRTUAL__ bool readStreamEnd () __DOOLP_VIRTUAL_IMPL__;  

__DOOLP_VIRTUAL__ bool readExceptionSubSection () __DOOLP_VIRTUAL_IMPL__;  


#undef __DOOLP_VIRTUAL__
#undef __DOOLP_VIRTUAL_IMPL__
