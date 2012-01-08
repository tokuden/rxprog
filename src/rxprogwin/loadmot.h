#ifndef H_LOADMOT
#define H_LOADMOT

bool load_mot_file(const char *filename,
				   unsigned char *buffer,
				   unsigned int buffer_size,
				   unsigned char default_padding,
				   unsigned long *pstartaddr,
				   unsigned long *plastaddr,
				   unsigned long *pentryaddr,
				   char *message_buf,
				   unsigned long message_bufsize);

#endif
