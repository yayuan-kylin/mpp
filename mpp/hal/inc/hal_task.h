/*
*
* Copyright 2015 Rockchip Electronics Co. LTD
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __HAL_TASK__
#define __HAL_TASK__

#include "rk_mpi.h"

#define MAX_DEC_REF_NUM     17

typedef enum MppTaskStatus_e {
    TASK_IDLE,
    TASK_PREPARE,
    TASK_WAIT_PROC,
    TASK_PROCESSING,
    TASK_PROC_DONE,
    TASK_BUTT,
} MppTaskStatus;

/*
 * modified by parser
 *
 * number   : the number of the data pointer array element
 * data     : the address of the pointer array, parser will add its data here
 */
typedef struct MppSyntax_t {
    RK_U32              number;
    void                *data;
} MppSyntax;

/*
 *  HalTask memory layout:
 *
 *  +----^----+ +----------------------+ +----^----+
 *       |      |     context type     |      |
 *       |      +----------------------+      |
 *       +      |      coding type     |      |
 *     header   +----------------------+      |
 *       +      |         size         |      |
 *       |      +----------------------+      |
 *       |      |     pointer count    |      |
 *  +----v----+ +----------------------+      |
 *              |                      |      |
 *              |       pointers       |      |
 *              |                      |      +
 *              +----------------------+    size
 *              |                      |      +
 *              |        data_0        |      |
 *              |                      |      |
 *              +----------------------+      |
 *              |                      |      |
 *              |        data_1        |      |
 *              |                      |      |
 *              +----------------------+      |
 *              |                      |      |
 *              |                      |      |
 *              |        data_2        |      |
 *              |                      |      |
 *              |                      |      |
 *              +----------------------+ +----v----+
 */
typedef struct HalDecTask_t {
    // set by parser to signal that it is valid
    RK_U32          valid;

    // current tesk protocol syntax information
    MppSyntax       syntax;

    MppBuffer       stmbuf;

    // for test purpose
    // current tesk output slot index
    RK_S32          output;
    // current task reference slot index, -1 for unused
    RK_S32          refer[MAX_DEC_REF_NUM];
} HalDecTask;

typedef struct HalEncTask_t {
    RK_U32          valid;

    // current tesk protocol syntax information
    MppSyntax       syntax;

    // current tesk output stream buffer index
    RK_S32          output;

    // current tesk input slot buffer index
    RK_S32          input;
    // current task reference index, -1 for unused
    RK_S32          refer;
    // current task recon index
    RK_S32          recon;
} HalEncTask;

typedef union HalTask_u {
    HalDecTask      dec;
    HalEncTask      enc;
} HalTaskInfo;

typedef void* HalTaskHnd;
typedef void* HalTaskGroup;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * group init / deinit will be called by hal
 *
 * NOTE: use mpp_list to implement
 *       the count means the max task waiting for process
 */
MPP_RET hal_task_group_init(HalTaskGroup *group, MppCtxType type, RK_S32 count);
MPP_RET hal_task_group_deinit(HalTaskGroup group);

/*
 * normal working flow:
 *
 * dec:
 *
 * - codec
 * hal_task_get_hnd(group, idle, hnd)       - dec try get idle task to work
 * hal_task_hnd_set_status(hnd, prepare)    - dec prepare the task
 * codec prepare task
 * hal_task_hnd_set_status(hnd, wait_proc)  - dec send the task to hardware queue
 *
 * - hal
 * hal_task_get_hnd(group, wait_proc, hnd)  - hal get task on wait_proc status
 * hal start task
 * hal_task_set_hnd(hnd, processing)        - hal send task to hardware for process
 * hal wait task done
 * hal_task_set_hnd(hnd, proc_done)         - hal mark task is finished
 *
 * - codec
 * hal_task_get_hnd(group, task_done, hnd)  - codec query the previous finished task
 * codec do error process on task
 * hal_task_set_hnd(hnd, idle)              - codec mark task is idle
 *
 */
MPP_RET hal_task_get_hnd(HalTaskGroup group, MppTaskStatus status, HalTaskHnd *hnd);
MPP_RET hal_task_hnd_set_status(HalTaskHnd hnd, MppTaskStatus status);
MPP_RET hal_task_hnd_set_info(HalTaskHnd hnd, HalTaskInfo *task);
MPP_RET hal_task_hnd_get_info(HalTaskHnd hnd, HalTaskInfo *task);
MPP_RET hal_task_info_init(HalTaskInfo *task, MppCtxType type);

#ifdef __cplusplus
}
#endif

#endif /*__HAL_TASK__*/

