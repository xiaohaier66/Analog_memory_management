
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<signal.h>
#include<iostream>


#define PROCESS_NAME_LEN 32			//����������
#define MIN_SLICE 10				//Ĭ����Ƭ��С��С
#define DEFAULT_MEM_SIZE 1024		//Ĭ�ϳ�ʼ�ڴ��С

#define MA_FF 1						//�״���Ӧ�㷨��Ӧ�ı�ʶ
#define MA_BF 2						//�����Ӧ�㷨��Ӧ�ı�ʶ
#define MA_WF 3						//�����Ӧ�㷨��Ӧ�ı�ʶ

using namespace std;

int mem_size = DEFAULT_MEM_SIZE;				//�ڴ��ʼ��Сʹ��Ĭ��ֵ
int ma_algorithm = MA_FF;						//�����㷨Ĭ��ֵѡ���״���Ӧ�㷨
static int pid = 0;								//���̱�ʶpid
int flag = 0;									//������¼�������ʼ�ڴ��С�����˵�һ���޸ģ�ֵ��1����ֹ�ڴ��С���ٴ��޸�
	
void display_menu();							//��ʾ�˵�
int set_mem_size();								//���ó�ʼ�ڴ��С
void set_algorithm();							//�����ڴ�����㷨
int new_process();								//�����½��̵��ڴ�����
void kill_process();							//����ɽ���
int display_mem_usage();						//��ʾ�ڴ�������
int allocate_mem(int size);						//�ӿ����������СΪsize���ڴ�
int dispose(struct allocated_block *free_ab);	//���ڴ��ѷ���������ɾ���ڵ�free_ab
int free_mem(struct allocated_block *ab);		//�ͷ��ѷ����ڴ��ab�Ŀռ�
struct allocated_block* find_process(int pid);	//����pid�����ڴ��ѷ��������ж�Ӧ�Ľڵ�
void do_exit();									//�˳�����
/*�����ڴ�����Ĵ�С������Ӧ�㷨����Ӧ�Ŀ��п��������ɾ��������ɾ���Ŀ��п鷵�أ�
���û���ʺϵĿ��п飬�򷵻ؿ�ֵ*/
struct free_block_type* Delete_FF(int size);	
struct free_block_type* Delete_BF(int size);
struct free_block_type* Delete_WF(int size);
/*������Ӧ�㷨����Ӧ�Ŀ��п��������ɾ��*/
void Delete2_FF(struct free_block_type* old_free);
void Delete2_BF(struct free_block_type* old_free);
void Delete2_WF(struct free_block_type* old_free);
/*������Ӧ�㷨���µĿ��п�new_free���������Ӧ��������*/
void Insert_FF(struct free_block_type* new_free);
void Insert_BF(struct free_block_type* new_free);
void Insert_WF(struct free_block_type* new_free);

/*���п�����ݽṹ*/
struct free_block_type {
	int size;							//���п��С
	int start_addr;						//���п���ʼ��ַ
	struct free_block_type *FF_next;	//����FF�㷨��������������ָ����
	struct free_block_type *BF_next;	//����BF�㷨��������������ָ����
	struct free_block_type *WF_next;	//����WF�㷨��������������ָ����
};

struct free_block_type *free_block;		//���п�ͷָ��
/*�ѷ����ڴ������ݽṹ*/
struct allocated_block {
	int pid;							//ʹ���ڴ�Ľ��̵�pid
	int size;							//�ռ��С
	int start_addr;						//��ʼ��ַ
	char process_name[PROCESS_NAME_LEN];//������
	struct allocated_block *next;		//�����ѷ����ڴ������ָ����
};

struct allocated_block *allocated_block_head = NULL;	//�ѷ����ڴ�����ͷָ��

/*��ʼ���ڴ���п�����*/
struct free_block_type* init_free_block(int mem_size) {
	struct free_block_type *fb;			//��Ϊͷ���
	struct free_block_type *fb2;		//������Ч���ݴ����￪ʼ
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

/*������*/
int main() {
	char choice;		//����ֵ��Ӧ��ͬ�˵�ѡ��
	pid = 0;
	int f;				//�������whileѭ��
	free_block = init_free_block(mem_size);
	for (;;) {
		display_menu();
		fflush(stdin);
		choice = getchar();
		f = 1;		//ʹ��fʵ�ֲ˵����ظ���ʾ
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

/*��ʾ�˵�
�˵�ѡ�������
	1�����ó�ʼ�ڴ��С
	2�������ڴ�����㷨
	3��Ϊ�µĽ��̿����ڴ�ռ�
	4��ȡ��һ�����̲��ͷ���Ӧ�ռ�
	5����ʾ�ڴ�ʹ�����
	6����0����������
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

/*�����ڴ��ʼ��С*/
int set_mem_size() {
	int size;
	//��ֹ�ڴ��С�ظ�����
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

/*�����ڴ�����㷨*/
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
	default:printf("��������\n"); return;
	}
}

/*�����½��̵��ڴ�����*/
int new_process() {
	struct allocated_block *ab;
	int size;
	int ret;

	pid++;	//��ȡ�µ�pid��
	
	cout << "PROCESS" << pid << endl;
	printf("Memory for %d:", pid);
	cin >> size;
	if (size <= 0) {
		printf("���ô�С�Ƿ�\n");
		pid--;
		ab = NULL;
		return -1;
	}
	//Ϊ�½��̷����ڴ棬���ʧ�ܣ�����-1
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
	//���µ��ѷ����ڴ��ͷ�����ѷ����ڴ�������
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

/*ɱ�����̣������վɽ��̵��ڴ�*/
void kill_process() {
	struct allocated_block *ab;
	int pid;
	printf("Kill Process, pid=");
	cin >> pid;
	//scanf("%d", &pid);
	ab = find_process(pid);		//���̶�Ӧ���ѷ����ڴ��ڵ�
	if (ab != NULL) {
		free_mem(ab);			//���վɽ��̵��ڴ�
		dispose(ab);			//���ڴ��ѷ���������ɾ���ڵ�ab
	}
	else
		printf("Process %d dosn't exist.", pid);
}

/*��ʾ�ڴ�������*/
int display_mem_usage() {
	struct free_block_type *fbt = free_block;
	struct free_block_type * tmp = fbt;
	struct allocated_block *ab = allocated_block_head;
	struct allocated_block *tmp2 = allocated_block_head;
	if (fbt == NULL)
		return -1;
	printf("------------------------------Memory Information ---------------------------------\n");
	/*���ڴ���ַ����˳����ʾ�ڴ�������*/
	printf("  * Free Memory Block: \n");
	printf("	  [start_addr]		   [ size ]\n");
	while (tmp->FF_next != NULL) {
		cout <<"		"<< tmp->FF_next->start_addr <<"		    "<<tmp->FF_next->size << endl;
		tmp = tmp->FF_next;
	}
	printf("\n");
	/*�����̴���ʱ���ɶ̵�����ʾ�ڴ�������*/
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

//�ӿ����������СΪsize���ڴ�
int allocate_mem(int size) {
	struct free_block_type* new_free;
	struct free_block_type* old_free = NULL;
	int start_addr;
	switch (ma_algorithm) {
	case MA_FF:
		old_free = Delete_FF(size);
		if(old_free == NULL){
			printf("�������ռ䲻��\n");
			return -1;
		}
		Delete2_BF(old_free);
		Delete2_WF(old_free);
		break;
	case MA_BF:
		old_free = Delete_BF(size);
		if (old_free == NULL) {
			printf("�������ռ䲻��\n");
			return -1;
		}
		Delete2_FF(old_free);
		Delete2_WF(old_free);
		break;
	case MA_WF:
		old_free = Delete_WF(size);
		if (old_free == NULL) {
			printf("�������ռ䲻��\n");
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

//�ͷ��ѷ����ڴ��ab�Ŀռ�
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

//���ڴ��ѷ���������ɾ���ڵ�free_ab
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

//����pid�����ڴ��ѷ��������ж�Ӧ�Ľڵ�
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