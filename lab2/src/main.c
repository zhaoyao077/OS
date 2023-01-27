#include <stdio.h>
#include <string.h>

#define ENTRY_SIZE 32
#define FILE_SYSTEM_SIZE 1000

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;



/************************** STRUCT DEFINITION ****************************/
/*************************************************************************/

/**
 * File node
 * 文件节点，根据type决定是文件夹或文件
 */
struct FileNode {
    char name[20];
    char fullname[2000];
    int type;            // 0:dir,1:file，-1:split node
    u32 size;
    int start_pos;       // start
    int sub_pos;         // start of sub dir, . and .. is 0
    u16 cluster;         // cluster number
};

/**
 * FAT12文件系统结构
 */
#pragma pack(1) // align by 1 byte
struct FAT12 {
    u16 BPB_BytesPerSector; // 每个扇区的字节数
    u8 BPB_SecPerClusters;  // 每个簇的扇区数
    u16 BPB_RsvSecCnt;      // Boot record占用的扇区数
    u8 BPB_NumFATs;         // FAT的数量,一般为2
    u16 BPB_RootEntCnt;     // 根目录文件数的最大值
    u16 BPB_TotSec16;       // 扇区数
    u8 BPB_Media;           // 媒体描述符
    u16 BPB_FATSz16;        // 一个FAT的扇区数
    u16 BPB_SecPerTrk;      // 每个磁道扇区数
    u16 BPB_NumHeads;       // 磁头数
    u32 BPB_HiddenSec;      // 隐藏扇区数
    u32 BPB_TotSec32;       // 如果BPB_TotSec16是0，则在这里记录
};
#pragma pack() // restore alignment

/**
 * Entry
 * 根目录入口
 */
struct RootEntry {
    char name[11];//8.3 file name
    u8 classification; //文件属性
    char reserves[10]; //保留位
    u16 wrtTime;
    u16 wrtDate;
    u16 fstCluster;
    u32 fileSize;
};



/************************** GLOBAL VARIABLES *****************************/
/*************************************************************************/
struct FileNode files[FILE_SYSTEM_SIZE]; // 1000 nodes at most
struct FAT12 header;                     // FAT12
int curFiles = 0;                        // current number of nodes
int RsvSec;
int Bytes_Per_Sector;
int Root_Entry_Cnt;
int Root_Sec;
int NumFATs;
int FATSz;
int Pre_Sec;



/************************ UTILITY FUNCTIONS ******************************/
/*************************************************************************/

/** 
 * Print chars
 * @param:cnt the content to print
 * @param:bytes the length of the content
*/
void my_print(char *str, int len);

/**
 * Set font color to red
 */
void set_color_red();

/**
 * Set font color back to default
 */
void set_default();

/**
 * Print a number
 * @param num number
 */
void PrintNum(u16 num) {
    char numbers[20];
    int size = 1;
    for (u16 t = num; t > 9; t /= 10) {
        size++;
    }
    for (int i = size - 1; i >= 0; i--) {
        // 不断除10，从低到高
        numbers[i] = num % 10 + '0';
        num /= 10;
    }
    my_print(numbers, size);
}

/**
 * print \n
 */
void PrintLn() {
    char *c = "\n";
    my_print(c, 1);
}

/**
 * Print error message
 * @param warn
 * @param warn_len
 * @param str
 * @param str_len
 */
void PrintError(char *warn, int warn_len, char *str, int str_len) {
    set_color_red();
    my_print(warn, warn_len);
    my_print(str, str_len);
    PrintLn();
    set_default();
}



/******************* FUNCTIONS FOR PROCESSING FILES **********************/
/*************************************************************************/

/**
 * Divide the files into different levels
 * 把数组的下一个元素设为 type = -1 的文件分隔符
 */
void DivideFiles() {
    files[curFiles++].type = -1;
}

/**
 * Initialize the file node
 * 初始化文件节点
 */
void InitFiles() {
    if (curFiles)
        return;

    files[curFiles].name[0] = '/';
    files[curFiles].name[1] = 0x00;
    files[curFiles].fullname[0] = '/';
    files[curFiles].fullname[1] = 0x00;
    files[curFiles].type = 0;
    curFiles++;

    DivideFiles();
}

/**
 * Read the boot record and put the data into header
 * @param fat12
 */
void ReadBootData(FILE *fat12) {
    struct FAT12 *head_ptr = &header;

    fseek(fat12, 11, SEEK_SET); // BPB begin from 11th byte
    fread(head_ptr, 1, sizeof(header), fat12);

    RsvSec = header.BPB_RsvSecCnt;
    NumFATs = header.BPB_NumFATs;
    Root_Entry_Cnt = header.BPB_RootEntCnt;
    Bytes_Per_Sector = header.BPB_BytesPerSector;
    Root_Sec = (Root_Entry_Cnt * ENTRY_SIZE + Bytes_Per_Sector - 1) / Bytes_Per_Sector;
    FATSz = header.BPB_FATSz16;
    Pre_Sec = 1 + NumFATs * FATSz;
}

/**
 * Get the type of FileNode
 * 0: nonsense
 * 1: file/dir
 * 2: .
 * 3: ..
 * @param name file name
 * @return type
 */
int TypeOf(char *name) {
    int type = -1;
    // . and ..
    if (name[0] == '.') {
        type = 2;
        if (name[1] == '.')
            type += 1;

        for (int i = 2; i < 11; i++) {
            if (name[i] != ' ')
                return 0;
        }
        return type;
    }
    // file and dir
    type = 1;
    for (int i = 0; i < 11; i++) {
        if (!((name[i] >= 'A' && name[i] <= 'Z')
              || (name[i] >= 'a' && name[i] <= 'z')
              || (name[i] >= '0' && name[i] <= '9')
              || name[i] == ' ' || name[i] == '_')) {
            type = 0;
            break;
        }
    }
    return type;
}


 /**
  * Read the fat12 recursively
  * 19 sectors in total
  * @param fat12
  * @param father
  * @param start
  * @param entry_cnt
  */
void LoadFiles(FILE *fat12, int father, int start, int entry_cnt) {
    int n = curFiles;
    files[father].sub_pos = curFiles;
    struct RootEntry ent;
    struct RootEntry *entry_ptr = &ent;
    for (int i = 0; i < entry_cnt; i++) { // go into each entry and read data
        fseek(fat12, start + ENTRY_SIZE * i, SEEK_SET);
        fread(entry_ptr, 1, ENTRY_SIZE, fat12);

        if (entry_ptr->classification == 0x00 || entry_ptr->classification == 0x10 || entry_ptr->classification == 0x20) {
            // 0x10:dir 0x20:file
            int type = TypeOf(entry_ptr->name);
            if (type == 0)
                continue;

            if (type == 2 || type == 3) { // . or .. type
                files[curFiles].name[0] = '.';
                files[curFiles].name[1] = 0x00;
                files[curFiles].fullname[0] = '.';
                files[curFiles].fullname[1] = 0x00;
                if (type == 3) {
                    files[curFiles].name[1] = '.';
                    files[curFiles].name[2] = 0x00;
                    files[curFiles].fullname[1] = '.';
                    files[curFiles].fullname[2] = 0x00;
                }
                files[curFiles].type = 0;
                files[curFiles].size = 0;
                files[curFiles].sub_pos = -1;
                files[curFiles].start_pos = 0;
            }// end if

            if(type == 1){
                //write the name of file or dir
                int t = (entry_ptr->classification == 0x10) ? 0 : 1; // 是否为文件？1为文件，0为目录
                int j;
                for (j = 0; j < 8; j++) {
                    if (entry_ptr->name[j] == ' ') {
                        break;
                    }
                    files[curFiles].name[j] = entry_ptr->name[j];
                }

                int pos = j;
                if (t) {
                    files[curFiles].name[pos++] = '.';
                    for (int j = 8; j < 11; j++) {
                        if (entry_ptr->name[j] != ' ') {
                            files[curFiles].name[pos++] = entry_ptr->name[j];
                        }
                    }
                }

                files[curFiles].name[pos] = 0x00;
                pos = 0;
                while (files[father].fullname[pos]) {
                    files[curFiles].fullname[pos] = files[father].fullname[pos];
                    pos++;
                }

                int cur = 0;
                while (files[curFiles].name[cur]) {
                    files[curFiles].fullname[pos++] = files[curFiles].name[cur++];
                }

                if (t == 0) {
                    files[curFiles].fullname[pos++] = '/';
                }
                files[curFiles].fullname[pos] = 0x00;

                //填充其他数据
                files[curFiles].size = t ? entry_ptr->fileSize : 0;
                files[curFiles].type = t;

                //计算文件夹起点和扇区号
                int begin = (Pre_Sec + Root_Sec + entry_ptr->fstCluster - 2) * Bytes_Per_Sector;
                files[curFiles].start_pos = begin;
                files[curFiles].cluster = entry_ptr->fstCluster;
            }

            curFiles++;
        }// end if
    }// end for

    DivideFiles();//set split node

    int m = curFiles;
    for (int i = n; i < m; i++) {
        if (files[i].type == 0 && files[i].start_pos != 0) {
            // 加载目录的子节点，但是要排除.和..
            LoadFiles(fat12, i, files[i].start_pos, Bytes_Per_Sector / ENTRY_SIZE); // 一般来说，最后一个参数等价于 512/32=16
        }
    }
}

/**
 * 读取扇区数据的过程抽象
 * @param fat12
 */
void DoLoad(FILE *fat12) {
    ReadBootData(fat12);
    InitFiles();
    LoadFiles(fat12, 0, Pre_Sec * Bytes_Per_Sector, Root_Entry_Cnt);
}

/**
 * Get the num-th FAT Cluster
 * @param fat12
 * @param num
 * @return byte
 */
int GetCluster(FILE *fat12, int num) {
    //基地址
    int fat_base = RsvSec * Bytes_Per_Sector;
    //偏移地址
    int fat_pos = fat_base + num * 3 / 2;
    //奇数和偶数FAT项分开处理
    int type = num % 2;

    u16 bytes;
    u16 *bytes_ptr = &bytes;
    int check;
    // read 2 bytes
    fseek(fat12, fat_pos, SEEK_SET);
    fread(bytes_ptr, 1, 2, fat12);

    // type: 0, 取低12位
    // type: 1, 取高12位
    if (type == 0) {
        return bytes & 0x0fff; //低12位
    } else {
        return bytes >> 4; // 高12位
    }
}

/**
 * 1. count the number of files and dirs begin from sub_pos
 * 2. print the number
 * @param sub_pos begin index
 */
void CountDirAndFile(int sub_pos) {
    int dir_n = 0;
    int file_n = 0;
    while (files[sub_pos].type != -1) {
        if (files[sub_pos].type == 0) { // is a dir
            if (files[sub_pos].sub_pos != -1) // not split node
                dir_n++;
        } else if (files[sub_pos].type == 1) {// is a file
            file_n++;
        }
        sub_pos++;
    }

    char *ws = " ";
    my_print(ws, 1);
    PrintNum(dir_n);
    my_print(ws, 1);
    PrintNum(file_n);
}

/**
 * Locate a file
 * @param p name ptr
 * @param size
 * @return index
 */
int Locate(char *file_ptr, int len) {
    int index = -1;
    for (int i = 0; i < curFiles; i++) {
        char *ptr = file_ptr;
        int j;
        for (j = 0; j < len; j++) {
            if (*ptr != files[i].fullname[j]) {
                break;
            }
            ptr++;
        }
        if (len == j) {
            index = i;
            break;
        }
    }
    return index;
}



/**************************** CORE FUNCTIONS *****************************/
/*************************************************************************/

/**
 * Do LS command
 * @param fpos father node position
 */
void DoLS(int fpos) {
    char *str = files[fpos].fullname;
    my_print(str, strlen(str));

    str = ":\n";
    my_print(str, strlen(str));

    int sub_pos = files[fpos].sub_pos;
    int l = sub_pos;
    while (files[sub_pos].type != -1) { // 遍历数组,直到文件分隔符
        char *name = files[sub_pos].name;
        if (files[sub_pos].type == 1) {
            // 输出文件名，白色
            my_print(name, strlen(name));
        } else if (files[sub_pos].type == 0) {
            // 输出目录
            set_color_red(); // 文件颜色：红
            my_print(name, strlen(name));
            set_default(); // 恢复颜色
        }
        if (files[sub_pos + 1].type != -1) {
            // 下一个不是分割，说明还有文件，那就要输出空格
            str = " ";
            my_print(str, 1);
        }
        sub_pos++;
    }
    PrintLn();
    for (int i = l; i < sub_pos; i++) {
        if (files[i].type == 0 && files[i].start_pos != 0) {
            DoLS(i);
        }
    }
}

/**
 * Do LS -L command
 * @param fpos father node position
 */
void DoLS_L(int fpos) {
    char *str = files[fpos].fullname; // str用来存储输入内容，可复用
    my_print(str, strlen(str));
    CountDirAndFile(files[fpos].sub_pos);
    str = ":\n";
    my_print(str, strlen(str));
    int sub_pos = files[fpos].sub_pos;
    int l = sub_pos;
    while (files[sub_pos].type != -1) {
        // 一直遍历，直到分割符
        char *name = files[sub_pos].name;
        if (files[sub_pos].type == 1) {
            // 输出文件名 大小，白色
            my_print(name, strlen(name));
            char *str = " ";
            my_print(str, 1);
            PrintNum(files[sub_pos].size);
        } else if (files[sub_pos].type == 0) {
            // 输出目录
            set_color_red(); // 文件颜色：红
            my_print(name, strlen(name));
            set_default(); // 恢复颜色
            if (files[sub_pos].sub_pos != -1) {
                // 需要排除.&..
                CountDirAndFile(files[sub_pos].sub_pos);
            }
        }
        PrintLn();
        sub_pos++;
    }
    for (int i = l; i < sub_pos; i++) {
        if (files[i].type == 0 && files[i].start_pos != 0) {
            PrintLn();
            DoLS_L(i);
        }
    }

}

/**
 * Do CAT command
 * @param fat12 file system
 * @param fpos father node position
 */
void DoCat(FILE *fat12, int fpos) {
    int start = files[fpos].start_pos;
    u16 nxtclst = GetCluster(fat12, files[fpos].cluster);
    int a = fseek(fat12, start, SEEK_SET);
    char str[1024 * 1024 * 4];
    int len = fread(str, 1, Bytes_Per_Sector, fat12);
    while (nxtclst < 0x0ff7) {
        // nxtclst != 0x0ff7 && nxtclst < 0x0ff8
        start = (Pre_Sec + Root_Sec + nxtclst - 2) * Bytes_Per_Sector;
        fseek(fat12, start, SEEK_SET);
        len += fread(str + len, 1, Bytes_Per_Sector, fat12);
        nxtclst = GetCluster(fat12, nxtclst);
    }
    my_print(str, len);
    PrintLn();
}



/******************** FUNCTIONS FOR RESOLVING COMMAND ********************/
/*************************************************************************/

char input[500];
int ptr = 0;
int l = 0; // 0:LS , 1:LS-L

/**
 * Read a command from the terminal
 * 0: error occur
 * 1: ls
 * 2: cat
 * -1: exit
 * @return type
 */
int ReadCommand() {
    fgets(input, 500, stdin);
    char cmd[10];
    int type; //-1 for exit, 1 for ls, and 2 for cat, a table-driven design
    while (input[ptr] != ' ' && input[ptr] != '\n' && input[ptr]) {
        cmd[ptr] = input[ptr];
        ptr++;
    } // 取第一个字段的命令
    cmd[ptr] = '\0';

    if (cmd[0] == 'l' && cmd[1] == 's') {
        type = 1;
    }
    else if (cmd[0] == 'c' && cmd[1] == 'a' && cmd[2] == 't') {
        type = 2;
    }
    else if (cmd[0] == 'e' && cmd[1] == 'x' && cmd[2] == 'i' && cmd[3] == 't') {
        type = -1;
    }
    else {
        char *warn = "Bad Command! Error:";
        PrintError(warn, strlen(warn), cmd, strlen(cmd));
        return 0; // 返回main，继续读取下一个指令
    }
    return type;
}

/**
 * Read the cmd
 * -l is only for ls
 * call the implements after parsing the inputs
 */
void LoadArgv(int cmd, FILE *fat12) {
    char dir[50];
    char tmp[50];
    int dnum = 0;   // dir number
    int ef = 0;     // error flag
    int erri = 0;   // error index
    int errn = 0;   // error length
    int curr = 0;   // cur length
    while (input[ptr] && input[ptr] != '\n') {

        if (input[ptr] == ' ') {
            ptr++;
            continue;
        }

        if (input[ptr] == '-') {
            // 输入选项
            erri = ptr;
            errn = 1;
            ptr++;
            if (cmd != 1) ef = 1;
            while (input[ptr] && input[ptr] != ' ' && input[ptr] != '\n') {
                if (input[ptr] != 'l') {
                    ef = 1;
                }
                ptr++;
                errn++;
            }
            //
            if (erri + 1 == ptr) {
                ef = 1;
            }//说明只输入了一个‘-’
            //
            if (!ef) {
                l = 1;
            } else {
                char *warn = "Bad Input! The Option not exist. Error:";
                PrintError(warn, strlen(warn), input + erri, errn);
                return;
            }
        } else {
            if (dnum > 0) {
                char *warn = "Only one file or directory is allowed!";
                char *str = "";
                PrintError(warn, strlen(warn), str, 0);
                return;
            } else {
                dnum += 1;
                while (input[ptr] && input[ptr] != ' ' && input[ptr] != '\n') {
                    tmp[curr++] = input[ptr++];
                }
            }
        }
        ptr++;
    }
    int start = -1;
    dir[0] = '/';
    if (!(tmp[0] == '.' && curr == 1)) {

        for (int i = 0; i < curr; i++) {
            if (!(tmp[i] == '.' && tmp[i + 1] == '/') && tmp[i] != '/') {
                start = i;
                break;
            }
        }
    }

    int dirlen = 1;

    if (start > -1) {
        for (int i = start; i < curr; i++) {
            dir[dirlen++] = tmp[i];
        }
    }

    if (cmd == 1) {
        //ls指令的目标是目录，所以需要补全目录，即保证以‘/’结尾
        if (curr > 0 && dir[dirlen - 1] != '/') {
            dir[dirlen++] = '/';
        }
    }
    dir[dirlen] = '\0'; // 保险起见加上终止符

    //实现额外的需求，忽略目录路径中的. 和 ..
    if (cmd == 1) {
        for (int i = 1; i < dirlen; i++) {
            if (dir[i] == '.') {
                dirlen = i;
            }
        }
    }

    int fpos = Locate(dir, dirlen);

    if (fpos == -1) {
        char *warn = "File Not found. Error:";
        PrintError(warn, strlen(warn), dir, dirlen);
        return;
    } else {
        char *err;

        switch (cmd) {

            case 2:
                if (files[fpos].type != 0) {
                    DoCat(fat12, fpos);
                    break;
                } else {
                    err = "Not a file:";
                    PrintError(err, strlen(err), dir, dirlen);
                    break;
                }
            case 1:
                if (l) {
                    DoLS_L(fpos);
                } else {
                    DoLS(fpos);
                }
                break;
            default:
                err = "A bad error has happened.";
                my_print(err, strlen(err));
                break;
        }
    }
}



/********************************* MAIN **********************************/
/*************************************************************************/
/**
 * 程序入口
 * Do the polling 轮询
 * @return
 */
int main() {
    FILE *fat12 = fopen("a.img", "rb"); // 打开文件系统
    DoLoad(fat12); // 导入文件，所有文件被填入数组数据结构
    while (1) {
        my_print(">", 1);
        ptr = 0; //读取新的输入时，游标归0
        l = 0;
        int code = ReadCommand();
        if (code == -1) {
            char *gb = "Goodbye!\n";
            my_print(gb, strlen(gb));
            break;
        }// the only way to break the loop
        // ==========================load arguments===================
        if (code) { // code == 0 means a error code, there is no need to read the left input
            LoadArgv(code, fat12); // 读入参数，根据输入参数调用不同的实现
        }
    }
    fclose(fat12);
}
