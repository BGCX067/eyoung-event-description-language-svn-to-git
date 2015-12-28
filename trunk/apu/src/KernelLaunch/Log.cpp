#include "Log.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

extern int status;


void Sample::setMsg( const char *fmt, const char *msg )
{
    _isMsg = true;

    _fmt = new char[ strlen( fmt )+1 ];
    strcpy( _fmt, fmt );
    _msg = new char[ strlen( msg )+1 ];
    strcpy( _msg, msg );
}

void Sample::setTimer( const char *fmt, const char *msg, double timer, unsigned int nbytes, int loops )
{
    _isMsg = false;
    _timer = timer;

    if( loops != 0 ) _loops = loops;
    if( nbytes > 0 ) _nbytes = nbytes;

    if ( strlen( msg ) > 0 )
    {
       _fmt = new char[ strlen( fmt )+1 ];
       strcpy( _fmt, fmt );
    }

    if ( strlen( msg ) > 0 )
    {
        _msg = new char[ strlen( msg )+1 ];
        strcpy( _msg, msg );
    }
}

void Sample::printSample( void )
{
    if( _isMsg == true )
        printf( _fmt, _msg );
    else
    {
       double bwd = (((double) _nbytes * _loops )/ _timer ) / 1e9;

       printf( _fmt, _msg, _timer, bwd );
    }
}

TestLog::TestLog( int nSamples ) : _logIdx(0), 
                                   _logLoops(0), 
                                   _logLoopEntries(0),
                                   _logLoopTimers(0)
{
    _samples = new Sample[ nSamples ];
}

void TestLog::loopMarker()
{
    _logLoopEntries = 0;
    _logLoopTimers = 0;
    _logLoops++;
}

void TestLog::Msg( const char *format, const char *msg )
{
    _samples[ _logIdx++ ].setMsg( format, msg );
    _logLoopEntries++;
}

void TestLog::Error( const char *format, const char *msg )
{
    _samples[ _logIdx ].setMsg( format, msg );
    _samples[ _logIdx++ ].setErr();
    _logLoopEntries++;
}

void TestLog::Timer( const char *format, const char *msg, double timer, unsigned int nbytes, int loops )
{
    _samples[ _logIdx++ ].setTimer( format, msg, timer, nbytes, loops );
    _logLoopEntries++; 
    _logLoopTimers++;
}

void TestLog::printLog( void )
{
    int idx = 0;

    std::cout << "\nLOOP ITERATIONS\n";
    std::cout << "---------------\n";

    for(int l=0; l < _logLoops; l++)
    {
        std::cout << "\nLoop " << l << std::endl;

       for( int i=0; i < _logLoopEntries; i++ )
          _samples[ idx++ ].printSample();
    }
}

void TestLog::printSummary( int skip )
{
// return if a error or expected error has occured
    if(status)
    return;
    std::cout << "\nAVERAGES (over loops " << skip << " - " << _logLoops - 1 << ", use -l for complete log)\n";
    std::cout << "--------\n";

    for( int i = 0; i < _logLoopEntries; i++ )
    {
       if( _samples[ i ].isMsg() )
       {
           bool foundError = false;

           for( int nl = 0; nl < _logLoops; nl++ )
           {
               int current =  i + nl * _logLoopEntries;

               if( _samples[ current ].isErr() )
               {
                   _samples[ current ].printSample();
                   foundError = true;
                   break;
               }
           }

           if( !foundError )
               _samples[ i ].printSample();
       }
       else
       {
           double sum=0.;

           for( int nl = skip; nl < _logLoops; nl++ )
           {
               sum += _samples[ i + nl * _logLoopEntries ].getTimer();
           }

           _samples[ i ].setTimer( "", "", sum / ( _logLoops-skip ), 0, 0 );
           _samples[ i ].printSample();
       }
   }
}
