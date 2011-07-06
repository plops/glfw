#include <GL/glfw.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define NAN __builtin_nan("")

#define len(x) (sizeof(x)/sizeof(x[0]))

enum { CMDLEN=100,
       CIRCBUFLEN=10,
       CIRCBUFNUMELEMS=CIRCBUFLEN-1,
       MAXARGS=3,
       DOCSTRINGLEN=200,
};

// http://en.wikipedia.org/wiki/Circular_buffer
char *circbuf[CIRCBUFLEN];
int circwrite=0, // points where the next element will be written
  circread=0, // points where the next element will be read from
  circsize=CIRCBUFNUMELEMS;
void
circ_init()
{
  circwrite=0;
  circread=0;
  circsize=CIRCBUFNUMELEMS;
}

int emptyp(){ return circwrite==circread;}

int fullp(){ return ((circwrite+1)%CIRCBUFLEN)==circread;}

// increase but keep integer within 0..CIRCBUFLEN-1
int
inc(int *pos)
{
  int v=*pos;
  v++;
  if(v>=CIRCBUFLEN)
    v=0;
  (*pos)=v;
  return v;
}

int
push(char*s)
{
  if(fullp())
    printf("error circbuffer is full\n");
  circbuf[circwrite]=s;
  return inc(&circwrite);
}

char*
pop()
{
  if(emptyp()){
    printf("error buffer is empty");
    return 0;
  }
  char*ret=circbuf[circread];
  inc(&circread);
  return ret;
}



int running=GL_TRUE;

// Draw a vertical line that moves to a different vertical position in
// every frame. This is useful for verifying the synchronization of
// the drawing function, screen and camera exposure.
int show_calibration_stripes=1;

double
quit(double*ignore) 
{
  (void)ignore;
  running=GL_FALSE;
  return 0.0;
}

// this function can only be defined after cmd[]
double help(double*); 

// array that contains all functions that can be called from text interface
struct{ 
  char name[CMDLEN];
  int args;
  double (*fptr)(double*);
  char docstring[DOCSTRINGLEN];
}cmd[]={{"help",0,help,"exit main program"},
       	{"quit",0,quit,"list all possible commands"},};


// print synopsis of each possible command
double
help(double*ignore)
{
  (void)ignore;
  unsigned int i;
  printf("This is a very simple parser. Each command is defined by one word\n"
	 "followed by some single float parameters.\n"
	 "cmd 'number of parameters' .. Description\n");
  for(i=0;i<len(cmd);i++)
    printf("%s %d .. %s\n",cmd[i].name,cmd[i].args,cmd[i].docstring);
  return 0.0;
}

// find the index of string s within cmd[]
int
lookup(char*s) 
{
  unsigned int i;
  int n=strlen(s);
  if(s[n-1]=='\n') // ignore trailing return
    n--;
  for(i=0;i<len(cmd);i++)
    if(0==strncmp(s,cmd[i].name,n))
      return i;
  return -1;
}

// is character c a character that would occur in a float?
int isfloatchar(int c) 
{
  if(c=='+'||c=='-'||c=='.'||isdigit(c))
    return 1;
  return 0;
}

// Parse the first token of a line as a name name
// and find this name within cmd[]. Return the index to cmd[] 
int
parse_name(char*tok) 
{
  int fun_index=-1;
  if(tok){
    if(isalpha(tok[0])){
      fun_index=lookup(tok);
      // printf("+%s=%d+\n",tok,fun_index);
    }else{
      printf("error, expected function name\n");
      return -1;
    }
  }else{
    printf("error, expected some function name but got nothing");
    return -1;
  }
  return fun_index;
}

// Global counter of all commands that have been executed
unsigned long long cmd_number=0;

// Split a line into a space separated tokens. THe first token should
// be the name of a function, the following tokens are parsed as float
// parameters
double
parse_line(char*line)
{
  char *search=" ",*tok;
  if(!line)
    return NAN;
  tok=strtok(line,search);

  int fun_index=parse_name(tok);
  if(fun_index<0)
    return NAN;

  int arg_num=cmd[fun_index].args;
  int i;
  double args[MAXARGS];
  for(i=0;i<arg_num;i++){
    tok=strtok(0,search);
    if(!tok){
      printf("error, expected an argument");
      return NAN;
    }
    if(isfloatchar(tok[0])){
        char*endptr;
        double d=strtod(tok,&endptr);
        if(endptr==tok){
          printf("error, couldn't parse double\n");
          return NAN;
        }else
          args[i]=d;
        //printf("%g\n",d);
    }else{
      printf("error, expected digit or .+- but found %c\n",tok[0]);
      return NAN;
    }   
  }
  // fprintf(logfile,"%llu running %s\n",cmd_number++,cmd[fun_index].name);
  // fflush(logfile);
  return cmd[fun_index].fptr(args);
}

// Keyboard callback. Exit program when escape key has been pressed.
void GLFWCALL
keyhandler(int key,int action)
{
  if(action!=GLFW_PRESS)
    return;
  if(key==GLFW_KEY_ESC)
    running=GL_FALSE;
  return;
}


// OpenGL Modelview Matrix
float m[4*4];

// Initialized Modelview matrix to do the affine transform from Camera
// coordinates into LCoS coordinates.
void
init_matrix()
{
  float s=.8283338739,
    sx=s,sy=-s,phi=-3.1017227,
    cp=cos(phi),sp=sin(phi),
    tx=608.43307,
    ty=168.91883;
  m[0]=sx*cp;
  m[1]=-1*sx*sp;
  m[2]=0;
  m[3]=0;
  
  m[4]=sy*sp;
  m[5]=sy*cp;
  m[6]=0.;
  m[7]=0.;

  m[8]=0;
  m[9]=0;
  m[10]=1;
  m[11]=0;
  
  m[12]=tx;
  m[13]=ty;
  m[14]=0;
  m[15]=1;
}

int
check_stdin()
{
  fd_set rfds;
  struct timeval tv;
  
  // Watch stdin (fd 0) to see when it has input.
  FD_ZERO(&rfds);
  FD_SET(0,&rfds);

  // wait for 10 ms				
  tv.tv_sec=0;
  tv.tv_usec=10000;
  
  int retval = select(1,&rfds,NULL,NULL,&tv);

  if(retval==-1)
    perror("select()");
  
  return retval;
}

// Main program. Open a window. Continuously read commands from the
// command line interface and draw frames with 60Hz onto the
// screen. Commands for multiple frames can be cached within a ring
// buffer, so that a consistent frame rate can be maintained even if
// the control program doesn't respond within 16ms. The control
// program is written in Common Lisp and a 16ms time granularity cannot
// always be maintained, due to the time a garbage collection may
// take.
int
main()
{
  push("1test");
  push("2bla");
  push("3hdsfih");
  printf("%s\n",pop());
  printf("%s\n",pop());
  printf("%s\n",pop());
  return 0;

  // make sure frame rate update cycle is phase locked to vertical
  // refresh of screen. On Nvidia hardware this can be done by setting
  // the following environment variable.
  setenv("__GL_SYNC_TO_VBLANK","1",1); 
  
  if(!glfwInit())
    exit(EXIT_FAILURE);
  
  if(!glfwOpenWindow(1280,1024,8,8,8,
		     0,0,0,
		     GLFW_WINDOW
		     )){
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  
  glfwSetWindowTitle("LCoS");
  //glfwSetWindowPos(-8,-31);

  // use glfw method to sync to vertical refresh
  glfwSwapInterval(1);

  glfwSetKeyCallback(keyhandler);
  init_matrix();
  
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,1280,1024,0,-1,1);
  glMatrixMode(GL_MODELVIEW);

  int count=0;
  
  char s[CMDLEN],*line;

  


  while(running){
    if(check_stdin()>0){
      line=fgets(s,sizeof(s),stdin);
      printf("retval: %g\n", parse_line(line));
    }
    
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadMatrixf(m);
    
    if(show_calibration_stripes){
      float v = 100+20*((count++)%10);
      glRectf(v,0,v+2,400);
    }

    glfwSleep(1./68);
    glfwSwapBuffers();
  }
  

  glfwCloseWindow();

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
