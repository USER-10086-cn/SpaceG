/**
 * @file fsm.h
 * @brief 有限状态机(FSM)头文件
 * 
 * 该头文件定义了有限状态机所需的数据类型、事件和状态枚举，
 * 以及状态机结构体和相关函数原型。
 */

#ifndef __FSM_H__
#define __FSM_H__

/**
 * @brief 事件类型枚举
 * 
 * 定义了状态机中使用的标准事件类型。
 */
enum {
    EVENT_STATE_INIT,       /**< 状态初始化事件 */
    EVENT_STATE_ENTER,      /**< 进入状态事件 */
    EVENT_STATE_EXIT,       /**< 退出状态事件 */
    EVENT_USER,             /**< 用户自定义事件起始值 */
};

/**
 * @brief 状态处理返回值枚举
 * 
 * 定义了状态处理函数可能返回的状态值。
 */
enum {
    STATUS_TRAN,            /**< 状态转换标志 */
    STATUS_HANDLED,         /**< 事件已处理标志 */
};

/**
 * @brief 简化状态转换代码的宏，设置新状态并返回状态转换标志。
 */
#define TRAN_TO(target) (((fsm_t *)self)->state = (state_handler_t)(target), STATUS_TRAN)

/**
 * @brief 状态处理函数返回值类型
 */
typedef unsigned char status_t;

/**
 * @brief 事件类型定义
 */
typedef unsigned int event_t;

/**
 * @brief 状态机类型前向声明
 */
typedef struct fsm fsm_t;

/**
 * @brief 状态处理函数指针类型
 * 
 * @param self 状态机对象指针
 * @param event 触发的事件
 * @return 状态处理结果，可以是STATUS_TRAN或STATUS_HANDLED
 */
typedef status_t (*state_handler_t)(fsm_t * self, event_t event);

/**
 * @brief 有限状态机结构体
 * 
 * 包含状态机当前状态处理函数的结构体。
 */
struct fsm {
    state_handler_t state;  /**< 当前状态处理函数指针 */
};

/**
 * @brief 构造状态机
 * 
 * 初始化状态机对象，设置初始状态处理函数。
 * 
 * @param self 状态机对象指针
 * @param initial 初始状态处理函数
 */
void fsm_ctor(fsm_t *self, state_handler_t initial);

/**
 * @brief 初始化状态机
 * 
 * 触发状态机进入初始状态，首先通过传入的事件确保状态机转换到默认状态，
 * 然后触发进入状态事件。
 * 
 * @param self 状态机对象指针
 * @param event 用于初始化的事件
 */
void fsm_init(fsm_t *self, event_t event);

/**
 * @brief 分发事件到当前状态
 * 
 * 将事件分发到当前状态处理函数，如果处理函数返回状态转换标志，
 * 则执行状态退出和进入操作。
 * 
 * @param self 状态机对象指针
 * @param event 要分发的事件
 */
void fsm_dispatch(fsm_t *self, event_t event);

#endif /* __FSM_H__ */
