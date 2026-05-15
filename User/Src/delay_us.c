#include "delay_us.h"

void delay_us(uint32_t us)
{
    // ��ʼ���ӳ�ʱ������Ϊ0
    uint32_t total = 0;
 
    // ������Ҫ�ӳٵ�ʱ��(��΢��Ϊ��λ)
    uint32_t target = (SystemCoreClock/1000000U) * us;
 
    // ��ȡ��ʼ��SysTick����ֵ
    int last = SysTick->VAL;
    int now = last;
    int diff = 0;
 
    // ѭ��ֱ���ۼƵ��ӳ�ʱ��ﵽĿ��ʱ��
    while(1)
    {
        
        now = SysTick->VAL;// ��ȡ��ǰ��SysTick����ֵ
        
        diff = last - now;// �������ζ�ȡ֮��Ĳ�ֵ
 
        if(diff > 0)// �����ֵ����0,��ʾʱ��������
        {
          
            total += diff;// �������ӳ�ʱ��
        }
        else
        {
            total += diff + SysTick->LOAD;// �����ֵС��0,��ʾʱ���м���,��Ҫ����������ֵ
        }
 
        if(total > target)// ������ӳ�ʱ�����Ŀ��ʱ��,���˳�ѭ��
        {
            return;
        }
 
        last = now;// �����ϴζ�ȡ��SysTick����ֵ
    }
}
