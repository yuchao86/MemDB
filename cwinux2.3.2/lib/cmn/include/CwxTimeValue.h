#ifndef  __CWX_TIME_VALUE_H__
#define  __CWX_TIME_VALUE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file CwxTimeValue.h
*@brief CwxTimeValue����
*@author cwinux@gmail.com
*@version 1.0
*@date  2019-06-05
*@warning  nothing
*@bug    
*/

#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"

CWINUX_BEGIN_NAMESPACE

/**
* @class CwxTimeValue
*
* @brief Operations on "timeval" structures, which express time in
* seconds (secs) and microseconds (usecs).
*/
class CWX_API CwxTimeValue
{
public:
    enum
    {
        CWX_MIN_USECS = 0,
        CWX_MAX_USECS = 1000000
    };
public:
    /// Constant "0".
    static const CwxTimeValue ZERO;
    ///Constant for maximum time representable.
    static const CwxTimeValue MAX;

    /// Default Constructor.
    CwxTimeValue(void);
    /// Constructor.
    explicit CwxTimeValue(time_t sec, suseconds_t usecs = 0);
    /// Construct the CwxTimeValue from a timeval.
    explicit CwxTimeValue (const timeval &t);
    /// Initializes the CwxTimeValue from seconds and useconds.
    void set (time_t sec, suseconds_t usecs);
    /// Initializes the CwxTimeValue from a timeval.
    void set (const timeval &t);
    ///Initializes the CwxTimeValue from microseconds format.
    void set (CWX_UINT64 usecs);
    /// Get seconds.
    time_t sec (void) const;
    /// Set seconds.
    void sec(time_t sec);
    /// Get microseconds.
    suseconds_t usec (void) const;
    /// Set microseconds.
    void usec (suseconds_t usec);
    /// set current datetime
    void now();
    ///Converts from CwxTimeValue format into microseconds format. 
    CWX_UINT64 to_usec(void) const;
    /// Returns the value of the object as a timeval.
    operator timeval () const;
    /// Returns a pointer to the object as a timeval.
    operator const timeval *() const;
    /// Add tv to this.
    CwxTimeValue &operator += (const CwxTimeValue &tv);
    /// Add tv to this.
    CwxTimeValue &operator += (time_t tv);
    /// Assign tv to this
    CwxTimeValue &operator = (const CwxTimeValue &tv);
    /// Assign tv to this
    CwxTimeValue &operator = (time_t tv);
    /// Subtract tv to this.
    CwxTimeValue &operator -= (const CwxTimeValue &tv);
    /// Substract  tv to this.
    CwxTimeValue &operator -= (time_t tv);
    /// Adds two CwxTimeValue objects together, returns the sum.
    friend CwxTimeValue operator + (const CwxTimeValue &tv1,
        const CwxTimeValue &tv2);
    /// Subtracts two CwxTimeValue objects, returns the difference.
    friend CwxTimeValue operator - (const CwxTimeValue &tv1,
        const CwxTimeValue &tv2);
    /// True if tv1 <  tv2.
    friend bool operator < (const CwxTimeValue &tv1,
        const CwxTimeValue &tv2);
    /// True if tv1 > tv2.
    friend bool operator > (const CwxTimeValue &tv1,
        const CwxTimeValue &tv2);
    /// True if  tv1 <=  tv2.
    friend bool operator <= (const CwxTimeValue &tv1,
        const CwxTimeValue &tv2);
    /// True if  tv1 >=  tv2.
    friend bool operator >= (const CwxTimeValue &tv1,
        const CwxTimeValue &tv2);
    /// True if tv1 ==  tv2.
    friend bool operator == (const CwxTimeValue &tv1,
        const CwxTimeValue &tv2);
    /// True if  tv1 !=  tv2.
    friend  bool operator != (const CwxTimeValue &tv1,
        const CwxTimeValue &tv2);
private:
    /// Put the timevalue into a canonical form.
    void normalize (void);
    /// Store the values as a timeval.
    timeval tv_;
};

/**
* @class CwxTimeouter
*
* @brief ��ʱ�����󣬼���Ƿ�ʱ
*
*/
class CWX_API CwxTimeouter
{
public:
    ///���캯����timeoutΪNULL��ʾû�г�ʱ
    CwxTimeouter(CwxTimeValue const* timeout=NULL);
    ///��������
    ~CwxTimeouter();
public:
    ///����Ƿ��Ѿ���ʱ,ͬʱ������ʣ���ʱ�䣻
    ///����ֵ��true ��ʱ��false û�г�ʱ��
    bool timeout() ;
    ///ʣ��ʱ�䣬NULL��ʾû�г�ʱ����
    CwxTimeValue const* left() const;
    ///�Ƿ���ж�ʱ������ֵ��true �ǣ�false û��
    bool isTimer() const;
    ///��ȡԭʼ��ʱʱ��
    CwxTimeValue const* getTimeout() const;
private:
    CwxTimeValue const*   timeout_; ///<��ʱʱ��
    CwxTimeValue    end_;  ///<����ʱ��
    CwxTimeValue    left_; ///<ʣ��ʱ��
};



CWINUX_END_NAMESPACE

#include "CwxTimeValue.inl"

#include "CwxPost.h"

#endif
