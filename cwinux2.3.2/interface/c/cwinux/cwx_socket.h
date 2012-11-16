#ifndef __CWX_SOCKET_H__
#define __CWX_SOCKET_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/



/**
@file cwx_socket.h
@brief ��window��unixƽ̨��socket��
@author cwinux@gmail.com
@version 0.1
@date 2009-12-01
@warning
@bug
*/

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include<sys/un.h>
#endif


#ifdef CWX_SOCKET
#undef CWX_SOCKET;
#endif

#ifdef WIN32
#define CWX_SOCKET SOCKET
#define CWX_INVALID_SOCKET INVALID_SOCKET
#else
#define CWX_SOCKET int
#define CWX_INVALID_SOCKET -1
#endif


///��ȡ��ǰ��ʱ�䣬uint64_t
uint64_t cwx_socket_timeofday();
///��ʼ����, ��Ҫ��Ϊ�˼���winsocket 0:�ɹ���-1��ʧ�ܣ�
int cwx_socket_init(char* szErr1K);
/**
*@brief tcp���ӡ�
*@param [in] szHost  ���ӵ�������Ϊ��������IP��ַ��
*@param [in] unPort  ���ӵĶ˿ںš�
*@param [in] millisecond ���ӵĳ�ʱms��
*@param [in] szErr1K ����ʱ�Ĵ���buf������ΪNULL������ռ�������1K
*@return CWX_INVALID_SOCKET��ʧ�ܣ�����Ϊ���ӵ�handle
*/
CWX_SOCKET cwx_socket_tcp_connect(char const* szHost, unsigned short unPort, unsigned int millisecond, char* szErr1K);

/**
*@brief tcp���ӡ�
*@param [in] szIp  ���ӵ�IP��ַ��
*@param [in] unPort  ���ӵĶ˿ںš�
*@param [in] millisecond ���ӵĳ�ʱms��
*@param [in] szErr1K ����ʱ�Ĵ���buf������ΪNULL������ռ�������1K
*@return CWX_INVALID_SOCKET��ʧ�ܣ�����Ϊ���ӵ�handle
*/
CWX_SOCKET cwx_socket_unix_connect(char const* szPath, char* szErr1K);

/**
*@brief �������϶�ȡ���ݡ�
*@param [in] fd  ���ӵ�handle��
*@param [in] buf ��Ŷ�ȡ���ݵ�buf
*@param [in] length ��ȡ�����ݵĳ���
*@param [in] millisecond ��ȡ�ĳ�ʱms��
*@param [in] szErr1K ����ʱ�Ĵ���buf������ΪNULL������ռ�������1K
*@return -1����ȡʧ�ܣ�0����ʾ��ʱ������Ϊ��ȡ���ֽ�����Ӧ����length���
*/
int cwx_socket_read(CWX_SOCKET fd, void *buf, unsigned int length, unsigned int millisecond, char* szErr1K);

/**
*@brief ��������д���ݡ�
*@param [in] fd  ���ӵ�handle��
*@param [in] buf �������ݵ�buf
*@param [in] length �������ݵĳ���
*@param [in] millisecond ���͵ĳ�ʱms��
*@param [in] szErr1K ����ʱ�Ĵ���buf������ΪNULL������ռ�������1K
*@return -1������ʧ�ܣ�0����ʾ��ʱ������Ϊ���͵��ֽ�����Ӧ����length���
*/
int cwx_socket_write(CWX_SOCKET fd, void *buf, unsigned int length, unsigned int millisecond, char* szErr1K);

/**
*@brief �ر�tcp���ӡ�
*@param [in] fd  ���ӵ�handle��
*@return -1���ر�ʧ�ܣ�0���رճɹ�
*/
int cwx_socket_close(CWX_SOCKET fd);
#endif
