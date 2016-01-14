///
/// @author Kevin Lynx
/// @date 5.22.2008
/// @modify by lbzhung
///

#include "snet_config.h"
#include "snet_session.h"

namespace lib_linux
{
    // globe unique session id 
    int ISession::_start_id = 1;

    ISession::ISession()
    :_id(_start_id++)
    {
    }

    int ISession::getid()
    {
        return _id;
    }

    int ISession::recv( char *buf, int len )
    {
        return _recvbuf.get( buf, len );
    }

    int ISession::send( const char *buf, int len )
    {
        if( buf == NULL || len <= 0 )
            return 0;

        return socket_send(buf, len);
    }

    int ISession::do_recv()
    {
        // note : here i use a trick about my buffer class.
        // it's not safe, but i think it's effective.
        char *buf = _recvbuf.free_space();
        int len = _recvbuf.free_size(); 

        ASSERT( buf != 0 );

        // if the free memory is 0, realloc the memory
        if( len == 0 )
        {
            _recvbuf.realloc();
            buf = _recvbuf.free_space();
            len = _recvbuf.free_size();
        }

        // do recv
        int ret = socket_recv(buf, len);

        // cool, if ::recv received some data, the data has already been in the Buffer object.:D
        // only change the buffer pointer
        if( ret > 0 )
        {
            _recvbuf.move_pos( ret );
        }

        return ret;
    }

    Buffer& ISession::GetBuffer()
    {
        return _recvbuf;
    }
}
