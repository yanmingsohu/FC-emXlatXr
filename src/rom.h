#ifndef ROM_H_INCLUDED
#define ROM_H_INCLUDED

#include "type.h"
#include "string"

using std::string;

/**
 * 读取filename文件到rom结构中,成功返回0并设置rom参数
 * rom参数必须是有效的, 失败返回-1
 */
int load_rom(nes_file* rom, string* filename);

#endif // ROM_H_INCLUDED
