/**
 * @file queue.c
 * @brief 通用队列实现源文件
 */

#include "queue.hpp"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 链表节点结构体
 */
typedef struct queue_node {
    void* data;                  /**< 数据指针 */
    struct queue_node* next;     /**< 下一个节点指针 */
} queue_node_t;

/**
 * @brief 队列结构体
 */
struct queue {
    queue_node_t* head;          /**< 队列头指针 */
    queue_node_t* tail;          /**< 队列尾指针 */
    uint16_t count;              /**< 当前项目数量 */
    uint16_t item_size;          /**< 单个项目的大小 */
};

/**
 * @brief 创建队列
 * 
 * @param item_size 单个队列项目的大小（字节）
 * @return queue_handle_t 创建的队列句柄，失败时返回NULL
 */
queue_handle_t queue_create(uint16_t item_size)
{
    if (item_size == 0) {
        return NULL;
    }

    /* 分配队列内存 */
    queue_handle_t queue = (queue_handle_t)malloc(sizeof(struct queue));
    if (queue == NULL) {
        return NULL;
    }

    /* 初始化队列 */
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    queue->item_size = item_size;

    return queue;
}

/**
 * @brief 删除队列并释放资源
 * 
 * @param queue 要删除的队列句柄
 */
void queue_delete(queue_handle_t queue)
{
    if (queue == NULL) {
        return;
    }

    /* 释放所有链表节点 */
    queue_node_t* current = queue->head;
    while (current != NULL) {
        queue_node_t* next = current->next;

        /* 释放数据 */
        if (current->data != NULL) {
            free(current->data);
        }

        /* 释放节点 */
        free(current);
        current = next;
    }

    /* 释放队列结构体 */
    free(queue);
}

/**
 * @brief 向队列发送项目
 * 
 * @param queue 队列句柄
 * @param item 要发送的项目指针
 * @return true 发送成功
 * @return false 发送失败
 */
bool queue_send(queue_handle_t queue, const void* item)
{
    if (queue == NULL || item == NULL) {
        return false;
    }

    /* 创建新节点 */
    queue_node_t* new_node = (queue_node_t*)malloc(sizeof(queue_node_t));
    if (new_node == NULL) {
        return false;  /* 内存分配失败 */
    }

    /* 分配项目数据内存 */
    new_node->data = malloc(queue->item_size);
    if (new_node->data == NULL) {
        free(new_node);
        return false;  /* 内存分配失败 */
    }

    /* 复制项目数据 */
    memcpy(new_node->data, item, queue->item_size);
    new_node->next = NULL;

    /* 添加到链表尾部 */
    if (queue->tail == NULL) {
        /* 队列为空 */
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }

    queue->count++;
    return true;
}

/**
 * @brief 从队列接收项目
 * 
 * @param queue 队列句柄
 * @param item 用于存储接收项目的缓冲区
 * @return true 接收成功
 * @return false 接收失败
 */
bool queue_receive(queue_handle_t queue, void* item)
{
    if (queue == NULL || item == NULL || queue_is_empty(queue)) {
        return false;
    }

    /* 获取头节点 */
    queue_node_t* node = queue->head;

    /* 复制项目数据 */
    memcpy(item, node->data, queue->item_size);

    /* 更新头指针 */
    queue->head = node->next;
    if (queue->head == NULL) {
        /* 队列已空 */
        queue->tail = NULL;
    }

    /* 释放项目数据和节点 */
    free(node->data);
    free(node);
    queue->count--;

    return true;
}

/**
 * @brief 检查队列是否为空
 * 
 * @param queue 队列句柄
 * @return true 队列为空
 * @return false 队列不为空
 */
bool queue_is_empty(queue_handle_t queue)
{
    if (queue == NULL) {
        return true;
    }

    return queue->count == 0;
}

/**
 * @brief 获取队列中当前项目数量
 * 
 * @param queue 队列句柄
 * @return uint16_t 队列中的项目数量
 */
uint16_t queue_count(queue_handle_t queue)
{
    if (queue == NULL) {
        return 0;
    }

    return queue->count;
}
