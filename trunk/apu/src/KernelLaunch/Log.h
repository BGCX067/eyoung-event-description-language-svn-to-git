#ifndef _LOG_H_
#define _LOG_H_

extern int nBytes;

class Sample {

public:

    Sample() : _isMsg(false), _isErr(false), _timer(0.), _loops(1), _msg(0) {}
    ~Sample() {}

    void   setMsg( const char *, const char * );
    void   setErr( void ) { _isErr = true; }
    bool   isMsg( void ) { return _isMsg; }
    bool   isErr( void ) { return _isErr; }
    void   setTimer( const char *, const char *, double, unsigned int, int );
    double getTimer( void ) { return _timer; }
    void   printSample ( void );

private:

    bool          _isMsg;
    bool          _isErr;
    double        _timer;
    unsigned int  _nbytes;
    int           _loops;
    char *        _fmt;
    char *        _msg;
};

class TestLog {

public:

    TestLog( int );
    ~TestLog() {}

    void loopMarker( void );
    void Msg( const char *, const char * );
    void Error( const char *, const char * );
    void Timer( const char *, const char *, double, unsigned int, int );

    void printLog( void );
    void printSummary( int );

private:

    int _logIdx;
    int _logLoops;
    int _logLoopEntries;
    int _logLoopTimers;

    Sample *_samples;
};

#endif // _LOG_H_
