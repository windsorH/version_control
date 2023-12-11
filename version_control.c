#include <stdio.h>
#include "version_control.h"
// 大版本号
static uint8_t major_version = 3;
// 中版本号
static uint8_t middle_version = 1;
// 小版本号
static uint8_t sub_version = 2;
uint32_t get_algo_version_uncode(void)
{

	uint32_t re = 0;
	// 大版本号保存在最高4位，中版本号保存在其次4位，小版本号保存在最后8位
	re = (((uint16_t)(major_version)) << 12) & 0xf000;
	re += (((uint16_t)(middle_version)) << 8)  & 0x0f00;
	re += ((uint16_t)(sub_version)) & 0x00ff;
	return re;
}
void main()
{
	// 获取版本号
	uint16_t re = get_algo_version_uncode();
	printf("version code: %d\n", re);
	// 输出get_algo_version_uncode()的细节版本号,例如 1.0.2
	printf("version code detail: %d.%d.%d\n", (re & 0xf000) >> 12 , (re & 0x0f00) >> 8 , (re & 0x00ff));
	while(1);
}
