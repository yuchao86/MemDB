#ifndef __CWX_STREAM_H__
#define __CWX_STREAM_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/



/**
@file cwx_stream.h
@brief apache module��ͨ�Žӿڶ���
@author cwinux@gmail.com
@version 0.1
@date 2009-10-12
@warning
@bug
*/

#include "cwx_config.h"

///��ȡ��ǰ��ʱ�䣬UINT64
CWX_UINT64 cwx_time_of_day();
/**
*@brief tcp���ӡ�
*@param [in] conn  ��������ӻ�������
*@param [in] millisecond ���ӵĳ�ʱms��
*@return -1��ʧ�ܣ�����Ϊ���ӵ�handle
*/
int cwx_stream_connect(CWX_CONNECT* conn, CWX_UINT32 millisecond);
/**
*@brief �������϶�ȡ���ݡ�
*@param [in] fd  ���ӵ�handle��
*@param [in] buf ��Ŷ�ȡ���ݵ�buf
*@param [in] length ��ȡ�����ݵĳ���
*@param [in] millisecond ��ȡ�ĳ�ʱms��
*@return -1����ȡʧ�ܣ�0����ʾ��ʱ������Ϊ��ȡ���ֽ�����Ӧ����length���
*/
int cwx_stream_read(CWX_CONNECT* conn, void *buf, CWX_UINT32 length, CWX_UINT32 millisecond);
/**
*@brief ��������д���ݡ�
*@param [in] fd  ���ӵ�handle��
*@param [in] buf �������ݵ�buf
*@param [in] length �������ݵĳ���
*@param [in] millisecond ���͵ĳ�ʱms��
*@return -1������ʧ�ܣ�0����ʾ��ʱ������Ϊ���͵��ֽ�����Ӧ����length���
*/
int cwx_stream_write(CWX_CONNECT* conn, void *buf, CWX_UINT32 length, CWX_UINT32 millisecond);
/**
*@brief �ر�tcp���ӡ�
*@param [in] fd  ���ӵ�handle��
*@return -1���ر�ʧ�ܣ�0���رճɹ�
*/
int cwx_stream_close(int fd);
#endif
