/**
 * @file queue.h
 * @brief 通用队列实现头文件
 */

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 队列句柄声明
 * 
 * 这是一个不透明指针，用于访问队列实例
 */
typedef struct queue* queue_handle_t;

/**
 * @brief 创建队列
 * 
 * @param item_size 单个队列项目的大小（字节）
 * @return queue_handle_t 创建的队列句柄，失败时返回NULL
 */
queue_handle_t queue_create(uint16_t item_size);

/**
 * @brief 删除队列并释放资源
 * 
 * @param queue 要删除的队列句柄
 */
void queue_delete(queue_handle_t queue);

/**
 * @brief 向队列发送项目
 * 
 * @param queue 队列句柄
 * @param item 要发送的项目指针
 * @return true 发送成功
 * @return false 发送失败
 */
bool queue_send(queue_handle_t queue, const void* item);

/**
 * @brief 从队列接收项目
 * 
 * @param queue 队列句柄
 * @param item 用于存储接收项目的缓冲区
 * @return true 接收成功
 * @return false 接收失败
 */
bool queue_receive(queue_handle_t queue, void* item);

/**
 * @brief 检查队列是否为空
 * 
 * @param queue 队列句柄
 * @return true 队列为空
 * @return false 队列不为空
 */
bool queue_is_empty(queue_handle_t queue);

/**
 * @brief 获取队列中当前项目数量
 * 
 * @param queue 队列句柄
 * @return uint16_t 队列中的项目数量
 */
uint16_t queue_count(queue_handle_t queue);

#endif /* __QUEUE_H__ */
