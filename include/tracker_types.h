#ifndef TRACKER_TYPES_H
#define TRACKER_TYPES_H

typedef struct Database_t{
	char* db_hostname;
	char* db_username;
	char* db_password;
	char* db_database;
	unsigned int db_port;
} Database_t;

typedef struct Camera_t
{
	char* cam_uri;
	char* cam_brand;
	char* cam_username;
	char* cam_password;
	char* vid_dir;
	char* vid_filename;

} Camera_t;

#endif // TRACKER_TYPES_H
