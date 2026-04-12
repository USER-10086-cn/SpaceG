#include "lq_all_demo.hpp"
#include "lq_udp_client.hpp"   // 必须加
#include <iostream>
#include <vector>
#include <cmath>
#include <stdint.h>
#include "all.h"
// 🔥🔥🔥 定义全局 UDP 客户端（你缺失的关键！）
// 把 192.168.xxx.xxx 换成你接收端电脑的真实IP
lq_udp_client udp_client("192.168.86.126", 8080);
// =====================================================
// 配置参数 - 根据需要修改
// =====================================================
// 目标IP地址（UDP接收端）
const std::string TARGET_IP    = "192.168.86.126";
// UDP目标端口
const uint16_t    TARGET_PORT  = 8080;
// 摄像头参数
const uint16_t    CAM_WIDTH    = 80;     // 宽
const uint16_t    CAM_HEIGHT   = 60;     // 高
const uint16_t    CAM_FPS      = 30;     // 帧率
// 方块参数（画面中间）
const uint16_t    RECT_SIZE    = 20;
// JPEG编码质量 (1-100)
const uint8_t     JPEG_QUALITY = 30;
// 强制设置摄像头参数，别用默认！

/********************************************************************************
 * @brief   UDP 图像传输测试.
 * @param   none.
 * @return  none.
 * @note    测试内容为 UDP 图像传输，使用OpenCV 读取摄像头图像，并使用 UDP 发送图像数据.
 * @note    使用时需搭配对应上位机 LoongHost.exe.
 ********************************************************************************/

void get_gray_fast(lq_camera& cam, uchar (*out_image)[CAM_WIDTH])
{
    // 1. 拿灰度图（零拷贝、零克隆）
    cv::Mat& gray = cam.get_gray_ref();
    // 2. 按行复制（和你原来 mt9v03x 取图方式一样）
    for (int i = 0; i < CAM_HEIGHT; i++)
    {
        // 取一行地址 → 和你 demo 思路完全一样！
        uchar* row_ptr = gray.ptr<uchar>(i);

        // 整行复制 最快！
        memcpy(out_image[i], row_ptr, CAM_WIDTH);
    }
}



void show_array_image(uchar img[60][80])
{
    // 1. 把二维数组转成 OpenCV 的 Mat
    cv::Mat mat(60, 80, CV_8UC1, img);
    ssize_t sent = udp_client.udp_send_image(mat, JPEG_QUALITY);
    if (sent < 0) {
        printf("ERROR: Failed to send image\r\n");
    }
    
}




#define IMAGE_H 60
#define IMAGE_W 80
#define threshold_max  (255*5)  // 补断线阈值
#define threshold_min  (255*2)  // 去噪点阈值
#define border_min  5    // 左边不要太靠边
#define border_max  75   // 右边不要太靠边
uchar flag_yuanhuan=0;
uchar flag_shizi=0;




unsigned char otsu_binary0(uchar (*gray_array)[IMAGE_W])
{
    unsigned int huidu[256] = {0};
    unsigned char YUZHI = 0;

    // ==================== 统计直方图（指针加速） ====================
    for (int i = 0; i < IMAGE_H; i++) {
        uchar* row = gray_array[i];  // 取行指针（加速）
        for (int j = 0; j < IMAGE_W; j++) {
            huidu[row[j]]++;
        }
    }

    // ==================== 找第一高峰 ====================
    unsigned int H1 = 0;
    unsigned char D1 = 0;
#define KUAN 30

    for (int i = 0; i < 256; i++) {
        if (huidu[i] > H1) {
            H1 = huidu[i];
            D1 = i;
        }
    }

    // ==================== 找第二高峰 ====================
    unsigned int H2 = 0;
    unsigned char D2 = 0;
    bool OK = false;

    for (int i = H1 - 5; i > 0 && !OK; i -= 5) {
        for (int j = 0; j < 256; j++) {
            if (huidu[j] > i && abs(j - D1) > KUAN) {
                H2 = i;
                D2 = j;
                OK = true;
                break;
            }
        }
    }

    // ==================== 找山谷（阈值） ====================
    if (OK) {
        unsigned int H3 = 4800;
        unsigned char D3 = 0;

        if (D1 < D2) {
            for (int i = D1; i < D2; i++) {
                if (huidu[i] < H3) {
                    H3 = huidu[i];
                    D3 = i;
                }
            }
        } else {
            for (int i = D2; i < D1; i++) {
                if (huidu[i] < H3) {
                    H3 = huidu[i];
                    D3 = i;
                }
            }
        }
        YUZHI = D3;
    } else {
        YUZHI = 100; // 没找到就给默认值
    }
    return YUZHI;

    
}




void otsu_binary(uchar (*gray_array)[IMAGE_W],uchar YUZHI)
{
    for (int i = 0; i < IMAGE_H; i++) {
        uchar* row = gray_array[i];  // 行指针
        for (int j = 0; j < IMAGE_W; j++) {
            row[j] = (row[j] < YUZHI) ? 0 : 255;
        }
    }
}





//功能：将输入的二值数组进行补线和滤波
void image_filter(uint8_t (*bin_image)[IMAGE_W])
{
    uint16_t i, j;
    uint32_t sum;

    // 只遍历内部像素（避免越界）
    for (i = 1; i < IMAGE_H - 1; i++)
    {
        // 取当前行 上一行 下一行 指针（加速核心！）
        uint8_t *prev = bin_image[i-1];
        uint8_t *curr = bin_image[i];
        uint8_t *next = bin_image[i+1];

        for (j = 1; j < IMAGE_W - 1; j++)
        {
            // 纯指针访问 8 邻域，速度比数组下标快非常多
            sum = prev[j-1] + prev[j] + prev[j+1]
                + curr[j-1] +          curr[j+1]
                + next[j-1] + next[j] + next[j+1];

            // 补断线：周围很白，自己是黑 → 变白
            if (sum >= threshold_max && curr[j] == 0)
                curr[j] = 255;

            // 去噪点：周围很黑，自己是白 → 变黑
            if (sum <= threshold_min && curr[j] == 255)
                curr[j] = 0;
        }
    }
}





void image_draw_rectan(uint8_t (*image)[IMAGE_W])
{
    uint8_t i;
    for(i=0;i<IMAGE_H;i++)
    {
        image[i][0] = 0;
        image[i][1] = 0;
        image[i][IMAGE_W-1] =0;
        image[i][IMAGE_W-2] =0;
    }
    for(i=0;i<IMAGE_W;i++)
    {
        image[0][i] =0;
        image[1][i] =0;
    }
}






/**
 * @brief 计算两点直线，仅更新边界数组：border[y] = x
 * @param start     起点数组 {x, y}
 * @param end       终点数组 {x, y}
 * @param border    边界数组首地址（y为索引，存储对应x）
 */
void line_to_border(
                    const uint16_t start[2],
                    const uint16_t end[2],
                    uint8_t *border)
{
    int x1 = (int)start[0];
    int y1 = (int)start[1];
    int x2 = (int)end[0];
    int y2 = (int)end[1];

    int dx = x2 - x1;
    int dy = y2 - y1;

    int stepX = dx >= 0 ? 1 : -1;
    int stepY = dy >= 0 ? 1 : -1;

    dx = dx >= 0 ? dx : -dx;
    dy = dy >= 0 ? dy : -dy;

    int x = x1;
    int y = y1;

    if (dx > dy)
    {
        int err = 2 * dy - dx;
        for (int i = 0; i <= dx; i++)
        {
            // 只更新边界：border[y] = x
            *(border + y) = (uint8_t)x;

            if (err >= 0)
            {
                y += stepY;
                err -= 2 * dx;
            }
            x += stepX;
            err += 2 * dy;
        }
    }
    else
    {
        int err = 2 * dx - dy;
        for (int i = 0; i <= dy; i++)
        {
            // 只更新边界
            *(border + y) = (uint8_t)x;

            if (err >= 0)
            {
                x += stepX;
                err -= 2 * dy;
            }
            y += stepY;
            err += 2 * dx;
        }
    }
}








//功能：找起点,AB点（白到黑的跳变点）找到的点在白色像素
unsigned char start_point_l[2] = {0};
unsigned char start_point_r[2] = {0};
unsigned char get_start_point(unsigned char start_row , uint8_t (*bin_image)[IMAGE_W])
{
    unsigned char i = 0, l_found = 0, r_found = 0;

    start_point_l[0] = 0;
    start_point_l[1] = 0;
    start_point_r[0] = 0;
    start_point_r[1] = 0;

    // 找左边界：白→黑跳变
    for (i = IMAGE_W/2; i > 0; i--)//原来是i > border_min
    {
        start_point_l[0] = i;
        start_point_l[1] = start_row;
        if (bin_image[start_row][i] == 255 && bin_image[start_row][i-1] == 0)
        {
            l_found = 1;
            break;
        }
    }

    // 找右边界：白→黑跳变
    for (i = IMAGE_W/2; i < IMAGE_W; i++)//原来是i < border_max
    {
        start_point_r[0] = i;
        start_point_r[1] = start_row;
        if (bin_image[start_row][i] == 255 && bin_image[start_row][i+1] == 0)
        {
            r_found = 1;
            break;
        }
    }

    if(l_found && r_found)
        return 1;
    else
        return 0;
}





#define USE_num    IMAGE_H*3

// 左边界点数组：points_l[编号][0]=x, [1]=y
uint16_t points_l[USE_num][2] = {0};
// 右边界点数组：points_r[编号][0]=x, [1]=y
uint16_t points_r[USE_num][2] = {0};

// 左/右边界每一步的生长方向（0~7，对应8个邻域方向）
uint16_t dir_r[USE_num] = {0};
uint16_t dir_l[USE_num] = {0};
//345    543
//2 6    6 2
//107左  701右

// 实际搜到的左/右边界点数量
uint16_t data_stastics_l = 0;
uint16_t data_stastics_r = 0;

// 左右边界相遇的最高行（巡线终止位置）
uint8_t hightest = 0;

// 绝对值函数（防止调用库函数）
#define my_abs(a) ((a)>(0)?(a):(-a))

// =====================================================================================
// 函数功能：八邻域搜索左右跑道边界
// 入口：
//   break_flag    最大搜索次数（防止死循环）
//   image         二值图像数组（0黑=线，255白=背景）
//   l_stastic     左边界点数量（指针，输出）
//   r_stastic     右边界点数量（指针，输出）
//   l_start_x/y   左边界起始点（底部）
//   r_start_x/y   右边界起始点（底部）
//   hightest      左右边界相遇的最高行（输出）
// =====================================================================================
void search_l_r(uint16_t break_flag, uint8_t(*image)[IMAGE_W],
                uint16_t *l_stastic, uint16_t *r_stastic,
                uint8_t l_start_x, uint8_t l_start_y,
                uint8_t r_start_x, uint8_t r_start_y,
                uint8_t* hightest)
{
    uint8_t i = 0, j = 0;

    // ==================== 左边界8邻域初始化 ====================
    // 8个邻域坐标缓存
    uint8_t search_filds_l[8][2] = {0};
    // 左边界候选点数量
    uint8_t index_l = 0;
    // 左边界候选点缓存
    uint8_t temp_l[8][2] = {0};
    // 左边界当前中心点
    uint8_t center_point_l[2] = {0};
    // 左边界点计数
    uint16_t l_data_statics;

    // 左边界8邻域搜索顺序（顺时针）
    // 顺序：下 → 左下 → 左 → 左上 → 上 → 右上 → 右 → 右下
    static int8_t seeds_l[8][2] = {
        {0,1}, {-1,1}, {-1,0}, {-1,-1},
        {0,-1},{1,-1},{1,0},{1,1}
    };

    // ==================== 右边界8邻域初始化 ====================
    uint8_t search_filds_r[8][2] = {0};
    uint8_t center_point_r[2] = {0};
    uint8_t index_r = 0;
    uint8_t temp_r[8][2] = {0};
    uint16_t r_data_statics;

    // 右边界8邻域搜索顺序（逆时针，保证对称搜索）
    static int8_t seeds_r[8][2] = {
        {0,1},{1,1},{1,0},{1,-1},
        {0,-1},{-1,-1},{-1,0},{-1,1}
    };

    // 将外部计数读入本地变量
    l_data_statics = *l_stastic;
    r_data_statics = *r_stastic;

    // 设置左右边界的起始点（从底部开始）
    center_point_l[0] = l_start_x;
    center_point_l[1] = l_start_y;
    center_point_r[0] = r_start_x;
    center_point_r[1] = r_start_y;

    // ==================== 开始循环向上搜索 ====================
    while (break_flag--)
    {
        // ========== 1. 计算左点8个邻域的坐标 ==========
        for (i=0; i<8; i++)
        {
            search_filds_l[i][0] = center_point_l[0] + seeds_l[i][0];
            search_filds_l[i][1] = center_point_l[1] + seeds_l[i][1];
        }
        // 保存当前左中心点到边界数组
        points_l[l_data_statics][0] = center_point_l[0];
        points_l[l_data_statics][1] = center_point_l[1];
        l_data_statics++;

        // ========== 2. 计算右点8个邻域的坐标 ==========
        for (i=0; i<8; i++)
        {
            search_filds_r[i][0] = center_point_r[0] + seeds_r[i][0];
            search_filds_r[i][1] = center_point_r[1] + seeds_r[i][1];
        }
        // 保存当前右中心点到边界数组
        points_r[r_data_statics][0] = center_point_r[0];
        points_r[r_data_statics][1] = center_point_r[1];
        // ========== 3. 左边界：寻找下一个边缘点 ==========
        index_l = 0;
        // 清空候选点
        for(i=0;i<8;i++){ temp_l[i][0]=0; temp_l[i][1]=0; }

        // 遍历8个方向，寻找【黑点→白点】跳变（跑道边缘）
        for(i=0;i<8;i++)
        {
            // 当前点是黑线(0)，下一个点是背景(255) → 找到边缘
            if( image[search_filds_l[i][1]][search_filds_l[i][0]] == 0
             && image[search_filds_l[(i+1)&7][1]][search_filds_l[(i+1)&7][0]] == 255 )
            {
                // 保存这个边缘点
                temp_l[index_l][0] = search_filds_l[i][0];
                temp_l[index_l][1] = search_filds_l[i][1];
                index_l++;
                // 记录方向（用于十字、弯道判断）
                dir_l[l_data_statics-1] = i;
                //cout<<(int)i;
            }

            // 如果找到候选点，选最靠上的那个作为下一个中心点
            if(index_l)
            {
                center_point_l[0] = temp_l[0][0];
                center_point_l[1] = temp_l[0][1];
                // 遍历所有候选，取y最小（最上面）的点
                for(j=0;j<index_l;j++)
                {
                    if(center_point_l[1] > temp_l[j][1])
                    {
                        center_point_l[0] = temp_l[j][0];
                        center_point_l[1] = temp_l[j][1];
                    }
                }
            }
        }

        // ========== 4. 防死循环 & 边界相遇 & 同步保护 ==========
        // 连续3个点不动 → 卡死，退出
        if( (points_r[r_data_statics][0]==points_r[r_data_statics-1][0] && points_r[r_data_statics][0]==points_r[r_data_statics-2][0]
          && points_r[r_data_statics][1]==points_r[r_data_statics-1][1] && points_r[r_data_statics][1]==points_r[r_data_statics-2][1])
          ||(points_l[l_data_statics-1][0]==points_l[l_data_statics-2][0] && points_l[l_data_statics-1][0]==points_l[l_data_statics-3][0]
          && points_l[l_data_statics-1][1]==points_l[l_data_statics-2][1] && points_l[l_data_statics-1][1]==points_l[l_data_statics-3][1]))
        {
            break;
        }

        // 左右点几乎重合 → 边界相遇，巡线结束
        if( my_abs(points_r[r_data_statics][0] - points_l[l_data_statics-1][0]) < 2
         && my_abs(points_r[r_data_statics][1] - points_l[l_data_statics-1][1]) < 2 )
        {
            // 记录相遇的最高行
            *hightest = (points_r[r_data_statics][1] + points_l[l_data_statics-1][1])/2;
            break;
        }

        // 右边比左边高 → 左边等待右边，不更新,需要同步更新
        if( points_r[r_data_statics][1] < points_l[l_data_statics-1][1] )
        {
            continue;
        }

        // 特殊方向修正，防止跑偏，7表示方向为右下
        if( dir_l[l_data_statics-1] ==7 && (points_r[r_data_statics][1] > points_l[l_data_statics-1][1]) )
        {
            center_point_l[0] = points_l[l_data_statics-1][0];
            center_point_l[1] = points_l[l_data_statics-1][1];
            l_data_statics--;
        }

        // 右点计数+1
        r_data_statics++;

        // ========== 5. 右边界：寻找下一个边缘点 ==========
        index_r = 0;
        for(i=0;i<8;i++){ temp_r[i][0]=0; temp_r[i][1]=0; }

        // 同左边界逻辑：找 黑→白 跳变
        for(i=0;i<8;i++)
        {
            if( image[search_filds_r[i][1]][search_filds_r[i][0]] ==0
             && image[search_filds_r[(i+1)&7][1]][search_filds_r[(i+1)&7][0]] ==255 )
            {
                temp_r[index_r][0] = search_filds_r[i][0];
                temp_r[index_r][1] = search_filds_r[i][1];
                index_r++;
                dir_r[r_data_statics-1] = i;
            }

            if(index_r)
            {
                center_point_r[0] = temp_r[0][0];
                center_point_r[1] = temp_r[0][1];
                // 选最靠上的点
                for(j=0;j<index_r;j++)
                {
                    if(center_point_r[1] > temp_r[j][1])
                    {
                        center_point_r[0] = temp_r[j][0];
                        center_point_r[1] = temp_r[j][1];
                    }
                }
            }
        }
    }

    // 把最终计数写回外部变量
    *l_stastic = l_data_statics;
    *r_stastic = r_data_statics;
}




uint8_t l_border[IMAGE_H];
uint8_t r_border[IMAGE_H];
uint8_t center_line[IMAGE_H];

void get_left(uint16_t total_L)//原理同下，看下面的函数
{
    uint8_t i;
    uint16_t j;
    uint8_t h;

    for(i=0;i<IMAGE_H;i++) l_border[i] = border_min;
    h = IMAGE_H - 2;

    for(j=0;j<total_L;j++)
    {
        if(points_l[j][1] == h)
        {
            l_border[h] = points_l[j][0]+1;
        }
        else
            continue;
        h--;
        if(h == 0) break;
    }
}






void get_right(uint16_t total_R)
{
    uint8_t i;
    uint16_t j;
    uint8_t h;

    for(i=0;i<IMAGE_H;i++) r_border[i] = border_max;//默认右线
    h = IMAGE_H -2;//从倒数第三行开始

    for(j=0;j<total_R;j++)//每行只保留一个边界点
    {
        if(points_r[j][1] == h)
        {
            r_border[h] = points_r[j][0]-1;
        }
        else
            continue;
        h--;
        if(h ==0) break;//完成标志
    }
}








bool fit_line_range(
    const uint8_t *center_line, 
    int array_len,          // 外部传入的真实数组长度（必须传）
    int length_threshold,   // 长度阈值（你自己定，比如 50）
    int y_start,            // 你想取的起始行
    int y_end,              // 你想取的结束行
    float *k, float *b
)
{
    float sum_x = 0, sum_y = 0;
    float sum_xy = 0, sum_y2 = 0;
    int count = 0;

    // 安全检查
    if (!center_line || !k || !b) return false;
    if (array_len <= 0 || length_threshold < 0) return false;

    int from, to;

    // ==============================================
    // 核心逻辑：根据数组长度自动选择遍历范围
    // ==============================================
    if (array_len > length_threshold)
    {
        // 长度 > 阈值：只遍历 y_start ~ y_end
        from = (y_start < 0) ? 0 : y_start;
        to   = (y_end >= array_len) ? (array_len - 1) : y_end;
    }
    else
    {
        // 长度 ≤ 阈值：遍历整个数组（全部）
        from = 0;
        to   = array_len - 1;
    }

    // 范围无效直接返回
    if (from >= to) return false;

    // 开始遍历（绝对安全）
    for (int y = from; y <= to; y++)
    {
        uint8_t x = center_line[y];

        // 过滤无效坐标
        if (x <= 2 || x >= IMAGE_W - 2)
            continue;

        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_y2 += y * y;
        count++;
    }

    // 有效点太少，拟合失败
    if (count < (to - from) / 2)
        return false;

    float D = count * sum_y2 - sum_y * sum_y;
    if (fabs(D) < 1e-6)
        return false;

    // 最小二乘拟合 x = k * y + b
    *k = (count * sum_xy - sum_x * sum_y) / D;
    *b = (sum_x * sum_y2 - sum_y * sum_xy) / D;

    return true;
}

 void lq_udp_img_trans_demo(void)
{
    printf("=========================================\r\n");
    printf("  UDP Camera + Encoder Stream\r\n");
    printf("=========================================\r\n");
    printf("Target IP:   %s\r\n", TARGET_IP.c_str());
    printf("Target Port: %d\r\n", TARGET_PORT);
    printf("Resolution:  %dx%d\r\n", CAM_WIDTH, CAM_HEIGHT);
    printf("FPS:         %d\r\n", CAM_FPS);
    printf("=========================================\r\n");

    // 初始化UDP客户端
    lq_udp_client udp_client;
    udp_client.udp_client_init(TARGET_IP, TARGET_PORT);
    printf("UDP client initialized\r\n");

    // 初始化摄像头
    lq_camera cam(CAM_WIDTH, CAM_HEIGHT, CAM_FPS);
    if (!cam.is_opened()) {
        printf("ERROR: Failed to open camera!\r\n");
        return;
    }
    printf("Camera opened: %dx%d @ %dfps\r\n", cam.get_width(), cam.get_height(), cam.get_fps());

    // 发送帧计数
    uint32_t frame_count = 0;
    uint32_t encoder_count = 0;
    // 记录开始时间
    auto start_time = std::chrono::high_resolution_clock::now();

    printf("Start streaming... Press Ctrl+C to stop\r\n");

    while (true) {
        // ===================== 获取并发送图像 =====================
        // 获取原始图像
        cv::Mat frame = cam.get_raw_frame();
        if (frame.empty()) {
            printf("ERROR: Failed to read frame\r\n");
            continue;
        }

        // 在画面中间画方块
        int rows = frame.rows;
        int cols = frame.cols;
        int x1 = (cols - RECT_SIZE) / 2;
        int y1 = (rows - RECT_SIZE) / 2;
        cv::rectangle(frame, cv::Point(x1, y1), cv::Point(x1 + RECT_SIZE, y1 + RECT_SIZE), cv::Scalar(0, 255, 0), 2);

        // 发送JPEG压缩图像
        ssize_t sent = udp_client.udp_send_image(frame, JPEG_QUALITY);
        if (sent < 0) {
            printf("ERROR: Failed to send image\r\n");
        }

        frame_count++;

        // ===================== 每秒打印状态 =====================
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
        if (elapsed >= 1) {
            float fps = (float)frame_count / (float)elapsed;
            printf("FPS: %.2f\r\n", fps);
            frame_count = 0;
            encoder_count = 0;
            start_time = now;
        }
    }
}


void test_ziji(void)
{
    ls_gpio gpio1(PIN_72, GPIO_MODE_OUT);
    ls_gpio gpio2(PIN_73, GPIO_MODE_OUT);
    gpio1.gpio_level_set(GPIO_HIGH);
    gpio2.gpio_level_set(GPIO_LOW);
    PID_Controller vision_pid(20.0f,0.0f,0.0f,100.0f,0.0f);//�⻷λ�û�����pd
    PID_Controller motor_pid_l(2.0f,0.0f,0.0f,500.0f,1.0f);//�ڻ��ٶȻ�����pid
    PID_Controller motor_pid_r(2.0f,0.0f,0.0f,500.0f,1.0f);
    printf("=========================================\r\n");
    printf("  UDP Camera + Encoder Stream\r\n");
    printf("=========================================\r\n");
    printf("Target IP:   %s\r\n", TARGET_IP.c_str());
    printf("Target Port: %d\r\n", TARGET_PORT);
    printf("Resolution:  %dx%d\r\n", CAM_WIDTH, CAM_HEIGHT);
    printf("FPS:         %d\r\n", CAM_FPS);
    printf("=========================================\r\n");

    // 初始化UDP客户端
    lq_udp_client udp_client;
    udp_client.udp_client_init(TARGET_IP, TARGET_PORT);
    printf("UDP client initialized\r\n");

    // 初始化摄像头
    lq_camera cam(CAM_WIDTH, CAM_HEIGHT, CAM_FPS);
    if (!cam.is_opened()) {
        printf("ERROR: Failed to open camera!\r\n");
        return;
    }
    printf("Camera opened: %dx%d @ %dfps\r\n", cam.get_width(), cam.get_height(), cam.get_fps());

    // 发送帧计数
    uint32_t frame_count = 0;
    uint32_t encoder_count = 0;
    // 记录开始时间
    auto start_time = std::chrono::high_resolution_clock::now();

    printf("Start streaming... Press Ctrl+C to stop\r\n");

    while (true) {
        // ===================== 获取并发送图像 =====================
        // 获取原始图像
        cv::Mat gray_p;
        uchar small_image[60][80];
        get_gray_fast(cam,small_image);

        uchar YUZHI=otsu_binary0(small_image);//更新阈值

        otsu_binary(small_image,YUZHI);//根据阈值二值化

        image_filter(small_image);//补断线，去噪声

        image_draw_rectan(small_image);//画黑框（算法需要

        uint16_t i;
        uint8_t hightest = 0;//重要参数，是左右线相遇时的最高点


        data_stastics_l =0;
        data_stastics_r =0;

        if(get_start_point((IMAGE_H-2),small_image)) // 5.找起点,只有找到时才运行下面程序
        {
            search_l_r(USE_num, small_image, &data_stastics_l, &data_stastics_r,
                   start_point_l[0], start_point_l[1],
                   start_point_r[0], start_point_r[1], &hightest);  //提取左右线

            get_left(data_stastics_l);    // 6.提取左线
            get_right(data_stastics_r);   // 7.提取右线

        }

         for(i=hightest; i<IMAGE_H-1; i++)
        {
            center_line[i] = (l_border[i] + r_border[i])/2;
            small_image[i][l_border[i]]=180;
            small_image[i][r_border[i]]=80;
            small_image[i][center_line[i]]=0;
        }
        int array_len = sizeof(center_line) / sizeof(center_line[0]);

        int threshold = 50;  





   
    float based_speed=1000.0;//60תÿ����
    float MAX_SPEED=1200.0f;

    float k, b;
    float target_y=20.0f;
    float l,r;//调试用
    int ld,rd;
    float lb,rb;
        if(fit_line_range(
            center_line,
            array_len,
            threshold,    // 阈值
            10, 40,       // 你想取的区间 start ~ end
            &k, &b
        ))
    
        {
            float predicted_x=k * target_y + b ;
            vision_pid.setTarget(40.0f);//Ŀ��x����
            float turn_output=-vision_pid.PID_Calculate(predicted_x);//Ԥ��x���꣬�����pd�����µ������
            float target_speed_l=based_speed + turn_output;
            float target_speed_r=based_speed - turn_output;
            //����޷�
            if (target_speed_l>MAX_SPEED) target_speed_l=MAX_SPEED;
            if (target_speed_r>MAX_SPEED) target_speed_r=MAX_SPEED;
            //��Ŀ���ٶ����ø��ڻ�pid
            motor_pid_l.setTarget(target_speed_l);
            motor_pid_r.setTarget(target_speed_r);
            //ͨ�����������ʵ���ٶ�
            float motor_speed_l=get_l_motor_speed() * 100;
            float motor_speed_r=-get_r_motor_speed() * 100;//调试时加的
            //��ȡĿ��pwm
            int motor_pwm_l=(int)motor_pid_l.PID_Calculate(motor_speed_l);
            int motor_pwm_r=(int)motor_pid_r.PID_Calculate(motor_speed_r);
            
            if(motor_pwm_l<0)
            {
                motor_pwm_l=-motor_pwm_l;
            }
            if(motor_pwm_r<0)
            {
                motor_pwm_r=-motor_pwm_r;
            }


            set_motor_pwm(motor_pwm_l,motor_pwm_r);
            l=target_speed_l;
            r=target_speed_r;
            ld=motor_pwm_l;
            rd=motor_pwm_r;
            lb=motor_speed_l;
            rb=motor_speed_r;
        }
        
        show_array_image(small_image);
        frame_count++;

        // ===================== 每秒打印状态 =====================
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
        if (elapsed >= 1) {
            float fps = (float)frame_count / (float)elapsed;
            printf("FPS: %.2f\r\n", fps);
            frame_count = 0;
            encoder_count = 0;
            start_time = now;
            printf("l: %.2f\r\n", l);
            printf("r: %.2f\r\n", r);
            printf("ld: %d\r\n", ld);
            printf("rd: %d\r\n", rd);
            printf("lb: %.2f\r\n", lb);
            printf("rb: %.2f\r\n", rb);
        }
    }
}