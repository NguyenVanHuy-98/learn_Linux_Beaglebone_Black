#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

int main(int argc, char *argv[])
{
    const char *ifname = "main_dcan1";  // đổi thành "can1" nếu bạn rename

    if (argc > 1) {
        ifname = argv[1];              // cho phép truyền tên interface từ dòng lệnh
    }

    int s;
    struct ifreq ifr;
    struct sockaddr_can addr;

    /* 1) Tạo socket CAN RAW */
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("socket");
        return 1;
    }

    /* 2) Lấy chỉ số interface (ifindex) từ tên, ví dụ "main_dcan1" */
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        close(s);
        return 1;
    }

    /* 3) Bind socket vào interface đó */
    memset(&addr, 0, sizeof(addr));
    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(s);
        return 1;
    }

    /* 4) (Tùy chọn) Thiết lập filter: chỉ nhận frame ID 0x123 */
    struct can_filter rfilter[1];
    rfilter[0].can_id   = 0x123;
    rfilter[0].can_mask = CAN_SFF_MASK;       // 11-bit ID
    if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter)) < 0) {
        perror("setsockopt filter");
        close(s);
        return 1;
    }

    printf("Listening on %s for CAN frames (ID 0x123)...\n", ifname);

    /* 5) Vòng lặp nhận frame */
    while (1) {
        struct can_frame frame;
        ssize_t nbytes = read(s, &frame, sizeof(frame));

        if (nbytes < 0) {
            perror("read");
            break;
        } else if (nbytes < (ssize_t)sizeof(struct can_frame)) {
            fprintf(stderr, "read: incomplete CAN frame\n");
            continue;
        }

        printf("ID=0x%03X DLC=%d Data=", frame.can_id & CAN_SFF_MASK, frame.can_dlc);
        for (int i = 0; i < frame.can_dlc; i++) {
            printf(" %02X", frame.data[i]);
        }
        printf("\n");
    }

    close(s);
    return 0;
}