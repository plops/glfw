// implementation of a deque, a queue for which elements can be added
// or removed only from the front or back

#include <malloc.h>
#include <string.h>
#include "queue.h"

#define e(q) do{fprintf(stderr,"error in file %s:%d in function %s, while calling %s\n",__FILE__,__LINE__,__FUNCTION__,q);}while(0)

typedef struct double_list_node double_list_node;
struct double_list_node {
  double_list_node *prev, *next;
  T data;
};

typedef struct double_list double_list;
struct double_list {
  double_list_node *first,*last;
};

double_list task_list;

void
insert_after(double_list list, 
	     double_list_node *node, 
	     double_list_node *new)
{
  new->prev = node;
  new->next = node->next;
  if(node->next)
    node->next->prev=new;
  else
    list.last=new;
  node->next=new;
}

void
insert_before(double_list list, 
	     double_list_node *node, 
	     double_list_node *new)
{
  new->prev = node->prev;
  new->next = node;
  if(node->prev)
    node->prev->next=new;
  else
    list.first=new;
  node->prev=new;
}

void insert_beginning(double_list list,
		      double_list_node *new)
{
  if(list.first)
    return insert_before(list,list.first,new);
  list.first=list.last=new;
  new->prev=0;
  new->next=0;
  printf("insert-beginning list=(0x%x 0x%x) new=0x%x\n",list.first,list.last,new);
}

T copy_T(const T*t)
{
  T r;
  r.type=t->type;
  int i;
  for(i=0;i<DATA_NUM;i++)
    r.data[i]=t->data[i];
  return r;
}

T* copy_T_into(T*dst,const T*src)
{
  dst->type=src->type;
  int i;
  for(i=0;i<DATA_NUM;i++)
    dst->data[i]=src->data[i];
  return dst;
}

double_list_node*
make_node(T t)
{
  double_list_node *new=malloc(sizeof(*new));
  copy_T_into(&(new->data),&t);
  e("make-node %d %d\n",new->data.type,new->data.data[0]);
  return new;
}

void push_front(T t)
{
  insert_beginning(task_list,make_node(t));
}


void insert_end(double_list list,
		double_list_node *new)
{
  if(list.last)
    return insert_after(list,list.last,new);
  insert_beginning(list,new);
}

void push_back(T t)
{
  insert_end(task_list,make_node(t));
}

T double_list_remove(double_list list,
	    double_list_node *node)
{
  T data={-1,{0,0,0,0}};
  if(!node){
    e("node to remove is null\n");
    return data;
  }
  if(node->prev)
    node->prev->next=node->next;
  else
    list.first = node->next;
  if(node->next)
    node->next->prev=node->prev;
  else
    list.last=node->prev;
  copy_T_into(&data,&(node->data));
  free(node);
  return data;
}

T pop_front()
{
  return double_list_remove(task_list,task_list.first);
}

T pop_back()
{
  return double_list_remove(task_list,task_list.last);
}


#ifdef TEST
int main()
{
  T bla={1,{2,3,4}};
  push_front(bla);
  T q=pop_front();
  printf("%d\n",q.type);
  

  return 0;
}
#endif
