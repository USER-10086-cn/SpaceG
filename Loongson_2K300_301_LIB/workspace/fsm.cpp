/**
 * @file fsm.c
 * @brief 有限状态机实现
 * 
 * 该文件实现了有限状态机(FSM)的基本功能，包括状态机的构造、初始化和事件分发。
 */

#include "fsm.hpp"

/**
 * @brief 构造状态机
 * 
 * 初始化状态机对象，设置初始状态处理函数。
 * 
 * @param self 状态机对象指针
 * @param initial 初始状态处理函数
 */
void fsm_ctor(fsm_t *self, state_handler_t initial)
{
    self->state = initial;
}

/**
 * @brief 初始化状态机
 * 
 * 触发状态机进入初始状态，首先通过传入的事件确保状态机转换到默认状态，
 * 然后触发进入状态事件。
 * 
 * @param self 状态机对象指针
 * @param event 用于初始化的事件
 */
void fsm_init(fsm_t *self, event_t event)
{
    (*self->state)(self, event);       /* 必须转换到默认状态 */
    (*self->state)(self, EVENT_STATE_ENTER); /* 进入默认状态 */
}

/**
 * @brief 分发事件到当前状态
 * 
 * 将事件分发到当前状态处理函数，如果处理函数返回状态转换标志，
 * 则执行状态退出和进入操作。
 * 
 * @param self 状态机对象指针
 * @param event 要分发的事件
 */
void fsm_dispatch(fsm_t *self, event_t event)
{
    state_handler_t prev_state = self->state;
    status_t status = (*self->state)(self, event);

    if (status == STATUS_TRAN)
    {
        (*prev_state)(self, EVENT_STATE_EXIT);
        (*self->state)(self, EVENT_STATE_ENTER);
    }
}
