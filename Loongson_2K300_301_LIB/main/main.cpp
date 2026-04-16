#include "main.hpp"


void test()
{
    set_motor_pwm(1000,1000);//0.80对应·1000
    while(1)
    {
        
        usleep(100*1000);
        float rb=get_r_motor_speed();
        float lb=get_l_motor_speed();
        printf("lb: %.2f\r\n", rb);
        printf("rb: %.2f\r\n", lb);
    }
}





int main()
{
    //lq_udp_img_trans_demo();
    //test();
    test_ziji();
    //test_ziji();
    //lq_udp_img_trans_demo();
    while(1)
    {
        usleep(100*100);
    }

    return 0;
}
