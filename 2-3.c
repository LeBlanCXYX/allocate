#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#define PROCESS_NAME_LEN 32    //进程名长度
#define MIN_SLICE 10           //最小碎片的大小
#define DEFAULT_MEM_SIZE 1024  //内存大小
#define DEFAULT_MEM_START 0    //起始位置
//内存分配算法
#define MA_FF 1
#define MA_BF 2
#define MA_WF 3
//描述每一个空闲块的数据结构
struct free_block_type
{
    int size;        //空闲块大小
    int start_addr;  //空闲块起始位置
    struct free_block_type *next;  //指向下一个空闲块
};
//指向内存中空闲块链表的首地址
struct free_block_type *free_block= NULL;
//每个进程分配到的内存块的描述
struct allocated_block
{
    int pid;
    int size;  //进程大小
    int start_addr;   //进程分配到的内存块的起始地址
    char process_name[PROCESS_NAME_LEN];  //进程名
    struct allocated_block *next;   //指向下一个进程控制块
};
//进程分配内存块链表的首指针
struct allocated_block *allocated_block_head= NULL;
int free_block_count= 0;    //空闲块的个数
int mem_size= DEFAULT_MEM_SIZE;   //内存大小
int current_free_mem_size= 0;     //当前空闲内存大小
int ma_algorithm= MA_FF;          //当前分配算法
static int pid= 0;
int flag= 0;       //设置内存大小标志，表示内存大小是否设置

struct free_block_type* init_free_block(int mem_size);
void display_menu();
int set_mem_size();
void set_algorithm();
void rearrange(int algorithm);
int rearrange_WF();
int rearrange_BF();
int rearrange_FF();
int new_process();
int allocate_mem(struct allocated_block *ab);
void kill_process();
int free_mem(struct allocated_block *ab);
int dispose(struct allocated_block *free_ab);
int display_mem_usage();
struct allocated_block* find_process(int pid);
int do_exit();
int allocate(struct free_block_type *pre,struct free_block_type *allocate_free_nlock,struct allocated_block *ab);
int mem_retrench(struct allocated_block *ab);
int main(){
 char choice; pid=0;
 free_block = init_free_block(mem_size); //初始化空闲区
 while(1) {
 display_menu(); //显示菜单
 fflush(stdin);
 choice=getchar(); //获取用户输入
 switch(choice){
 case '1': set_mem_size(); break; //设置内存大小
 case '2': set_algorithm();flag=1; break;//设置算法
 case '3': new_process(); flag=1; break;//创建新进程
 case '4': kill_process(); flag=1; break;//删除进程
 case '5': display_mem_usage(); flag=1; break; //显示内存使用
 case '0': do_exit(); exit(0);//释放链表并退出
 default: break; } } 
 return 0;
}

//初始化空闲块，默认为一块，可以指定大小及起始地址
struct free_block_type* init_free_block(int mem_size)
{
    struct free_block_type *fb;
    fb= (struct free_block_type*)malloc(sizeof(struct free_block_type));
    if(fb== NULL)
    {
        printf("No mem\n");
        return NULL;
    }
    current_free_mem_size= mem_size;
    fb->size= mem_size;
    fb->start_addr= DEFAULT_MEM_START;
    fb->next= NULL;    //首地址指向NULL
    free_block_count=1;
    return fb;
}

//设置内存的大小
int set_mem_size()
{
    int size;
    if(flag!= 0)     //防止重复设置
    {
        printf("you've already set the memory size\n");
        return 0;
    }
    printf("Total memory size=");
    for(;;)
    {
        scanf("%d",&size);
        getchar();
        if(size> 0)
        {
            current_free_mem_size= size;
            mem_size= size;
            free_block->size= mem_size;
            break;
        }
        else
        {
            printf("The size must greater than zero!Please input again:");
        }
    }
    flag= 1;    //内存大小已经设置
    return 1;
}

//设置当前的分配算法
void set_algorithm()
{
    int algorithm;
    //system("clear");
    printf("\t1-First Fit\n");   //首次适应算法
    printf("\t2-Best Fit\n");    //最佳适应算法
    printf("\t3-Worst Fit\n");   //最坏适应算法
    printf("Please choose(1-3):");
    for(;;)
    {
        scanf("%d",&algorithm);
        getchar();
        if(algorithm>= 1&&algorithm<= 3)
        {
            ma_algorithm= algorithm;
            break;
        }
        else
        {
            printf("\nCannot input %d,Please input 1-3:",algorithm);
        }
    }
    rearrange(ma_algorithm);
}

void rearrange(int algorithm)
{
    switch(algorithm)
    {
        case MA_FF:
            rearrange_FF();
            break;
        case MA_BF:
            rearrange_BF();
            break;
        case MA_WF:
            rearrange_WF();
            break;
    }
}

//按FF算法重新整理内存空闲块链表，按空闲块首地址排序
int rearrange_FF()
{
    struct free_block_type *head= free_block;
    struct free_block_type *forehand,*pre,*rear;
    int i;
    if(head== NULL)
        return -1;
    for(i= 0;i< free_block_count-1;i++)
    {
        forehand= head;
        pre= forehand->next;
        rear= pre->next;
        if(forehand== head&&forehand->start_addr>= pre->start_addr)
            {
                //比较空闲链表中第一个空闲块与第二个空闲块的开始地址的大小
                head->next= pre->next;
                pre->next= head;
                head= pre;
                forehand= head;
                pre= forehand->next;
            }
        while(pre->next!= NULL)
        {  
            if(pre->start_addr>= rear->start_addr)
            {
                //比较链表中其它相邻两个结点的开始地址的大小
                pre->next= rear->next;
                forehand->next= rear;
                rear->next= pre;
                forehand= rear;
                rear= pre->next;
            }
            else
            {
                forehand= pre;
                pre= rear;
                rear= rear->next;
            }
        }
    }
    free_block=head;
    return 0;
}

//按BF算法重新整理内存空闲块链表，按空闲块大小从小到大排序
int rearrange_BF()
{
    struct free_block_type *head= free_block;
    struct free_block_type *forehand,*pre,*rear;
    int i;
    if(head== NULL)
        return -1;
    for(i= 0;i< free_block_count-1;i++)
    {
        forehand= head;
        pre= forehand->next;
        rear= pre->next;
        if(forehand== head&&forehand->size>= pre->size)//排在考前的空闲块的size比其之后的更大，需要交换
            {
                //比较空闲链表中第一个空闲块与第二个空闲块的空间的大小，size从小到大排序
                head->next= pre->next;
                pre->next= head;
                head= pre;
                forehand= head;
                pre= forehand->next;
                rear= pre->next;
            }
        while(pre->next!= NULL)
        {
            
            if(pre->size>= rear->size)
            {
                //比较链表中其它相邻两个结点的空间的大小
                pre->next= rear->next;
                forehand->next= rear;
                rear->next= pre;
                forehand= rear;
                rear= pre->next;
            }
            else
            {
                forehand= pre;
                pre= rear;
                rear= rear->next;
            }
        }
    }
    free_block=head;
    return 0;
}

//按WF算法重新整理内存空闲块链表，按空闲块大小从大到小排序
int rearrange_WF()
{
    struct free_block_type *head= free_block;
    struct free_block_type *forehand,*pre,*rear;
    int i;
    if(head== NULL)
        return -1;
    for(i= 0;i< free_block_count-1;i++)
    {
        forehand= head;
        pre= forehand->next;
        rear= pre->next;
        if(forehand== head&&forehand->size<=pre->size)
            {
                //比较空闲链表中第一个空闲块与第二个空闲块空间的大小,第一个空闲块比第二个空闲块小时交换
                head->next= pre->next;
                pre->next= head;
                head= pre;
                forehand= head;
                pre= forehand->next;
                rear= pre->next;
            }
        while(pre->next!= NULL)
        {
            
            if(pre->size<= rear->size)
            {
                //比较链表中其它相邻两个结点的空间的大小
                pre->next= rear->next;
                forehand->next= rear;
                rear->next= pre;
                forehand= rear;
                rear= pre->next;
            }
            else
            {
                forehand= pre;
                pre= rear;
                rear= rear->next;
            }
        }
    }
    free_block=head;
    return 0;
}


//创建一个新进程
int new_process()
{
    struct allocated_block *ab;
    int size;
    int ret;
    ab= (struct allocated_block*)malloc(sizeof(struct allocated_block));
    if(!ab)
        exit(-5);
    ab->next= NULL;
    pid++;
    sprintf(ab->process_name,"PROCESS-%02d",pid);//将格式化的数据写入某字符串中
    ab->pid= pid;
    printf("Memory for %s:",ab->process_name);
    for(;;)
    {
        scanf("%d",&size);
        getchar();
        if(size> 0)
        {
            ab->size= size;
            break;
        }
        else
            printf("The size have to greater than zero!Please input again!");
    }
    ret= allocate_mem(ab);
    if((ret== 1)&&(allocated_block_head== NULL))  //如果此时未赋值，则赋值
    {
        allocated_block_head= ab;
        return 1;
    }
    else if(ret== 1)  //分配成功，将该已分配块的描述插入已分配链表
    {
        struct allocated_block *p=allocated_block_head;
        while(p->next!=NULL)p=p->next;
        p->next=ab;
        return 2;
    }
    else if(ret== -1)  //分配不成功
    {
        printf("Allocation fail.\n");
        free(ab);
        return -1;
    }
    return 3;
}

//分配内存模块
int allocate_mem(struct allocated_block *ab)
{
    int ret;
    struct free_block_type *pre= NULL,*f= free_block;
    if(f== NULL)
        return -1;
    while(f!= NULL)
    {
        if(f->size>= ab->size)
        {
            ret= allocate(pre,f,ab);
            rearrange(ma_algorithm);
            return ret;
        }
        pre= f;
        f= pre->next;
    }
    if(f== NULL&&current_free_mem_size> ab->size)
        ret= mem_retrench(ab);
    else
        {
            ret= -2;
            printf("no such spare rooms!!\n");
        }
    return ret;
}

//给新进程分配内存空间
int allocate(struct free_block_type *pre,struct free_block_type *allocate_free_block,struct allocated_block *ab)
{
    //struct allocated_block *p= allocated_block_head;
    ab->start_addr= allocate_free_block->start_addr;
    if(allocate_free_block->size-ab->size< MIN_SLICE)
    {
        ab->size= allocate_free_block->size;
        if(pre!= NULL)
        {
            pre->next= allocate_free_block->next;
        }
        else
        {
            free_block= allocate_free_block->next;
        }
        free(allocate_free_block);
        free_block_count--;
    }
    else
    {
        allocate_free_block->start_addr+= ab->size;
        allocate_free_block->size-= ab->size;
    }
    /*if(p== NULL)
    {
        allocated_block_head= ab;
    }
    else
    {
        while(p->next!= NULL)
            p= p->next;
        p->next= ab;
    }*/
    current_free_mem_size-= ab->size;
    if(current_free_mem_size== 0)
        free_block= NULL;
    return 1;
}

//通过内存紧缩技术给新进程分配内存空间
int mem_retrench(struct allocated_block *ab)
{
    struct allocated_block *allocated_work,*allocated_pre= allocated_block_head;
    struct free_block_type *free_rare,*free_work= free_block->next;
    if(allocated_pre== NULL)
        return -1;
    allocated_pre->start_addr= 0;
    allocated_work= allocated_pre->next;
    while(allocated_work!= NULL)
    {
        allocated_work->start_addr= allocated_pre->start_addr+ allocated_pre->size;
        allocated_pre= allocated_work;
        allocated_work= allocated_work->next;
    }
    free_block->start_addr= allocated_pre->start_addr+ allocated_pre->size;
    free_block->size= current_free_mem_size;
    free_block->next=NULL;
    free_rare= free_work->next;
    while(free_rare!= NULL)
    {
        free(free_work);
        free_work= free_rare;
        free_rare=free_rare->next;
        if(free_rare== NULL)
            free(free_work);
    }
    free_block_count=1;
    allocate(NULL,free_block,ab);
    return 1;
}

//删除进程，归还分配的存储空间，并删除描述该进程内存分配的结点
void kill_process()
{
    struct allocated_block *ab;
    int pid;
    printf("Kill Process,pid=");
    scanf("%d",&pid);
    getchar();
    ab= find_process(pid);
    if(ab!= NULL)
    {
        free_mem(ab);  //释放ab所表示的分配区(free区)
        dispose(ab);   //释放ab数据结构结点
    }
}

struct allocated_block* find_process(int pid){
    struct allocated_block *p=allocated_block_head;
    while(p!=NULL){
        if(p->pid==pid)return p;
        p=p->next;
    }
    printf("no such a pid process!\n");
    return NULL;
}

//将ab所表示的已分配区归还，并进行可能的合并,对空闲块进行操作
int free_mem(struct allocated_block *ab){
    current_free_mem_size+= ab->size;
    struct free_block_type *pre,*work,*rear;
    work=(struct free_block_type*)malloc(sizeof(struct free_block_type));
    work->start_addr=ab->start_addr;
    work->size=ab->size;
    work->next=NULL;
    pre=free_block;
    if(pre==NULL){free_block=work;free_block_count++;return 1;}
    rear=pre->next;
    while(rear!=NULL){
        pre=rear;
        rear=rear->next;
    }
    pre->next=work;
    free_block_count++;
    rearrange_FF();
    pre=free_block;
    rear=free_block->next;
    while(rear!=NULL)
    {
        while(pre->start_addr+pre->size==rear->start_addr)
        {  
            pre->size+=rear->size;
            pre->next=rear->next;
            free(rear);
            free_block_count--;
            rear=pre->next;
            if(rear==NULL)break;
        }
        if(rear!=NULL){
        pre=rear;
        rear=rear->next;
        }
    }
    rearrange(ma_algorithm);
    return 1;
}

int dispose(struct allocated_block *free_ab){
    struct allocated_block *pre,*work;
    pre=allocated_block_head;
    if(pre==NULL)return -1;
    //如果是第一个节点
    if(pre->pid==free_ab->pid){allocated_block_head=pre->next;free(pre);return 1;}
    work=pre->next;
    while(work!=NULL){
        if(work->pid==free_ab->pid){
            pre->next=work->next;
            free(work);
            return 1;
        }
        else{
            pre=work;
            work=work->next;
        }
    }
}

//显示当前内存的使用情况，包括空闲区的情况和已经分配的情况
int display_mem_usage()
{
    struct free_block_type *fbt= free_block;
    struct allocated_block *ab= allocated_block_head;
    printf("-------------------------------------------------------------\n");
    printf("Free Memory:\n");
    printf("%20s %20s\n","     start_addr","     size");
    while(fbt!= NULL)
    {
        printf("%20d %20d\n",fbt->start_addr,fbt->size);
        fbt= fbt->next;
    }
    printf("\nUsed Memory:\n");
    printf("%10s %20s %10s %10s\n","PID","ProcessName","start_addr","size");
    while(ab!= NULL)
    {
        printf("%10d %20s %10d %10d\n",ab->pid,ab->process_name,ab->start_addr,ab->size);
        ab= ab->next;
    }
    printf("-------------------------------------------------------------\n");
    return 1;
}


int do_exit()
{
    struct allocated_block *allocated_ab,*allocated_pre;
    struct free_block_type *free_ab,*free_pre;
    free_pre= free_block;
    allocated_pre= allocated_block_head;
    if(free_pre!= NULL)
    {
        free_ab= free_pre->next;
        while(free_ab!= NULL)
        {
            free(free_pre);
            free_pre= free_ab;
            free_ab= free_ab->next;
        }
        free(free_pre);
    }
    if(allocated_pre!= NULL)
    {
        allocated_ab= allocated_pre->next;
        while(allocated_ab!= NULL)
        {
            free(allocated_pre);
            allocated_pre= allocated_ab;
            allocated_ab= allocated_ab->next;
        }
        free(allocated_pre);
    }
    return 0;
}

//显示主菜单
void display_menu()
{
    printf("\n");
    //system("clear");
    printf("1-Set memory size(default= %d)\n",DEFAULT_MEM_SIZE);
    printf("2-Select memory allocation algorithm\n");
    printf("3-New process\n");
    printf("4-Terminate a process\n");
    printf("5-Display memory usage\n");
    printf("0-Exit\n");
}