#include "CwxAppProcessMgr.h"
#include "UnistorApp.h"

int main(int argc, char** argv){
    //����unistor��app����ʵ��
    UnistorApp* pApp = new UnistorApp();
    //��ʼ��˫���̹�����
    if (0 != CwxAppProcessMgr::init(pApp)) return 1;
    //����˫���̣�һ��Ϊ���unistor���̵ļ�ؽ��̣�һ��Ϊ�ṩunistor����Ĺ������̡�
    CwxAppProcessMgr::start(argc, argv, 200, 300);
    return 0;
}
