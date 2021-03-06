/**
 * Created by jed
 * User: jedartois@gmail.com
 * Date: 25/01/12
 * Time: 11:47
 Simple an robust posix serial interface

ChangeLog:
- fix concurrent JVM (open the same file descriptor)   27 juin 2012 jed
- fix  trigger event on one byte                       11 octobre 2012 jed
 */

#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include "serialposix.h"
#include <dirent.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define VERSION 1.7
#define JED_IPC_PRIVATE 24011950
#define BUFFER_SIZE 512
#define OK 0
#define ERROR -1
#define ALIVE 0
#define EXIT 1

#define FD_DISCONNECTED -10
#define NOP_READ_MAX 10
#define CLOSED_THREAD_READER -11
#define CLOSED_THREAD_MONITEUR -12

#define EXIT_CONCURRENT_VM -42
#define FD_ALREADY_CLOSED -12
//#define PROC_BASE  "/dev/"

int shmid;

typedef struct _device
{
  char device_name[512];
  int fd;
  int alive;
} Device;

typedef struct _context
{
        Device devices[25];
        int nb;
} Context;

static Context *ctx;
static char current_device[512];
/**
 *  function to throw event
 */
void (*SerialEvent) (int taille,unsigned char *data);

/**
 *  Assign function
 * @param fn pointer to the callback
 */
int register_SerialEvent( void* fn){
	SerialEvent=fn;
	return OK;
};


/*                     -
Port* scan_fd(void)
{
	DIR *dir;
	struct dirent *de;
	char path[PATH_MAX+1];
	pid_t pid;
	int empty;
	struct stat sb;
	if (!(dir = opendir(PROC_BASE)))
	{
		perror(PROC_BASE);
		return NULL;
	}
	empty = 0;
	char name[PATH_MAX+1];


	static Port myports[MAX_PORTS];
	int i;
	for(i=0;i<MAX_PORTS;i++)
	{
		memset(myports[i].devicename,0,sizeof(myports[i].devicename));
	}
	while ( ( de = readdir(dir) ) )
	{

		// /dev/cu.*     /dev/cu.usbserial-XXXXX
		// /dev/tty.*    dev/tty.usbserial-XXXXX
		// dev/ttyUSB*
		// dev/ttyAC*
		if(de->d_name[0] == 't' && de->d_name[1] == 't' && de->d_name[2] == 'y' || de->d_name[0] == 'c' && de->d_name[1] == 'u' )
		{
			sprintf(name,"%s%s",PROC_BASE,de->d_name);
			if (stat(name,&sb) >= 0) {
				switch (sb.st_mode & S_IFMT) {
				case S_IFCHR:

					if(de->d_name[3] == 'U' || de->d_name[3] == '.' || de->d_name[3] == 'A')
					{
						strcpy(myports[empty].devicename,de->d_name);
						empty++;
					}
					break;
				}
			}
		}
	}

	closedir(dir);

	if (empty == 0)
	{
		return myports;
		//fprintf(stderr,PROC_BASE " is empty (not mounted ?)\n");

	}
	return &myports[0];
}
*/

/**
 *  Write a byte on the fd
 * @param  file descriptor
 * @param  byte
 */
int serialport_writebyte( int fd, uint8_t b)
{
	int n = write(fd,&b,1);
	if( n!=1)
		return -1;
	return OK;
}

/**
 * Read a byte on the fd
 * @param  file descriptor
 * @param  byte
 */
int serialport_readbyte( int fd, uint8_t *b)
{
	int n = read(fd,&b,1);
	if( n!=1)
		return ERROR;
	return OK;
}

/**
 * write an array of char
 * @param  file descriptor
 * @param  pointer of char
 */
int serialport_write(int fd,  char* str)
{
	int len,n;
	len= strlen(str);

	n = write(fd, str, len);
	if( n!=len )
	{
	    perror("write");
	   	return ERROR;
	}
	return serialport_writebyte(fd,'\n'); /* finish */
}

/**
 * read an array of char
 * @param  file descriptor
 * @param  pointer of char
 */

int serialport_read(int fd, char *ptr){
	char b[1];
	int i=0;
	int n;
	int nop_count=0;
	do {
		n = read(fd, b, 1);
		if( n==-1) {
			usleep( 100*1000 );
			nop_count++;
			continue;
		}
		if( n==0 ) {
			usleep( 10 * 1000 );
            nop_count++;
			continue;
		}
		if(b[0] != 10){
			ptr[i] = b[0];
			i++;
		}
	} while(((b[0] != '\n') && (isAlive(search_device(current_device)) == ALIVE) && (i < BUFFER_SIZE)) && (i > 0 &&  nop_count < NOP_READ_MAX)); /* detect finish and protect overflow*/

	return i;
}


/**
 *   Verify is the serial port is opennable
 * @param devicename the name of serial port  eg: /dev/ttyUSB0
 */
int verify_fd(char *devicename)
{
	int fd;
	if((fd = open(devicename,O_RDONLY|  O_NONBLOCK )) == -1)
	{
		return ERROR;
	}
	else
	{
		close(fd);
		return 0;
	}

}

/**
 *   Monitor the serial and throw an event if the link is broken
 * @param devicename the name of serial port  eg: /dev/ttyUSB0
 */
void *serial_monitoring(char *devicename)
{
	char name[512];
	int fd;
	strcpy(name,devicename);    // store local to protect garbage collector JNA
	while(isAlive(search_device(devicename))==ALIVE)
	{
		sleep(1);
		if(verify_fd(name) == -1)
		{
			SerialEvent(FD_DISCONNECTED,"");
		    pthread_exit(NULL);
		}
	}
    SerialEvent(CLOSED_THREAD_MONITEUR,"");
	pthread_exit(NULL);
}

/**
 * throw callback event on icomming data
 * @param int file descriptor
 */
void *serial_reader(int fd)
{

	char byte[BUFFER_SIZE];
	int taille;
	while(isAlive(search_device(current_device)) ==ALIVE)
	{
		if((taille =serialport_read(fd,byte)) > 0 && isAlive(search_device(current_device)) ==ALIVE)
		{
			SerialEvent(taille,byte);
			memset(byte,0,sizeof(byte));
		}
	}
    SerialEvent(CLOSED_THREAD_READER,"");
	if(isAlive(search_device(current_device)) == EXIT_CONCURRENT_VM)
	{
	 	SerialEvent(EXIT_CONCURRENT_VM,"");
	}
	pthread_exit(NULL);
}

/**
 * Create a pthread  to manage incomming data
 * @param int file descriptor
 */
int reader_serial(int fd){
	pthread_t lecture;
	if(isAlive(search_device(current_device)) ==ALIVE)
	{
		return  pthread_create (& lecture, NULL,&serial_reader, fd);
	} else
	{
	    return ERROR;
	}
}

/**
 * Create a pthread  to manage incomming data
 * @param devicename the name of serial port  eg: /dev/ttyUSB0
 */
int monitoring_serial(char *name_device)
{
	pthread_t monitor;
	if(isAlive(search_device(name_device)) ==ALIVE)
    {
     	return pthread_create (& monitor, NULL,&serial_monitoring, name_device);
    }
    else
    {
        return ERROR;
   }
}


int search_device(char *device_name)
{
int i=0;
    for(i=0;i<ctx->nb;i++)
    {
        if(strcmp(ctx->devices[i].device_name,device_name) == 0)
        {
       // printf("%s == %s\n",ctx->devices[i].device_name,device_name);
            return i;
         break;
        }
     }
    return -1;
}
/*

int search_device_fd(int fd)
{
    int i =0;
     for(i=0;i<ctx->nb;i++)
     {
         if(ctx->devices[i].fd ==fd)
         {
             return i;
          break;
         }
      }
     return -1;
}
 */

int isAlive(int indice)
{
    if(indice != -1)
    {
     return ctx->devices[indice].alive;
    }
    else
     {
        return EXIT;
    }
}

Context * getContext()
{
    char *addr;
   // create memory shared
   shmid = shmget(JED_IPC_PRIVATE,sizeof(Context), 0666 | IPC_CREAT );
   if(shmid < 0)
   {
       perror("shmid");
       return NULL;
   }
   addr = shmat(shmid, 0, 0);
   if(addr < 0)
   {
      perror("shmat");
      return NULL;
   }
    // bind to memory shared
  return  (Context *) addr;
}
/**
 * Open a file descriptor
 * @param devicename the name of serial port  eg: /dev/ttyUSB0
 * @param bitrate the speed of the serial port
 */
int open_serial(const char *_name_device,int _bitrate){
	int fd,bitrate;
	struct termios termios;
    int  status = 0;
	/* process baud rate */
	switch (_bitrate) {
	case 4800: bitrate = B4800; break;
	case 9600: bitrate = B9600; break;
	case 19200: bitrate = B19200; break;
	case 38400: bitrate = B38400; break;
	case 57600: bitrate = B57600; break;
	case 115200: bitrate = B115200; break;
	case 230400 : bitrate = B230400; break;
	default:
		return -1;
	}

	ctx = getContext();
    if(ctx == NULL)
    {
        return -1;
    }

    memset(current_device,0,sizeof(current_device));
    strcpy(current_device,_name_device);
    int indice = search_device((char*)_name_device);
    if(indice == -1)
    {
         ctx->nb = ctx->nb +1;
         indice =   ctx->nb;
         strcpy(ctx->devices[indice].device_name,_name_device);
         ctx->devices[indice].alive = ALIVE;
    }
    else
    {
      //  printf("device %s found %d alive %d\n",_name_device,indice,ctx->devices[indice].alive);

        if(ctx->devices[indice].alive ==ALIVE)
        {
       //   printf("was alive \n");
          ctx->devices[indice].alive = EXIT_CONCURRENT_VM;;
          sleep(2);
          ctx->devices[indice].alive = ALIVE;
        }
        else
        {
              //  printf("Creating \n");
          ctx->devices[indice].alive = ALIVE;
        }

    }


	/* open the serial device */
	fd = open(_name_device,O_RDWR |O_NOCTTY | O_NONBLOCK );

	if(fd < 0)
	{
		close_serial(fd);
		return -2;
	}
     // set context
    ctx->devices[indice].fd = fd;

    tcgetattr(fd, &termios);
    termios.c_iflag       = INPCK;
    termios.c_lflag       = 0;
    termios.c_oflag       = 0;
    termios.c_cflag       = CREAD | CS8 | CLOCAL;
    termios.c_cc[ VMIN ]  = 0;
    termios.c_cc[ VTIME ] = 0;
    cfsetispeed(&termios, bitrate);
    cfsetospeed(&termios, bitrate);
    tcsetattr(fd, TCSANOW, &termios);

	if (tcsetattr(fd, TCSANOW, & termios) != 0) {
		perror("tcflush");
       	return  -4;
	}

    /* flush the serial device */
	if (tcflush(fd, TCIOFLUSH))
	{
		perror("tcflush");
		return  -4;
	}

	if( tcsetattr(fd, TCSANOW, &termios) < 0) {
		perror("init_serialport: Couldn't set term attributes");
		return -5;
	}

	return fd;
}

int close_serial(int fd)
{

	ctx = getContext();
    if(ctx == NULL)
    {
         printf("ERROR\n") ;
      	close(fd);
        return ERROR;
    }
    // todo destroy memory shared if there is no more clients
	if(isAlive(search_device(current_device)) ==ALIVE)
	{
		close(fd);
		ctx->devices[search_device(current_device)].alive = EXIT;
	}else
	{
		return FD_ALREADY_CLOSED;
	}

	return OK;
}


