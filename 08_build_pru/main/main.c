#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

typedef struct {
    volatile uint32_t magic;
    volatile uint32_t pwm_period;
    volatile uint32_t pwm_duty;
    volatile int32_t  enc0_count;
    volatile int32_t  enc1_count;
} pru_mailbox_t;

static int read_text_file(const char *path, char *buf, size_t buflen)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    ssize_t n = read(fd, buf, buflen - 1);
    close(fd);
    if (n <= 0) return -1;
    buf[n] = '\0';
    // strip newline
    char *p = strchr(buf, '\n');
    if (p) *p = '\0';
    return 0;
}

static unsigned long read_hex_ul(const char *path)
{
    char buf[128];
    if (read_text_file(path, buf, sizeof(buf)) < 0) {
        perror(path);
        exit(1);
    }
    errno = 0;
    unsigned long v = strtoul(buf, NULL, 0); // accepts 0x...
    if (errno) {
        perror("strtoul");
        exit(1);
    }
    return v;
}

static int find_uio_with_name(const char *want_name, char *out_uio, size_t outlen)
{
    // Thử uio0..uio15 (thường đủ)
    for (int i = 0; i < 16; i++) {
        char path[256], name[256];
        snprintf(path, sizeof(path), "/sys/class/uio/uio%d/name", i);
        if (read_text_file(path, name, sizeof(name)) == 0) {
            // name ví dụ: "pruss_evt0", "uio_pruss", ...
            if (strstr(name, want_name) != NULL) {
                snprintf(out_uio, outlen, "/dev/uio%d", i);
                return i;
            }
        }
    }
    return -1;
}

int main(int argc, char **argv)
{
    // Bạn có thể chỉnh "want_name" theo đúng name trên máy bạn trong /sys/class/uio/uioX/name
    char devuio[64];
    int uio_idx = find_uio_with_name("pruss", devuio, sizeof(devuio));
    if (uio_idx < 0) uio_idx = find_uio_with_name("PRUSS", devuio, sizeof(devuio));
    if (uio_idx < 0) {
        fprintf(stderr, "Không tìm thấy uio pruss. Hãy kiểm tra /sys/class/uio/uio*/name\n");
        return 1;
    }

    // Ta cần mmap đúng map chứa "shared RAM".
    // Tùy hệ thống map0/map1/... khác nhau, nên đọc name từng map.
    int map_found = -1;
    for (int m = 0; m < 6; m++) {
        char path[256], mapname[256];
        snprintf(path, sizeof(path), "/sys/class/uio/uio%d/maps/map%d/name", uio_idx, m);
        if (read_text_file(path, mapname, sizeof(mapname)) == 0) {
            // thường shared RAM có tên kiểu "pruss_shrdrm2" hoặc có chữ "shr"/"shared"
            if (strstr(mapname, "shr") || strstr(mapname, "shared") || strstr(mapname, "SHR")) {
                map_found = m;
                break;
            }
        }
    }

    if (map_found < 0) {
        fprintf(stderr, "Không tìm được map shared RAM qua sysfs. In danh sách map để bạn xem:\n");
        for (int m = 0; m < 6; m++) {
            char path[256], mapname[256];
            snprintf(path, sizeof(path), "/sys/class/uio/uio%d/maps/map%d/name", uio_idx, m);
            if (read_text_file(path, mapname, sizeof(mapname)) == 0) {
                fprintf(stderr, "  map%d name=%s\n", m, mapname);
            }
        }
        return 1;
    }

    char addr_path[256], size_path[256];
    snprintf(addr_path, sizeof(addr_path), "/sys/class/uio/uio%d/maps/map%d/addr", uio_idx, map_found);
    snprintf(size_path, sizeof(size_path), "/sys/class/uio/uio%d/maps/map%d/size", uio_idx, map_found);

    unsigned long phys_addr = read_hex_ul(addr_path);
    unsigned long map_size  = read_hex_ul(size_path);

    printf("Dùng %s, map%d (shared?) phys=0x%lx size=0x%lx\n", devuio, map_found, phys_addr, map_size);

    int fd = open(devuio, O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open /dev/uioX");
        return 1;
    }

    void *base = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)map_found * getpagesize());
    if (base == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    volatile pru_mailbox_t *mb = (volatile pru_mailbox_t*)base;

    // Kiểm tra magic (nếu bạn set ở PRU)
    printf("magic = 0x%08x\n", mb->magic);

    // Ví dụ: set PWM period & duty từ Linux
    // (nhớ match đơn vị: PRU cycles)
    mb->pwm_period = 20000;
    mb->pwm_duty   = 1000;

    // Loop đọc encoder và thay duty theo input (demo)
    while (1) {
        int32_t e0 = mb->enc0_count;
        int32_t e1 = mb->enc1_count;

        printf("enc0=%d enc1=%d duty=%u/%u\n",
               e0, e1, (unsigned)mb->pwm_duty, (unsigned)mb->pwm_period);

        // Demo: tăng duty theo enc0 (chỉ ví dụ, bạn có thể thay logic)
        // mb->pwm_duty = ...;

        usleep(100000); // 100ms
    }

    munmap((void*)base, map_size);
    close(fd);
    return 0;
}
