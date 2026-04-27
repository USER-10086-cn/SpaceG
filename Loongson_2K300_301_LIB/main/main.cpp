#include "main.hpp"








// void test()
// {
//     set_motor_pwm(1000,1000);//0.80对应·1000
//     while(1)
//     {
        
//         usleep(100*1000);
//         float rb=get_r_motor_speed();
//         float lb=get_l_motor_speed();
//         printf("lb: %.2f\r\n", rb);
//         printf("rb: %.2f\r\n", lb);
//     }
// }





// int main()
// {
//     //lq_udp_img_trans_demo();
//     //test();
//     test_ziji();
//     //test_ziji();
//     //lq_udp_img_trans_demo();
//     while(1)
//     {
//         usleep(100*100);
//     }

//     return 0;
// }



/*
 * 文件名: main.c
 * 描述: 状态机配合消息队列的示例
 * 平台: STM32F10x
 * 功能: 展示了状态机与消息队列结合使用的实现
 * 特点: 使用消息队列解耦事件产生和处理，提高系统响应性和模块化
 * 优点: 1. 事件产生和处理解耦，提高系统灵活性
 *       2. 支持事件缓存，避免事件丢失
 *       3. 便于实现多任务环境下的状态机
 *       4. 简化状态机的事件处理流程
 */

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>


ls_gpio gpio(PIN_44, GPIO_MODE_IN);



/* 用户自定义事件枚举，从EVENT_USER开始定义 */
enum
{
    EVENT_GOTO_STATE1 = EVENT_USER, // 切换到状态1事件
    EVENT_GOTO_STATE2,              // 切换到状态2事件
    EVENT_GOTO_STATE3,              // 切换到状态3事件
    EVENT_RUN_CURRENT,              // 执行当前状态事件
    EVENT_RUN_STATE1_ONLY,          // 仅限状态1执行事件
    EVENT_RUN_STATE2_ONLY,          // 仅限状态2执行事件
    EVENT_RUN_STATE3_ONLY,          // 仅限状态3执行事件
};

/* 全局状态机实例 */
static fsm_t g_fsm;
/* 全局事件队列，用于缓存事件 */
static queue_handle_t g_event_queue;

/* 状态处理函数声明 */
static status_t handle_state_init(fsm_t *self, event_t event);
static status_t handle_state1(fsm_t *self, event_t event);
static status_t handle_state2(fsm_t *self, event_t event);
static status_t handle_state3(fsm_t *self, event_t event);

/**
 * @brief 处理队列中的事件
 *
 * 该函数从事件队列中取出所有事件，并逐一分发给状态机处理。
 * 这是状态机与消息队列结合使用的核心函数，实现了事件的生产和消费解耦。
 *
 * 处理流程：
 * 1. 检查事件队列是否为空
 * 2. 如果不为空，取出队列头部的事件
 * 3. 将事件分发给状态机处理
 * 4. 重复上述过程直到队列为空
 *
 * 注意事项：
 * - 该函数应该在主循环或适当的地方被调用
 * - 事件处理顺序与入队顺序一致(FIFO)
 * - 如果队列满，新事件将被丢弃
 */
static void process_events(void)
{
    event_t event;

    /* 处理队列中的所有事件，直到队列为空 */
    while (!queue_is_empty(g_event_queue))
    {
        /* 从队列中取出事件 */
        if (queue_receive(g_event_queue, &event))
        {
            /* 将事件分发给状态机处理 */
            fsm_dispatch(&g_fsm, event);
        }
    }
}

/**
 * @brief 添加事件到队列
 *
 * 该函数将事件添加到事件队列中，供后续处理。
 * 这是事件生产者调用的接口，实现了事件产生和处理的解耦。
 *
 * @param event 要添加的事件
 *
 * 使用场景：
 * - 中断服务程序中添加事件
 * - 其他任务或模块中添加事件
 * - 定时器回调中添加事件
 *
 * 注意事项：
 * - 如果队列满，事件将被丢弃
 * - 该函数可以被多个调用者调用
 * - 事件处理顺序与添加顺序一致(FIFO)
 */
static void add_event_to_queue(event_t event)
{
    /* 将事件添加到队列尾部 */
    queue_send(g_event_queue, &event);
}

/**
 * @brief 主函数
 *
 * 程序入口函数，负责初始化硬件、状态机和消息队列，
 * 并演示状态机与消息队列结合使用的效果。
 *
 * 初始化流程：
 * 1. 硬件初始化
 * 2. 创建事件队列
 * 3. 构造状态机
 * 4. 初始化状态机
 *
 * 测试场景：
 * 1. 场景1：测试在状态1下的各种事件
 * 2. 场景2：测试从状态1切换到状态2
 * 3. 场景3：测试从状态2切换到状态3
 *
 * @return int 程序退出码(嵌入式系统通常不返回)
 */
int main(void)
{
    /* 创建事件队列，大小为事件类型的大小 */
    g_event_queue = queue_create(sizeof(event_t));
    /* 构造状态机，设置初始状态处理函数 */
    fsm_ctor(&g_fsm, handle_state_init);
    /* 初始化状态机，触发状态初始化流程 */
    fsm_init(&g_fsm, EVENT_STATE_INIT);
    vision_init();


    /* 主循环 */
    while (1)
    {
        process_events();  
    }
}

/**
 * @brief 状态机的初始化函数
 *
 * 该函数是状态机的初始状态处理函数，负责将状态机转换到状态1。
 * 这是状态机启动时调用的第一个状态处理函数。
 *
 * @param self 状态机对象指针
 * @param event 触发的事件
 * @return status_t 处理结果，总是返回状态转换标志
 *
 * 设计思路：
 * - 简单直接，只负责将状态机转换到状态1
 * - 使用TRAN_TO宏简化状态转换代码
 * - 不处理任何其他事件，保持简洁
 */
static status_t handle_state_init(fsm_t *self, event_t event)
{
    /* 转换到状态1 */
    return TRAN_TO(handle_state1);
}

/**
 * @brief 状态1的处理函数
 *
 * 该函数处理状态1下的所有事件，包括状态进入/退出、状态切换和业务逻辑执行。
 *
 * @param self 状态机对象指针
 * @param event 触发的事件
 * @return status_t 处理结果，可能是状态转换标志或事件已处理标志
 *
 * 事件处理：
 * - EVENT_STATE_ENTER: 状态进入时执行初始化操作
 * - EVENT_STATE_EXIT: 状态退出时执行清理操作
 * - EVENT_GOTO_STATE1: 已经是状态1，无需切换
 * - EVENT_GOTO_STATE2: 切换到状态2
 * - EVENT_GOTO_STATE3: 切换到状态3
 * - EVENT_RUN_CURRENT: 执行当前状态的业务逻辑
 * - EVENT_RUN_STATE1_ONLY: 执行状态1专属操作
 * - EVENT_RUN_STATE2_ONLY/EVENT_RUN_STATE3_ONLY: 在状态1下执行其他状态专属操作，失败
 */
static status_t handle_state1(fsm_t *self, event_t event)
{
    /* 默认返回事件已处理标志 */
    status_t ret = STATUS_HANDLED;

    switch (event)
    {
    case EVENT_STATE_ENTER:
        add_event_to_queue(EVENT_RUN_CURRENT);
        break;

    case EVENT_STATE_EXIT:
        break;

    case EVENT_GOTO_STATE1:
        break;

    case EVENT_GOTO_STATE2:
        ret = TRAN_TO(handle_state2);
        break;

    case EVENT_GOTO_STATE3:
        ret = TRAN_TO(handle_state3);
        break;

    case EVENT_RUN_CURRENT:
        vision_process_image();
        if(gpio.gpio_level_get()==0)       // 按键判断函数
        {
            add_event_to_queue(EVENT_GOTO_STATE2);  // 立刻切到 STATE2
            break;
        }
        vision_follow_center_line();
        add_event_to_queue(EVENT_RUN_CURRENT);
        break;

    case EVENT_RUN_STATE1_ONLY:
        break;

    case EVENT_RUN_STATE2_ONLY:
    case EVENT_RUN_STATE3_ONLY:
        break;
    }

    return ret;
}

/**
 * @brief 状态2的处理函数
 *
 * 该函数处理状态2下的所有事件，包括状态进入/退出、状态切换和业务逻辑执行。
 *
 * @param self 状态机对象指针
 * @param event 触发的事件
 * @return status_t 处理结果，可能是状态转换标志或事件已处理标志
 *
 * 事件处理：
 * - EVENT_STATE_ENTER: 状态进入时执行初始化操作
 * - EVENT_STATE_EXIT: 状态退出时执行清理操作
 * - EVENT_GOTO_STATE1: 切换到状态1
 * - EVENT_GOTO_STATE2: 已经是状态2，无需切换
 * - EVENT_GOTO_STATE3: 切换到状态3
 * - EVENT_RUN_CURRENT: 执行当前状态的业务逻辑
 * - EVENT_RUN_STATE2_ONLY: 执行状态2专属操作
 * - EVENT_RUN_STATE1_ONLY/EVENT_RUN_STATE3_ONLY: 在状态2下执行其他状态专属操作，失败
 */
static status_t handle_state2(fsm_t *self, event_t event)
{
    /* 默认返回事件已处理标志 */
    status_t ret = STATUS_HANDLED;

    switch (event)
    {
    case EVENT_STATE_ENTER:
        print_camera();
        add_event_to_queue(EVENT_RUN_CURRENT);

        break;

    case EVENT_STATE_EXIT:
        break;

    case EVENT_GOTO_STATE1:
        ret = TRAN_TO(handle_state1);
        break;

    case EVENT_GOTO_STATE2:
        break;

    case EVENT_GOTO_STATE3:
        ret = TRAN_TO(handle_state3);
        break;

    case EVENT_RUN_CURRENT:
        if(gpio.gpio_level_get()==1)       // 按键判断函数
        {
            add_event_to_queue(EVENT_GOTO_STATE1);  // 立刻切到 STATE2
            break;
        }
        add_event_to_queue(EVENT_RUN_CURRENT);
        break;

    case EVENT_RUN_STATE2_ONLY:
        break;

    case EVENT_RUN_STATE1_ONLY:
    case EVENT_RUN_STATE3_ONLY:
        break;
    }

    return ret;
}

/**
 * @brief 状态3的处理函数
 *
 * 该函数处理状态3下的所有事件，包括状态进入/退出、状态切换和业务逻辑执行。
 *
 * @param self 状态机对象指针
 * @param event 触发的事件
 * @return status_t 处理结果，可能是状态转换标志或事件已处理标志
 *
 * 事件处理：
 * - EVENT_STATE_ENTER: 状态进入时执行初始化操作
 * - EVENT_STATE_EXIT: 状态退出时执行清理操作
 * - EVENT_GOTO_STATE1: 切换到状态1
 * - EVENT_GOTO_STATE2: 切换到状态2
 * - EVENT_GOTO_STATE3: 已经是状态3，无需切换
 * - EVENT_RUN_CURRENT: 执行当前状态的业务逻辑
 * - EVENT_RUN_STATE3_ONLY: 执行状态3专属操作
 * - EVENT_RUN_STATE1_ONLY/EVENT_RUN_STATE2_ONLY: 在状态3下执行其他状态专属操作，失败
 */
static status_t handle_state3(fsm_t *self, event_t event)
{
    /* 默认返回事件已处理标志 */
    status_t ret = STATUS_HANDLED;

    switch (event)
    {
    case EVENT_STATE_ENTER:
        break;

    case EVENT_STATE_EXIT:
        break;

    case EVENT_GOTO_STATE1:
        ret = TRAN_TO(handle_state1);
        break;

    case EVENT_GOTO_STATE2:
        ret = TRAN_TO(handle_state2);
        break;

    case EVENT_GOTO_STATE3:
        break;

    case EVENT_RUN_CURRENT:
        break;

    case EVENT_RUN_STATE3_ONLY:
        break;

    case EVENT_RUN_STATE1_ONLY:
    case EVENT_RUN_STATE2_ONLY:
        break;
    }

    return ret;
}