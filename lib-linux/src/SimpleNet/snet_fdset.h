///
/// @author Kevin Lynx
/// @date 5.22.2008
/// @modify by lbzhung
///

#ifndef _SNET_FDSET_H_
#define _SNET_FDSET_H_

#include <stdlib.h>

namespace lib_linux
{
    ///
    /// wrap fd_set so that it's dependent on the value of FD_SETSIZE.
    ///
#if defined(_WIN32)
    class Fdset
    {
        public:
            /// constructor
            Fdset( unsigned int max_fd = FD_SETSIZE )
            {
                _max_count = max_fd > FD_SETSIZE ? max_fd : FD_SETSIZE;
                _set = (fd_set*) malloc( sizeof( fd_set ) + sizeof( SOCKET ) * ( _max_count - FD_SETSIZE ) );
                clear();
            }

            Fdset(const Fdset& _instance)
            {
                set_size(_instance.max_count());
                clear();        

                fd_set *_ins_set = _instance._set;

                if( _ins_set->fd_count > 0)
                {
                    memcpy(_set->fd_array, _ins_set->fd_array, sizeof( _set->fd_array[0] ) * _ins_set->fd_count );
                    _set->fd_count = _ins_set->fd_count ;
                }
            }

            const Fdset &operator = (const Fdset& _rhs)
            {
                if (this != &_rhs)
                {
                    set_size(_rhs.max_count());
                    clear();

                    fd_set *_ins_set = _rhs._set;

                    if( _ins_set->fd_count > 0)
                    {
                        memcpy(_set->fd_array, _ins_set->fd_array, sizeof( _set->fd_array[0] ) * _ins_set->fd_count );
                        _set->fd_count = _ins_set->fd_count ;
                    }
                }

                return *this;
            }

            /// destructor
            ~Fdset()
            {
                free( _set );
            }

            /// convert a fd_set type object.
            operator fd_set* ()
            {
                return _set;
            }

            /// 
            /// set the fd count
            ///
            void set_size( unsigned int max_fd )
            {
                if( max_fd <= _max_count )
                {
                    return ;
                }

                _max_count = max_fd;
                _set = (fd_set*) realloc( _set, sizeof( fd_set ) + sizeof( SOCKET ) * ( _max_count - FD_SETSIZE ) );
            }

            ///
            /// add a fd into the set
            ///
            bool add( SOCKET s )
            {
                // no space to add.
                if( _set->fd_count >= _max_count )
                {
                    return false;
                }

                unsigned int i;
                for( i = 0; i < _set->fd_count; ++ i )
                {
                    if( _set->fd_array[i] == s )
                    {
                        // already exist.
                        return true;
                    }
                }

                // otherwise, i==fd_count
                _set->fd_array[_set->fd_count++] = s;
                return true;
            }

            ///
            /// remove a fd from the set
            ///
            void remove( SOCKET s )
            {
                FD_CLR( s, _set );
            }

            ///
            /// clear the set
            ///
            void clear()
            {
                FD_ZERO( _set );
            }

            // socket count
            unsigned int count()
            {
                return _set->fd_count;
            }

            /// 
            /// check one fd is in this set?
            ///
            bool is_set( SOCKET s ) const
            {
                return FD_ISSET( s, _set ) != 0;
            }

            /// return the max fd count
            int max_count() const
            {
                return _max_count;
            }

        private:
            /// fd_set
            ::fd_set *_set;
            /// max fd count
            unsigned int _max_count;
    };
#else
    // TODO fd_set max fd is less FD_SETSIZE(1024) on linux
    class Fdset
    {
        public:
            /// constructor
            Fdset( unsigned int max_fd = FD_SETSIZE )
                :_fd_count(0)
            {
                ASSERT(max_fd <= FD_SETSIZE && "fd_set max fd is less FD_SETSIZE(1024)");
                _max_count = max_fd;
                clear();
            }

            /// convert a fd_set type object.
            operator fd_set* ()
            {
                return &_set;
            }

            /// 
            /// set the fd count
            ///
            void set_size( unsigned int max_fd )
            {
                ASSERT(max_fd <= FD_SETSIZE && "fd_set max fd is less FD_SETSIZE(1024)");
                _max_count = max_fd;
            }

            ///
            /// add a fd into the set
            ///
            bool add( SOCKET s )
            {
                // no space to add.
                if( _fd_count >= _max_count )
                {
                    return false;
                }

                if (!is_set(s))
                {
                    _fd_count++;
                }
                FD_SET(s, &_set);
                return true;
            }

            ///
            /// remove a fd from the set
            ///
            void remove( SOCKET s )
            {
                if (is_set(s))
                {
                    _fd_count--;
                    ASSERT(_fd_count >=0);
                }
                FD_CLR( s, &_set );
            }

            ///
            /// clear the set
            ///
            void clear()
            {
                FD_ZERO( &_set );
                _fd_count = 0;
            }

            // socket count
            unsigned int count()
            {
                return _fd_count;
            }

            /// 
            /// check one fd is in this set?
            ///
            bool is_set( SOCKET s ) const
            {
                return FD_ISSET( s, &_set ) != 0;
            }

            /// return the max fd count
            int max_count() const
            {
                return _max_count;
            }

        private:
            /// fd_set
            ::fd_set _set;
            /// max fd count
            unsigned int _max_count;
            /// fd count current
            unsigned int _fd_count;
    };

#endif
}

#endif
