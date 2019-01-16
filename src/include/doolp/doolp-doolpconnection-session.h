

class DoolpConnectionSession : public DoolpConnection
{
  DoolpMsg_BlockHeader header;
  DoolpConnection * father;

  DoolpConnectionSession::DoolpConnectionSession
    ( DoolpConnection * _father;
      DoolpMsg_BlockHeader * h )
    {
      memcpy ( &header, h, sizeof ( DoolpMsg_BlockHeader ) );
      father = _father;
    }

  DoolpConnectionSession::~DoolpConnectionSession ()
    {
      Log ( "Quitting Session Connection : %p\n", this );
    }
};
