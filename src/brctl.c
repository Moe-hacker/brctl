#include <linux/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#if __STDC_VERSION__ < 202000L
#define bool _Bool
#define true ((_Bool) + 1u)
#define false ((_Bool) + 0u)
#endif
#include "include/k2v.h"
struct CONFIG {
	int low_max;
	int mid_max;
	float low_inc;
	float mid_inc;
	float hig_inc;
	int sleep_time;
	char *panel;
} config = {
	.low_max = 0,
	.mid_max = 0,
	.low_inc = 0,
	.mid_inc = 0,
	.hig_inc = 0,
	.sleep_time = 0,
	.panel = NULL,
};
void init_config(void)
{
	char *conf_file = "/data/adb/modules/brctl/br.conf";
	char *buf = k2v_open_file(conf_file, 4096);
	config.panel = key_get_char("panel", buf);
	config.low_max = key_get_int("low_max", buf);
	config.mid_max = key_get_int("mid_max", buf);
	config.low_inc = key_get_float("low_inc", buf);
	config.mid_inc = key_get_float("mid_inc", buf);
	config.hig_inc = key_get_float("hig_inc", buf);
	config.sleep_time = key_get_int("sleep_time", buf);
	free(buf);
}
u_int ftoi(char *path)
{
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		exit(EXIT_FAILURE);
	}
	char buf[128] = { '\0' };
	read(fd, buf, sizeof(buf));
	return (u_int)atoi(buf);
}
void set_br(char *brctl_path, u_int br)
{
	int fd = open(brctl_path, O_RDWR);
	if (fd < 0) {
		exit(EXIT_FAILURE);
	}
	char buf[128] = { '\0' };
	sprintf(buf, "%u\n", br);
	write(fd, buf, strlen(buf));
	close(fd);
}
u_int new_br(u_int cur_br, u_int max_br)
{
	u_int ret = cur_br;
	int br_per = (int)(((float)cur_br / (float)max_br) * 100);
	if (br_per < config.low_max) {
		ret = (u_int)((float)cur_br * config.low_inc);
		if (ret > max_br) {
			ret = max_br;
		}
		return ret;
	}
	if (br_per < config.mid_max) {
		ret = (u_int)((float)cur_br * config.mid_inc);
		if (ret > max_br) {
			ret = max_br;
		}
		return ret;
	}
	ret = (u_int)((float)cur_br * config.hig_inc);
	if (ret > max_br) {
		ret = max_br;
	}
	return ret;
}
void brctl_daemon()
{
	init_config();
	char brctl_path[PATH_MAX] = { '\0' };
	char brmax_path[PATH_MAX] = { '\0' };
	sprintf(brctl_path, "/sys/class/backlight/%s/brightness", config.panel);
	sprintf(brmax_path, "/sys/class/backlight/%s/max_brightness", config.panel);
	u_int max_br = ftoi(brmax_path);
	u_int cur_br = ftoi(brctl_path);
	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs, SIGTTIN);
	sigaddset(&sigs, SIGTTOU);
	sigprocmask(SIG_BLOCK, &sigs, 0);
	u_int bk_br = cur_br;
	while (true) {
		cur_br = ftoi(brctl_path);
		if (cur_br != bk_br) {
			sleep(config.sleep_time);
			cur_br = ftoi(brctl_path);
			bk_br = new_br(cur_br, max_br);
			set_br(brctl_path, bk_br);
			fflush(stdout);
		}
		sleep(3);
	}
}
int main(void)
{
	brctl_daemon();
}
