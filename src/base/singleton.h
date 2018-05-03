#ifndef SINGLETON_HEADER_H
#define SINGLETON_HEADER_H
#include "mutex_lock.h"

template<typename T>
class singleton
{
public:

    static T* instance ( )
    {
        if ( NULL == _single_instance )
        {
            static MutexLock locker;
            MutexLockGuard g ( locker );
            _single_instance = new(std::nothrow ) T ( );
        }
        return _single_instance;
    }

protected:
    singleton ( ){}
    ~singleton ( ){}
    singleton ( const singleton &other ){}
    T& operator== ( const singleton &rhs ){}

private:
    static T* _single_instance;
};

#endif /* SINGLETON_H */

