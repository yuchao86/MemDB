#include "CwxAppProcessMgr.h"
#include "CwxEchoApp.h"

int main(int argc, char** argv){
    //����ECHO��app����ʵ��
    CwxEchoApp* pApp = new CwxEchoApp();
    //��ʼ��˫���̹�����
    if (0 != CwxAppProcessMgr::init(pApp)) return 1;
    //����˫���̣�һ��Ϊ���echo���̵ļ�ؽ��̣�һ��Ϊ�ṩecho����Ĺ������̡�
    CwxAppProcessMgr::start(argc, argv, 200, 300);
    return 0;
}
