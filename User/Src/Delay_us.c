#include "Delay_us.h"

void Delay_us(uint32_t us)
{
    // 初始化延迟时间总数为0
    uint32_t total = 0;
 
    // 计算需要延迟的时间(以微秒为单位)
    uint32_t target = (SystemCoreClock/1000000U) * us;
 
    // 获取初始的SysTick计数值
    int last = SysTick->VAL;
    int now = last;
    int diff = 0;
 
    // 循环直到累计的延迟时间达到目标时间
    while(1)
    {
        
        now = SysTick->VAL;// 获取当前的SysTick计数值
        
        diff = last - now;// 计算两次读取之间的差值
 
        if(diff > 0)// 如果差值大于0,表示时间有增加
        {
          
            total += diff;// 增加总延迟时间
        }
        else
        {
            total += diff + SysTick->LOAD;// 如果差值小于0,表示时间有减少,需要加上最大计数值
        }
 
        if(total > target)// 如果总延迟时间大于目标时间,则退出循环
        {
            return;
        }
 
        last = now;// 更新上次读取的SysTick计数值
    }
}
