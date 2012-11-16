#include "CwxBinlogOp.h"

int main(int argc, char** argv){
    if (argc != 2){
        printf("Miss binlog file.\n Using: %s binlog-file.\n", argv[0]);
        return 0;
    }
    CwxBinlogOp binlogOp;
    if (0 != binlogOp.init(argv[1])) return 0;
    binlogOp.run();
    return 0;
}

