#include "CwxAppProcessMgr.h"
#include "CwxEchoChannelApp.h"

int main(int argc, char** argv){
    //����ECHO��app����ʵ��
    CwxEchoChannelApp* pApp = new CwxEchoChannelApp();
    //��ʼ��˫���̹�����
    if (0 != CwxAppProcessMgr::init(pApp)) return 1;
    //����˫���̣�һ��Ϊ���echo���̵ļ�ؽ��̣�һ��Ϊ�ṩecho����Ĺ������̡�
    CwxAppProcessMgr::start(argc, argv, 200, 300);
    return 0;
}
