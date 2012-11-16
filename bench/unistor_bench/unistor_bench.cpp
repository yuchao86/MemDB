#include "CwxAppProcessMgr.h"
#include "UnistorBenchApp.h"

int main(int argc, char** argv){
    UnistorBenchApp* pApp = new UnistorBenchApp();
    if (0 != CwxAppProcessMgr::init(pApp)) return 1;
    CwxAppProcessMgr::start(argc, argv, 200, 300);
    return 0;
}
