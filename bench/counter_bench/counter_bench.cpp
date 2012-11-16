#include "CwxAppProcessMgr.h"
#include "CounterBenchApp.h"

int main(int argc, char** argv){
    CounterBenchApp* pApp = new CounterBenchApp();
    if (0 != CwxAppProcessMgr::init(pApp)) return 1;
    CwxAppProcessMgr::start(argc, argv, 200, 300);
    return 0;
}
