#ifndef _SNET_SESSIONFACTORY_
#define _SNET_SESSIONFACTORY_

#include "snet_session.h"

namespace lib_linux
{
    // Session factory method
    class SessionFactory
    {
        public:
            virtual ISession* CreateSession() =0;

        protected:
            SessionFactory()
            {}

        private:
            SessionFactory(const SessionFactory&);
            SessionFactory& operator = (const SessionFactory&);
    };


	// Factory method template.
	// you can create new factory by this:
	//     SessionFactoryImp<MySession> sessionMy;
    template <class S>
    class SessionFactoryImp: public SessionFactory
    {
        public:
            ISession* CreateSession()
            {
                return new S;
            }
    };
}

#endif
