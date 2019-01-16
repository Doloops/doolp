#ifndef __DOOLP_DOOLPSTREAM_NOTIFY_H_
#define __DOOLP_DOOLPSTREAM_NOTIFY_H_

namespace Doolp
{
  template <typename T_obj>
    class StreamNotifiable
  {
  public:
    virtual ~StreamNotifiable() {}
    virtual bool notifyStream ( T_obj & ) = 0; // Return true if you wish the stream buffer to be filled (despite notification)
  };

  template<typename T_obj>
    class StreamWithNotify : public Stream<T_obj>
  {
  protected:
    StreamNotifiable<T_obj> * notifiableObject;
  public:
    StreamWithNotify ( StreamNotifiable<T_obj> * _notifiableObject )
      { notifiableObject = _notifiableObject; }
    StreamWithNotify ( ) 
      { notifiableObject = NULL; }
    void setNotifiable ( StreamNotifiable<T_obj> * _notifiableObject )
    { notifiableObject = _notifiableObject; }
    bool readFrom ( Connection * connection, CallContext * callContext )
    {
      if ( _isFinished )
	Bug ( "This Stream is Finished and I still have data for it.\n" );
      if ( connection->readStreamEnd () )
	{ 
	  _isFinished = true; 
	  if ( notifiableObject == NULL )
	    sem_post ( &semaphore ) ; 
	  return true; 
	}
      T_obj t;
      connection->Read ( &t );
      if ( ( notifiableObject != NULL ) && ( notifiableObject->notifyStream ( t ) ) )
	{
	  recvBuff.push_back ( t );
	  sem_post ( &semaphore ) ; 
	}
      return true;
    }
  };

}; // NameSpace Doolp
#endif // __DOOLP_DOOLPSTREAM_NOTIFY_H_
