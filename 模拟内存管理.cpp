
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<signal.h>
#include<iostream>


#define PROCESS_NAME_LEN 32			//进程名长度
#define MIN_SLICE 10				//默认碎片最小大小
#define DEFAULT_MEM_SIZE 1024		//默认初始内存大小

#define MA_FF 1						//首次适应算法对应的标识
#define MA_BF 2						//最佳适应算法对应的标识
#define MA_WF 3						//最差适应算法对应的标识

using namespace std;

int mem_size = DEFAULT_MEM_SIZE;				//内存初始大小使用默认值
int ma_algorithm = MA_FF;						//分配算法默认值选用首次适应算法
static int pid = 0;								//进程标识pid
int flag = 0;									//操作记录，如果初始内存大小进行了第一次修改，值置1，阻止内存大小被再次修改
	
void display_menu();							//显示菜单
int set_mem_size();								//设置初始内存大小
void set_algorithm();							//设置内存分配算法
int new_process();								//处理新进程的内存请求
void kill_process();							//处理旧进程
int display_mem_usage();						//显示内存分配情况
int allocate_mem(int size);						//从空闲区分配大小为size的内存
int dispose(struct allocated_block *free_ab);	//从内存已分配链表中删除节点free_ab
int free_mem(struct allocated_block *ab);		//释放已分配内存块ab的空间
struct allocated_block* find_process(int pid);	//根据pid查找内存已分配链表中对应的节点
void do_exit();									//退出程序
/*根据内存申请的大小按照相应算法将对应的空闲块从索引中删除，并将删除的空闲块返回，
如果没有适合的空闲块，则返回空值*/
struct free_block_type* Delete_FF(int size);	
struct free_block_type* Delete_BF(int size);
struct free_block_type* Delete_WF(int size);
/*按照相应算法将对应的空闲块从索引中删除*/
void Delete2_FF(struct free_block_type* old_free);
void Delete2_BF(struct free_block_type* old_free);
void Delete2_WF(struct free_block_type* old_free);
/*按照相应算法将新的空闲块new_free有序插入相应的索引中*/
void Insert_FF(struct free_block_type* new_free);
void Insert_BF(struct free_block_type* new_free);
void Insert_WF(struct free_block_type* new_free);

/*空闲块的数据结构*/
struct free_block_type {
	int size;							//空闲块大小
	int start_addr;						//空闲块起始地址
	struct free_block_type *FF_next;	//按照FF算法建立有序索引的指针域
	struct free_block_type *BF_next;	//按照BF算法建立有序索引的指针域
	struct free_block_type *WF_next;	//按照WF算法建立有序索引的指针域
};

struct free_block_type *free_block;		//空闲块头指针
/*已分配内存块的数据结构*/
struct allocated_block {
	int pid;							//使用内存的进程的pid
	int size;							//空间大小
	int start_addr;						//起始地址
	char process_name[PROCESS_NAME_LEN];//进程名
	struct allocated_block *next;		//建立已分配内存链表的指针域
};

struct allocated_block *allocated_block_head = NULL;	//已分配内存链表头指针

/*初始化内存空闲块链表*/
struct free_block_type* init_free_block(int mem_size) {
	struct free_block_type *fb;			//作为头结点
	struct free_block_type *fb2;		//链表有效内容从这里开始
	fb = (struct free_block_type*)malloc(sizeof(struct free_block_type));
	if (fb == NULL) {
		printf("No mem\n");
		return NULL;
	}
	fb2 = (struct free_block_type*)malloc(sizeof(struct free_block_type));
	if (fb == NULL) {
		printf("No mem\n");
		return NULL;
	}
	fb2->size = mem_size;
	fb2->start_addr = 0;
	fb2->BF_next = NULL;
	fb2->FF_next = NULL;
	fb2->WF_next = NULL;
	fb->BF_next = fb2;
	fb->FF_next = fb2;
	fb->WF_next = fb2;
	return fb;
}

/*主函数*/
int main() {
	char choice;		//变量值对应不同菜单选项
	pid = 0;
	int f;				//见下面的while循环
	free_block = init_free_block(mem_size);
	for (;;) {
		display_menu();
		fflush(stdin);
		choice = getchar();
		f = 1;		//使用f实现菜单的重复显示
		while (f) {
			f= 0;
			switch (choice) {
			case '1':set_mem_size(); break;
			case '2':set_algorithm(); break;
			case '3':new_process(); flag = 1; break;
			case '4':kill_process(); flag = 1; break;;
			case '5':display_mem_usage(); flag = 1; break;
			case '0':do_exit();
			default:f = 1; choice = getchar(); break;
			}
		}
	}
	return 0;
}

/*显示菜单
菜单选项包括：
	1、设置初始内存大小
	2、设置内存分配算法
	3、为新的进程开辟内存空间
	4、取消一个进程并释放相应空间
	5、显示内存使用情况
	6、（0）结束程序
*/
void display_menu() {
	printf("\n");
	printf("1 - Set memory size (default=%d)\n", DEFAULT_MEM_SIZE);
	printf("2 - Select memory allocation algorithm\n");
	printf("3 - New process \n");
	printf("4 - Terminate a process \n");
	printf("5 - Display memory usage \n");
	printf("0 - Exit\n");
}

/*设置内存初始大小*/
int set_mem_size() {
	int size;
	//阻止内存大小重复设置
	if (flag != 0) {
		printf("Cannot set memory size again\n");
		return 0;
	}
	printf("Total memory size = ");
	cin >> size;
	if (size > 0) {
		mem_size = size;
		free_block->FF_next->size = mem_size;
	}
	flag = 1;
	return 1;
}

/*设置内存分配算法*/
void set_algorithm() {
	int algorithm;
	printf("\t 1 - First Fit\n");
	printf("\t 2 - Best Fit\n");
	printf("\t 3 - Worst Fit\n");
	cin >> algorithm;
	switch (algorithm) {
	case 1:ma_algorithm = MA_FF; break;
	case 2:ma_algorithm = MA_BF; break;
	case 3:ma_algorithm = MA_WF; break;
	default:printf("输入有误\n"); return;
	}
}

/*处理新进程的内存请求*/
int new_process() {
	struct allocated_block *ab;
	int size;
	int ret;

	pid++;	//获取新的pid号
	
	cout << "PROCESS" << pid << endl;
	printf("Memory for %d:", pid);
	cin >> size;
	if (size <= 0) {
		printf("设置大小非法\n");
		pid--;
		ab = NULL;
		return -1;
	}
	//为新进程分配内存，如果失败，返回-1
	ret = allocate_mem(size);
	if (ret == -1) {
		printf("Allocation fail\n");
		return -1;
	}
	ab = (struct allocated_block *)malloc(sizeof(struct allocated_block));
	if (!ab)
		exit(-5);
	ab->next = NULL;
	ab->size = size;
	ab->start_addr = ret;
	ab->pid = pid;
	//将新的已分配内存块头插在已分配内存链表中
	if (allocated_block_head == NULL) {
		allocated_block_head = ab;
		return 1;
	}
	else{
		ab->next = allocated_block_head;
		allocated_block_head = ab;
		return 2;
	}
	return 3;
}

/*杀死进程，并回收旧进程的内存*/
void kill_process() {
	struct allocated_block *ab;
	int pid;
	printf("Kill Process, pid=");
	cin >> pid;
	//scanf("%d", &pid);
	ab = find_process(pid);		//进程对应的已分配内存块节点
	if (ab != NULL) {
		free_mem(ab);			//回收旧进程的内存
		dispose(ab);			//从内存已分配链表中删除节点ab
	}
	else
		printf("Process %d dosn't exist.", pid);
}

/*显示内存分配情况*/
int display_mem_usage() {
	struct free_block_type *fbt = free_block;
	struct free_block_type * tmp = fbt;
	struct allocated_block *ab = allocated_block_head;
	struct allocated_block *tmp2 = allocated_block_head;
	if (fbt == NULL)
		return -1;
	printf("------------------------------Memory Information ---------------------------------\n");
	/*按内存块地址递增顺序显示内存分配情况*/
	printf("  * Free Memory Block: \n");
	printf("	  [start_addr]		   [ size ]\n");
	while (tmp->FF_next != NULL) {
		cout <<"		"<< tmp->FF_next->start_addr <<"		    "<<tmp->FF_next->size << endl;
		tmp = tmp->FF_next;
	}
	printf("\n");
	/*按进程创建时间由短到长显示内存分配情况*/
	printf("  * Allocated Memory Block: \n");
	printf("	  [start_addr ]		    [ size ]	  [process_pid]\n");
	while (tmp2 != NULL) {
		cout << "		" << tmp2->start_addr << "			" << tmp2->size<<"		" <<tmp2->pid<< endl;
		tmp2 = tmp2->next;
	}
	printf("----------------------------------------------------------------------------------\n");
	return 0;
}

void Insert_FF(struct free_block_type* new_free) {
	struct free_block_type *tmp;
	struct free_block_type *old_free;
	tmp = free_block;
	while (tmp->FF_next != NULL && ((tmp->FF_next->start_addr+tmp->FF_next->size) <= new_free->start_addr )) {
		tmp = tmp->FF_next;
	}

	if (tmp->FF_next == NULL) {
		tmp->FF_next = new_free;
		new_free->FF_next = NULL;
		if (tmp != free_block && (tmp->start_addr + tmp->size == new_free->start_addr)) {
			new_free->start_addr = tmp->start_addr;
			new_free->size = tmp->size + new_free->size;
			Delete2_FF(tmp);
			Delete2_BF(tmp);
			Delete2_WF(tmp);
			free(tmp);
			tmp = NULL;
		}
	}
	else {
		new_free->FF_next = tmp->FF_next;
		tmp->FF_next = new_free;
		if (tmp != free_block && tmp->start_addr + tmp->size == new_free->start_addr) {
			new_free->start_addr = tmp->start_addr;
			new_free->size = tmp->size + new_free->size;
			Delete2_FF(tmp);
			Delete2_BF(tmp);
			Delete2_WF(tmp);
			free(tmp);
			tmp = NULL;
		}
		if (new_free->start_addr + new_free->size == new_free->FF_next->start_addr) {
			new_free->size = new_free->FF_next->size + new_free->size;
			old_free = new_free->FF_next;
			Delete2_FF(old_free);
			Delete2_BF(old_free);
			Delete2_WF(old_free);
			free(old_free);
			old_free = NULL;
		}
	}
}
struct free_block_type* Delete_FF(int size) {
	struct free_block_type *tmp;
	struct free_block_type *old_free;
	tmp = free_block;
	while (tmp->FF_next != NULL && tmp->FF_next->size < size)
		tmp = tmp->FF_next;
	if (tmp->FF_next == NULL) {
		tmp->FF_next = NULL;
		return NULL;
	}
	else {
		old_free = tmp->FF_next;
		tmp->FF_next = old_free->FF_next;
		return old_free;
	}
}
void Delete2_FF(struct free_block_type *old_free) {
	struct free_block_type *tmp;
	tmp = free_block;
	while (tmp->FF_next != old_free)
		tmp = tmp->FF_next;
	tmp->FF_next = old_free->FF_next;
}

void Insert_BF(struct free_block_type* new_free) {
	struct free_block_type *tmp;
	tmp = free_block;
	while (tmp->BF_next != NULL && tmp->BF_next->size < new_free->size)
		tmp = tmp->BF_next;
	if (tmp->BF_next == NULL) {
		tmp->BF_next = new_free;
		new_free->BF_next = NULL;
	}
	else {
		new_free->BF_next = tmp->BF_next;
		tmp->BF_next = new_free;
	}	
}
struct free_block_type* Delete_BF(int size) {
	struct free_block_type* tmp;
	struct free_block_type* old_free;
	tmp = free_block;
	while (tmp->BF_next != NULL && tmp->BF_next->size < size)
		tmp = tmp->BF_next;
	if (tmp->BF_next == NULL) {
		tmp->BF_next = NULL;
		return NULL;
	}
	else {
		old_free = tmp->BF_next;
		tmp->BF_next = old_free->BF_next;
		return old_free;
	}
}

void Delete2_BF(struct free_block_type *old_free) {
	struct free_block_type *tmp;
	tmp = free_block;
	while ( tmp->BF_next != NULL && tmp->BF_next != old_free)
		tmp = tmp->BF_next;
	tmp->BF_next = old_free->BF_next;
}

void Insert_WF(struct free_block_type *new_free) {
	struct free_block_type *tmp;
	tmp = free_block;
	while (tmp->WF_next != NULL && tmp->WF_next->size > new_free->size)
		tmp = tmp->WF_next;
	if (tmp->WF_next == NULL) {
		tmp->WF_next = new_free;
		new_free->WF_next = NULL;
	}
	else {
		new_free->WF_next = tmp->WF_next;
		tmp->WF_next = new_free;
	}
}

struct free_block_type* Delete_WF(int size) {
	struct free_block_type *tmp;
	struct free_block_type *old_free;
	tmp = free_block;
	if (tmp->WF_next->size < size)
		return NULL;
	old_free = tmp->WF_next;
	tmp->WF_next = old_free->WF_next;
	return old_free;
}

void Delete2_WF(struct free_block_type *old_free) {
	struct free_block_type *tmp;
	tmp = free_block;
	while (tmp->WF_next != NULL && tmp->WF_next != old_free)
		tmp = tmp->WF_next;
	tmp->WF_next = old_free->WF_next;
}

//从空闲区分配大小为size的内存
int allocate_mem(int size) {
	struct free_block_type* new_free;
	struct free_block_type* old_free = NULL;
	int start_addr;
	switch (ma_algorithm) {
	case MA_FF:
		old_free = Delete_FF(size);
		if(old_free == NULL){
			printf("空闲区空间不足\n");
			return -1;
		}
		Delete2_BF(old_free);
		Delete2_WF(old_free);
		break;
	case MA_BF:
		old_free = Delete_BF(size);
		if (old_free == NULL) {
			printf("空闲区空间不足\n");
			return -1;
		}
		Delete2_FF(old_free);
		Delete2_WF(old_free);
		break;
	case MA_WF:
		old_free = Delete_WF(size);
		if (old_free == NULL) {
			printf("空闲区空间不足\n");
			return -1;
		}
		Delete2_BF(old_free);
		Delete2_FF(old_free);
		break;
	}
	start_addr = old_free->start_addr;
	new_free = (struct free_block_type*)malloc(sizeof(struct free_block_type));
	new_free->start_addr = old_free->start_addr + size;
	new_free->size = old_free->size - size;
	Insert_FF(new_free);
	Insert_BF(new_free);
	Insert_WF(new_free);
	free(old_free);
	old_free = NULL;
	return start_addr;
}

//释放已分配内存块ab的空间
int free_mem(struct allocated_block *ab) {
	struct free_block_type* new_free;
	new_free = (struct free_block_type*)malloc(sizeof(struct free_block_type));
	new_free->start_addr = ab->start_addr;
	new_free->size = ab->size;
	Insert_FF(new_free);
	Insert_BF(new_free);
	Insert_WF(new_free);
	return 0;
}

//从内存已分配链表中删除节点free_ab
int dispose(struct allocated_block *free_ab) {
	struct allocated_block *pre, *ab;
	if (free_ab == allocated_block_head) {
		allocated_block_head = allocated_block_head->next;
		free(free_ab);
		return 1;
	}
	pre = allocated_block_head;
	ab = allocated_block_head->next;
	while (ab != free_ab) {
		pre = ab;
		ab = ab->next;
	}
	pre->next = ab->next;
	free(ab);
	return 2;
}
void do_exit() {
	exit(0);
}

//根据pid查找内存已分配链表中对应的节点
struct allocated_block* find_process(int pid) {
	struct allocated_block *p;
	p = allocated_block_head;
	while (p != NULL) {
		if (p->pid == pid)
			return p;
		else
			p = p->next;
	}
	return NULL;
}