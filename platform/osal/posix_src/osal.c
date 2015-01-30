/*


Copyright (c) 2013, Rasmus Zakarias, Aarhus University
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software

   must display the following acknowledgement:

   This product includes software developed by Rasmus Winther Zakarias 
   at Aarhus University.

4. Neither the name of Aarhus University nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY Rasmus Zakarias at Aarhus University 
''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Rasmus Zakarias at Aarhus University BE 
LIABLE FOR ANY, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Created: 2014-10-26

Author: Rasmus Winther Zakarias, rwl@cs.au.dk

Changes: 
2014-10-26 14:30: Initial version created
*/

/*!
 * Linux Operating System Abstraction Layer
 * 
 * Author: Rasmus Winther Lauritsen
 * 
 * 
 */

// Framework
#include "osal.h"
#include "coov3.h"
#include "list.h"
#include "singlelinkedlist.h"
#include <common.h>
#include <config.h>

// libc (stdandard libc functions)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Linux System
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <netinet/in.h>

// Library names for names which can be expected of the standard platform.
const char * DATE_TIME_LIBRARY = "OSAL_DATE_TIME_LIBRARY";
const char * TERMINAL_LIBRARY = "OSAL_TERMINAL_LIBRARY";
const char * NETWORK_LIBRARY = "OSAL_NETWORK_LIBRARY";
const char * GRAPHICS_LIBRARY = "OSAL_GRAPHICS_LIBRARY";

extern char *strerror (int __errnum);

// static structure boot-strapping the osal system
MUTEX _static_lock;
static uint no_osal_instance;

// informative return value
typedef struct _osal_ret_val_ {
  uint err;
  int rval;
} retval;

// static read libstdc is static in structure
// _static_lock protects libc.
static
retval __read(int fd, byte * buf, uint lbuf) {
  retval res = {0};
  Mutex_lock(_static_lock);
  res.rval = read(fd, buf, lbuf);
  res.err = errno;
  Mutex_unlock(_static_lock);
  return res;
}

// static write libstdc is static in structure
// _static_lock protects libc.
static
retval __write(int fd, byte * buf, uint lbuf) {
  retval res = {0};
  Mutex_lock(_static_lock);
  res.rval = write(fd, buf, lbuf);
  res.err = errno;
  Mutex_unlock(_static_lock);
  return res;
}


// ------------------------------------------------------------
Cmaphore Cmaphore_new(OE oe, uint count) {
  Cmaphore res = (Cmaphore)oe->getmem(sizeof(*res));
  if (!res) return 0;
  res->lock = oe->newmutex();
  res->count = count;
  res->oe = oe;
  return res;
};

void Cmaphore_up(Cmaphore c) {
  OE oe = 0;
  if (!c) return;
  if (!c->lock) return;
  c->oe->lock(c->lock);
  ++(c->count);
  c->oe->unlock(c->lock);
}

void Cmaphore_down(Cmaphore c) {
  OE oe = 0;
  if (!c) return;
  if (!c->lock) return;

  oe = c->oe;
  if (!oe) return;

  oe->lock(c->lock);
  while(c->count <= 0) {
    oe->unlock( c->lock );
    oe->yieldthread();
    oe->lock( c->lock );
    if (!c->oe)  { 
      oe->unlock(c->lock);
      return; // semaphore destroyed, leave !
    }
  }
  --(c->count);
  oe->unlock( c->lock );
}

void Cmaphore_destroy(Cmaphore * c) {
  OE oe = 0;
  Cmaphore s = 0;
  if (!c) return;
  if (!*c) return;

  s = *c;
  oe = s->oe;
  
  // inform all down's that we are destroying this semaphore
  oe->lock(s->lock);
  s->oe = 0;
  oe->unlock(s->lock);

  // give all other threads a chance to run.
  oe->yieldthread();

  // Really now we destroy it !
  oe->lock(s->lock);
  s->count = 0;
  oe->destroymutex(&s->lock);
  zeromem(s, sizeof(*s));
  *c = 0;
  oe->putmem(s);
}


// ------------------------------------------------------------


static
void set_non_blocking(int fd);

static
unsigned long long _nano_time() {
 return 0; }
#if 0
static
unsigned long long _nano_time() {
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return 1000000000L*tspec.tv_sec + tspec.tv_nsec;
  } else {
    return 0;
  }
}
#endif

typedef struct _simple_oe_ {
  /*!
   * mm = main memory
   */
  Memory mm;
  /*!
   * sm = special memory in which writing and executing the same
   * addresses are allowed.
   */
  Memory sm;
  /*!
   * Threads allocated by this instance.
   */
  List threads;
  /*!
   * List of all open file descriptors
   */
  List filedescriptors;
  
  /*!
   * Lock that protects the state of this instance.
   */
  MUTEX lock;

  /*!
   * The current log level.
   */
  LogLevel loglevel;

  /*!
   * File descriptor to which logs are written. Stdout is zero.
   */
  int log_file;

} * SimpleOE;


typedef struct _file_descriptor_entry_ {
  int osfd;
  uint fd;
} * FDEntry;

uint fd_pool;
static uint FDEntry_new(OE oe, List entries, int osfd) {
  FDEntry r = (FDEntry)oe->getmem(sizeof(*r));
  if (!r) return 0;

  r->osfd = osfd;
  r->fd = ++fd_pool;

  entries->add_element(r);

  return r->fd;
}

static void FDEntry_destroy(OE oe, FDEntry * ent) {
  FDEntry e = 0;
  if (!ent) return;
  if (!*ent) return;
  e = *ent;

  oe->putmem(e);
  *ent = 0;
}

static int FDEntry_remove(OE oe, List entries, uint fd) {
  uint siz = 0;
  int i  = 0;

  if (!entries) return 0;
  
  siz = entries->size();
  for(i = 0;i < siz;++i) {
    FDEntry c = (FDEntry)entries->get_element(i);
    if (c && c->fd == fd) {
      int r = c->osfd;
      entries->rem_element(i);
      FDEntry_destroy(oe, &c);
      return r;
    }
  }
  return -1;
}

static int FDEntry_lookup(List entries, uint fd) {
  uint siz = 0;
  int i  = 0;

  if (!entries) return 0;
  
  siz = entries->size();
  for(i = 0;i < siz;++i) {
    FDEntry c = (FDEntry)entries->get_element(i);
    if (c && c->fd == fd) return c->osfd;

  }

  return -1;
}


COO_DCL( OE, void *, getmem, uint size)
COO_DEF_RET_ARGS(OE, void *, getmem, uint i;,i) {
  uint j = 0;
  byte * res = (byte*) malloc(i);
  if (!res) { 
    this->syslog(OSAL_LOGLEVEL_FATAL, "Out of memory");
    return 0;
  }
  for(j=0;j<i;++j) res[j] = 0;
  return res;
}

COO_DCL( OE, void, putmem, void * p)
COO_DEF_NORET_ARGS(OE, putmem, void * p;,p) {
  if (p != 0) {
    free(p); 
  }
}

COO_DCL(OE, RC, read, uint fd, byte * buf, uint * lbuf)
COO_DEF_RET_ARGS(OE, RC, read, uint fd;byte *buf;uint * lbuf;, fd, buf, lbuf ) {
  int r = 0;
  SimpleOE soe = (SimpleOE)this->impl;
  int os_fd = 0;
  ull start = 0;
  retval ret = {0};
  if (!lbuf) return RC_FAIL;

  if (fd < 1) return RC_FAIL;

  this->lock(soe->lock);
  os_fd = FDEntry_lookup(soe->filedescriptors,fd);
  this->unlock(soe->lock);

  if (os_fd < 0) return RC_FAIL; 

  {
    fd_set read_set = {0};
    struct timeval timeout = {0};
    FD_ZERO(&read_set);
    timeout.tv_sec = 0;
    timeout.tv_usec = 230; // latency on Giga bit LAN
    FD_SET(os_fd, &read_set);
    if (select(os_fd+1, &read_set, 0,0,&timeout) <= 0) { 
      *lbuf = 0;
      return RC_OK;
    } else {

    }
  }

  start = _nano_time();
  ret = __read(os_fd,buf, *lbuf); 
  r = ret.rval;
  if (r == 0) {
    return RC_DISCONN;
  }

  if (r < 0) {
    if (ret.err == EAGAIN || ret.err == EWOULDBLOCK) {
      *lbuf = 0;
      return RC_OK;
    } else {
      return RC_FAIL; // unexpected error or end of file
    }
  } else {
    //    printf("Result return in %llu ns\n",_nano_time()-start);
    *lbuf = r;
  }

  return RC_OK;
}

/*
static
void force_os_to_send(int fd, int cork) {
  int cork_val = cork;
  int lcork_val = sizeof(cork_val);
  if (setsockopt(fd, SOL_SOCKET, TCP_CORK, &cork_val, lcork_val) < 0) {
    printf("Error: Failed to set cork %u\n",cork);
  }
}
*/

COO_DCL(OE, RC, write, uint fd, byte * buf, uint lbuf)
COO_DEF_RET_ARGS(OE, RC, write, uint fd; byte*buf;uint lbuf;,fd,buf,lbuf) {
  SimpleOE soe = (SimpleOE)this->impl;
  int writesofar = 0;
  int lastwrite = 0;
  int os_fd = 0;
  ull start = _nano_time();

  this->lock(soe->lock);
  os_fd = FDEntry_lookup(soe->filedescriptors,fd);
  this->unlock(soe->lock);

  if (os_fd < 0) return RC_FAIL;


  while(writesofar < lbuf && lastwrite >= 0) {
    struct timeval t = {0};
    fd_set wfds = {0};
    retval r = {0};
    FD_SET(os_fd, &wfds);
    t.tv_usec = 230;
    select(os_fd+1, 0, &wfds, 0, &t);

    r = __write(os_fd, buf+writesofar, lbuf-writesofar);
    lastwrite = r.rval;
    if ( lastwrite == -1) {
      if (r.err == EAGAIN) {
        lastwrite = 0;
        continue;
      } 

      printf("Error: %u %s os_fd=%u\n",r.err, strerror(r.err),os_fd);
      return -1;
    } 
    writesofar += lastwrite;
  }
  //  printf("Send time: %llu\n",_nano_time()-start);
  
  if (lastwrite < 0) {
    this->p("ERROR Failed to write");
    return RC_FAIL;
  }
  return writesofar;

}

/* proposal:
 *
 * file <path>          - open a file
 * net <ip or dns name> - make a client connection
 * listen://[<ip>:]port   - open server socket
 *
 * Returns non-zero on success, zero on failure.
 */
COO_DCL(OE, int, open, const char * name)
COO_DEF_RET_ARGS(OE, int, open , const char * name;, name) {
  uint lname = 0;
  SimpleOE soe = 0;
  uint res = 0;

  if (!name) return -1;
  lname = osal_strlen(name);

  soe = (SimpleOE)this->impl;
  if (!soe) return -128;
  
  if (lname > 5 && (mcmp((void*)"file ",(void*)name, 5) == 0)) {
    int fd = open(name+5, O_RDWR|O_CREAT|O_APPEND,S_IRWXU|S_IRGRP|S_IROTH);
    if (fd < 0) goto failure;
    
    this->lock(soe->lock);
    res = FDEntry_new(this,soe->filedescriptors,fd);
    this->unlock(soe->lock);
    return res;
  }
  
  // listen for incoming connections
  if (lname > 7 && mcmp(( void*)"listen ",( void *)name,7) == 0)   {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    char port[6] = {0};
    int reuse_addr_option = 1;
    struct sockaddr_in serv_addr = {0};

    
    if (setsockopt(server_fd, SOL_SOCKET,
		   SO_REUSEADDR,
		   (char *)&reuse_addr_option, sizeof(reuse_addr_option)) < 0 ) {
      close(server_fd);
      return 0;
    }

    sscanf(name+7,"%s", port);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(port)); 
    {
      socklen_t lserv_addr = sizeof(serv_addr);
      if ( bind ( server_fd, (struct sockaddr *)&serv_addr,	
		  lserv_addr) < 0) {
        close(server_fd);
        return 0;
      }
    }

    {
      uint flags = fcntl(server_fd, F_GETFL, 0);
      fcntl(server_fd, F_SETFL, flags | O_NONBLOCK  );
    }

    if (listen(server_fd, 20) != 0) {
      return 0;
    }
    this->lock(soe->lock); 
    res = FDEntry_new(this, soe->filedescriptors, server_fd);
    this->unlock(soe->lock);
    return res;
  }


  // ip address
  if (lname > 3 && mcmp((void *) "ip ", (void*)name, 3) == 0) {
    int socket_fd = 0;
    char ip[20]={0},port[6]={0};
    struct sockaddr_in addr = {0};
    sscanf(name+3,"%s %s", ip, port);
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) return 0;

    addr.sin_family = PF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(atoi(port));

    if (connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      close(socket_fd);
      return 0;
    }

    set_non_blocking(socket_fd);

    this->lock(soe->lock);
    res = FDEntry_new(this, soe->filedescriptors, socket_fd);
    this->unlock(soe->lock);
    return res;
  }

 failure:
  {
    char m[128] = {0};
    char * e = (char*)strerror(errno);
    sprintf(m,"Failed to create file descriptor, \"%s\" see error below:",name);
    this->syslog(OSAL_LOGLEVEL_FATAL, m);
    this->syslog(OSAL_LOGLEVEL_FATAL ,e);
  }
  if (soe) {
    this->unlock(soe->lock);
  }
  return 0;
}



static 
void set_non_blocking(int fd) {
  int keep_alice_opt = 1;
  int tcp_nodelay_opt = 1;
  int lopt = sizeof(int);
  int flags = fcntl(fd, F_GETFL, &flags, sizeof(flags));
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  /*
  if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keep_alice_opt, lopt ) < 0) {
    printf("No keep alive \n");
    // error but who cares if the socket works, this just might degrade performance 
  }
  
  if (setsockopt(fd, SOL_SOCKET, TCP_NODELAY, &tcp_nodelay_opt, lopt) < 0) {
    printf("TCP Nodelay failed ... \n");
    // May degrade performance but ...
  }
  */
}




COO_DCL(OE, int, accept, uint fd)
COO_DEF_RET_ARGS(OE, int, accept, uint fd;,fd) {
  SimpleOE soe = (SimpleOE)this->impl;
  int os_fd = 0;
  int os_client_fd = 0;
  uint res = 0;
  fd_set rd = {0};
  struct timeval timeout = {0};

  this->lock(soe->lock);
  os_fd = FDEntry_lookup(soe->filedescriptors, fd);
  this->unlock(soe->lock);

  if (os_fd < 0) return RC_FAIL;

  FD_ZERO(&rd);
  FD_SET(os_fd, &rd);
  timeout.tv_usec=100000;
  
  while(1) {
    select(os_fd+1,&rd,0,0,&timeout);
    os_client_fd = accept(os_fd,0,0);
    if (os_client_fd < 0) { 
      if (errno == EAGAIN) {
        usleep(0);
        this->yieldthread();
        continue;
      } else {
        return 0; // report failure
      } 
    } else {
      set_non_blocking(os_client_fd);
      break; // we have a client
    }
  }
  // RX ERR 4060083


  this->lock(soe->lock);
  res = FDEntry_new(this, soe->filedescriptors, os_client_fd);
  this->unlock(soe->lock);
  return res;
}

COO_DCL(OE, int, close, uint _fd)
COO_DEF_RET_ARGS(OE, int, close, uint _fd;, _fd) {
  int fd = 0;
  SimpleOE soe = (SimpleOE)this->impl;
  FDEntry ent = 0;
  
  if (!soe) { 
    return -1;
  }

  this->lock(soe->lock);

  fd = FDEntry_remove(this, soe->filedescriptors, _fd);
  if (fd >= 0) {
    close(fd);
  }

  this->unlock(soe->lock);


  return RC_OK;
}

COO_DCL(OE, ThreadID, newthread, ThreadFunction tf, void * args)
COO_DEF_RET_ARGS(OE, ThreadID, newthread, ThreadFunction tf; void * args;, tf, args) {

  pthread_t * t = this->getmem(sizeof(*t));
  SimpleOE soe = (SimpleOE)this->impl;
  ThreadID tid = 0;

  if (pthread_create(t, 0, tf, args) == 0) {
    pthread_t tt = pthread_self();
    this->lock(soe->lock);
    tid = soe->threads->size()+1;
    soe->threads->add_element(t);
    this->unlock(soe->lock);
    return tid;
  }
  this->syslog(OSAL_LOGLEVEL_WARN, "newthread: failed to create thread");
  return 0;
}

COO_DCL(OE, void, yieldthread)
COO_DEF_NORET_NOARGS(OE, yieldthread) {
  sched_yield();
  //  this->syslog(OSAL_LOGLEVEL_FATAL,"Yield thread is *NOT* implemented");
}

COO_DCL(OE, void *, jointhread, ThreadID tid)
COO_DEF_RET_ARGS(OE, void *, jointhread, ThreadID tid;,tid) {
  
  SimpleOE soe = (SimpleOE)this->impl;
  pthread_t * t = (pthread_t *)soe->threads->get_element(tid-1);
  void * p = 0;
  if (tid > soe->threads->size()+1) { 
    this->syslog(OSAL_LOGLEVEL_FATAL, "Failed to join thread"
		 ", thread id is greater than any active thread id.");
    return 0;
  }
  if (t) {
    pthread_join(*t,&p);
    this->lock(soe->lock);
    //    soe->threads->rem_element(tid-1);
    this->unlock(soe->lock);
  } else {
    this->p("Auch ! Thread not found");
  }
  
  return p;
}

COO_DCL(OE, uint, number_of_threads);
COO_DEF_RET_NOARGS(OE, uint, number_of_threads) {
  SimpleOE soe = (SimpleOE)this->impl;
  uint answer = 0;
  this->lock(soe->lock);
  answer = soe->threads->size()+1; // +1 is the main thread
  this->unlock(soe->lock);
  return answer;
}

COO_DCL(OE, MUTEX, newmutex )
COO_DEF_RET_NOARGS(OE, MUTEX, newmutex) {
  return Mutex_new(MUTEX_FREE);
}

COO_DCL(OE, void, destroymutex, MUTEX * m)
COO_DEF_NORET_ARGS(OE, destroymutex, MUTEX * m;,m) {
  Mutex_destroy(*m);
}

COO_DCL(OE, void, lock, MUTEX m)
COO_DEF_NORET_ARGS(OE, lock,  MUTEX m;,m) {
  //pid_t tid;
  //  char msg[64] = {0};
  //tid = syscall(SYS_gettid);
  //  sprintf(msg,"LOCK(%p) taken by THREAD(%d)",m,tid);
  //this->p(msg);
  Mutex_lock(m);
}

COO_DCL(OE, void, unlock,  MUTEX m) 
COO_DEF_NORET_ARGS(OE, unlock, MUTEX m;,m) {
  //pid_t tid;
  //char msg[64] = {0};
  //tid = syscall(SYS_gettid);
  //sprintf(msg,"LOCK(%p) released by THREAD(%d)",m,tid);
  //this->p(msg);
  Mutex_unlock(m);  
}

COO_DCL(OE, Cmaphore, newsemaphore, uint count)
COO_DEF_RET_ARGS(OE, Cmaphore, newsemaphore, uint count;,count) {
  return Cmaphore_new(this,count);
}

COO_DCL(OE, void, up, Cmaphore c)
COO_DEF_NORET_ARGS(OE, up, Cmaphore c;,c) {
  Cmaphore_up(c);
}

COO_DCL(OE, void, down, Cmaphore c)
COO_DEF_NORET_ARGS(OE, down, Cmaphore c;, c) {
  Cmaphore_down(c);
}

COO_DCL(OE, void, setloglevel, LogLevel level);
COO_DEF_NORET_ARGS(OE, setloglevel, LogLevel level;, level) {
  SimpleOE soe = (SimpleOE)this->impl;
  soe->loglevel = level;
}

COO_DCL(OE, void, set_log_file, char * filename);
COO_DEF_NORET_ARGS(OE, set_log_file, char * filename;,filename) {
  SimpleOE soe = (SimpleOE)this->impl;
  char fname[512] = {0};
  int fd = 0;
  osal_sprintf(fname,"file %s", filename);
  fd = this->open(fname);
  if (fd >= 0) {
    soe->log_file = fd;
  }
}

COO_DCL(OE, void, syslog, LogLevel level, const char * msg) 
COO_DEF_NORET_ARGS(OE, syslog, LogLevel level; const char * msg;, level, msg) {
  SimpleOE soe = (SimpleOE)this->impl;
  uint lmsg = osal_strlen(msg);
  if (level < soe->loglevel) return;

  // log to file?
  if (soe->log_file != 0) {
    char *s = (char*)this->getmem(lmsg+2);
    osal_sprintf(s,"%s\n",msg);
    this->write(soe->log_file,(byte*)s,lmsg+2);
    this->putmem(s);
    return;
  }

  switch(level) {
  case OSAL_LOGLEVEL_TRACE: {
    printf("\033[0;34m - log - \033[00m");
  } break;
  case OSAL_LOGLEVEL_DEBUG: {
    printf("\033[1;34m - log - \033[00m");
  } break;
  case OSAL_LOGLEVEL_WARN: {
    printf("\033[1;33m - log - \033[00m");
  } break;
  case OSAL_LOGLEVEL_FATAL: {
    printf("\033[1;31m - log - \033[00m");
  } break;
    
  default: {
    printf(" - log - ");
  } break;
  } 
  
  printf("%s\n", msg);
}

COO_DCL(OE, void, p, const char * msg)
COO_DEF_NORET_ARGS(OE, p, const char * msg;,msg) {
  this->syslog(OSAL_LOGLEVEL_WARN, msg);
}

COO_DCL(OE, void, destroysemaphore, Cmaphore * s)
COO_DEF_NORET_ARGS(OE, destroysemaphore, Cmaphore * s;,s) {
  Cmaphore_destroy(s);
}


COO_DCL(OE, char *, get_version)
COO_DEF_RET_NOARGS(OE, char *, get_version) {
  static char version_str[256] = {0};
  osal_sprintf(version_str,"%s %s %s",PACKAGE_STRING, CODENAME, BUILD_TIME);
  return version_str;
}

COO_DCL(OE, void, print,const char * fmt,...)
COO_DEF_NORET_ARGS(OE,print,const char * fmt;,fmt) {
  char buf[128] = {0};
  uint i = 0;
  byte * stack = (byte*)&this;
  printf("Address of fmt: %p address of this %p, diff %llu\n",&fmt,&this,(ull)&this-(ull)&fmt);
  printf("Values of this %p fmt=%p\n",this,fmt);
  for(i = 0; i < 256;++i) {
    if (i > 0 && i % 8 == 0) printf("\n");
    printf("%02x ",stack[i]);
  }
  printf("\n");
}


COO_DCL(OE, ThreadID, get_thread_id);
COO_DEF_RET_NOARGS(OE,ThreadID,get_thread_id) {
  SimpleOE simpleOE = (SimpleOE)this->impl;
  uint i = 0;
  pthread_t __t = pthread_self();
  this->lock(simpleOE->lock);
  for(i = 0;i < simpleOE->threads->size();++i) {
    pthread_t * cur = (pthread_t *)simpleOE->threads->get_element(i);
    if (cur) {
      if (pthread_equal(*cur,__t)) {
        this->unlock(simpleOE->lock);
        return i+1;
      }
    } else {
      printf("NULL :(\n");
    }
  }
  this->unlock(simpleOE->lock);
  return 0; // the main thread
}


COO_DCL(OE, void *, getSystemLibrary, const char * name);
COO_DEF_RET_ARGS(OE, void *, getSystemLibrary, const char * name;,name) {
	this->syslog(OSAL_LOGLEVEL_WARN,"System libraries are not implemented yet.");
	return 0;
}

COO_DCL(OE, void, provideSystemLibrary, const char * name, DefaultConstructor f);
COO_DEF_NORET_ARGS(OE, provideSystemLibrary ,const char * name; DefaultConstructor f;,name,f) {

}

OE OperatingEnvironment_LinuxNew() {
  SimpleOE simpleOE = 0;
  OE oe = 0;
  Memory mem = (Memory)LinuxMemoryNew();

  if (_static_lock == 0) {
    _static_lock = Mutex_new(MUTEX_FREE);
  }

  simpleOE = mem->alloc(sizeof(*simpleOE));
  
  oe = (OE)mem->alloc(sizeof(*oe));
  if (!oe) return 0;
  zeromem(oe,sizeof(*oe));
  
  InitializeCOO(10240*5	,mem);

  COO_ATTACH(OE, oe, set_log_file);
  COO_ATTACH(OE, oe, get_version);
  COO_ATTACH(OE, oe, destroysemaphore);
  COO_ATTACH(OE, oe, yieldthread);
  COO_ATTACH(OE, oe, accept);
  COO_ATTACH(OE, oe, getmem );
  COO_ATTACH(OE, oe, putmem );
  COO_ATTACH(OE, oe, read);
  COO_ATTACH(OE, oe, write);
  COO_ATTACH(OE, oe, open);
  COO_ATTACH(OE, oe, close);
  COO_ATTACH(OE, oe, newthread);
  COO_ATTACH(OE, oe, jointhread);
  COO_ATTACH(OE, oe, newmutex);
  COO_ATTACH(OE, oe, destroymutex);
  COO_ATTACH(OE, oe, lock);
  COO_ATTACH(OE, oe, unlock);
  COO_ATTACH(OE, oe, newsemaphore);
  COO_ATTACH(OE, oe, down);
  COO_ATTACH(OE, oe, up);
  COO_ATTACH(OE, oe, syslog);
  COO_ATTACH(OE, oe, p);
  COO_ATTACH(OE, oe, number_of_threads);
  COO_ATTACH(OE, oe, get_thread_id);
  COO_ATTACH(OE, oe, setloglevel);
  COO_ATTACH(OE, oe, print);
  COO_ATTACH(OE, oe, getSystemLibrary);
  COO_ATTACH(OE, oe, provideSystemLibrary);

  oe->impl = simpleOE;
  simpleOE->lock = oe->newmutex();

  simpleOE->mm = mem;
  simpleOE->threads = SingleLinkedList_new(oe);
  simpleOE->filedescriptors = SingleLinkedList_new(oe);
  simpleOE->loglevel = OSAL_LOGLEVEL_TRACE;
  simpleOE->log_file = 0;

  oe->p("************************************************************");
  oe->p("  #"PACKAGE_STRING" - "CODENAME );
  oe->p("   "BUILD_TIME);
  oe->p("************************************************************");
  oe->lock(_static_lock);
  no_osal_instance += 1;
  oe->unlock(_static_lock);
  return oe;
}

void OperatingEnvironment_LinuxDestroy( OE * oe) {
  if (!oe) return;
  if (!*oe) return;
 
  if ((*oe)->impl) {
    SimpleOE soe = (SimpleOE)(*oe)->impl;
    Memory m = soe->mm;
    SingleLinkedList_destroy( &soe->threads );
    SingleLinkedList_destroy( &soe->filedescriptors );
    Mutex_destroy( soe->lock );
    m->free((*oe)->impl);
    m->free(*oe);

    free(m);
  }
  Mutex_lock(_static_lock);
  no_osal_instance -= 1;
  if (!no_osal_instance) {
  //  coo_end();
    Mutex_unlock(_static_lock);
    Mutex_destroy(_static_lock);
  } else {
    Mutex_unlock(_static_lock);
  }
  
}

Data Data_shallow(byte * d, uint ld) {
  static struct _data_ shallow = {0};
  shallow.data =d;
  shallow.ldata = ld;
  return &shallow;
}

bool Data_equal(Data a, Data b) {
  if (a == 0 && b == 0) return True;
  if (a == 0) return False;
  if (b == 0) return False;

  if (a->ldata != b->ldata) return False;

  return mcmp(a->data,b->data,a->ldata) == 0 ? True : False;

}

Data Data_new(OE oe, uint size) {
  Data res = 0;
  if (!oe) return 0;

  res = (Data)oe->getmem(sizeof(*res));
  if (!res) return 0;

  res->data = (byte*)oe->getmem(size);
  if (!res->data) goto failure;
  
  res->ldata = size;
  return res;
 failure:
  Data_destroy(oe,&res);
  return 0;
}

Data Data_copy(OE oe, Data other) {
  Data res = 0;
  uint i = 0;

  if (!oe) return 0;
  if (!other) return 0;
  if (!other->data) return 0;
  
  res = oe->getmem(sizeof(*res));
  if (!res) return 0;

  res->data = (byte*)oe->getmem(other->ldata);
  if (!res->data) goto failure;
  
  i=other->ldata;
  while(i--) res->data[i] = other->data[i];

  res->ldata = other->ldata;
  return res;
 failure:
  Data_destroy(oe, &res );
  return res;
  
}

void Data_destroy(OE oe, Data * d) {
  if (d) {
    if (*d) {
      if ((*d)->data) {
        oe->putmem((*d)->data);
      }
      oe->putmem((*d));
      *d = 0;
    }
  }
  
}
