#ifndef DEV_IDENTIFY_H
#define DEV_IDENTIFY_H

#ifdef __cplusplus
extern "C" {
#endif

#define NAME_LEN 64
#define PORT_COUNT 50

struct devscan_result_s
{
	int status;// ip state
	int devtype_id;
	char devtype_cn_name[NAME_LEN];//?D??
	char devtype_en_name[NAME_LEN];//иобщ??
	char devclass_name[NAME_LEN];
	int sysclass_id;
	char sysclass_name[NAME_LEN];//linux
	char systype_name[NAME_LEN];//linux 2.6
	int portcount;
	int portlist[PORT_COUNT];
};

int devscan_identify(unsigned int ipaddr,struct devscan_result_s *devscan_result);

#ifdef __cplusplus
}
#endif


#endif

