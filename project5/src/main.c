/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


char * algo;

typedef struct
{
	int page;
	int age;
}frame_t;

frame_t * frame_table;
int nframes;
struct disk *disk;
int page_faults, disk_reads, disk_writes; 

static int rand_policy()
{
	return rand() % nframes;
}

static int fifo_policy()
{
	static int tiktok;
	int frame = 0;

	++tiktok;
	for (int i = 0; i < nframes; ++i)
	{
		++frame_table[i].age;
		if (frame_table[frame].age < frame_table[i].age)
			frame = i;
	}
	frame_table[frame].age = 0;

	return frame;
}

static int lru_policy(int f)
{
	static int tiktok;
	int frame = 0;

	++tiktok;
	if (f != -1)
	{
		frame_table[f].age = tiktok;
		return f;
	}
	for (int i = 0; i < nframes; ++i)
	{
		if (frame_table[frame].age > frame_table[i].age)
			frame = i;
	}
	frame_table[frame].age = tiktok;

	return frame;
}

void page_fault_handler( struct page_table *pt, int page )
{
	int frame;
	int bits;

	++page_faults;
	page_table_get_entry(pt, page, &frame, &bits);
	if (bits & PROT_READ) // write failed
	{
		page_table_set_entry(pt, page, frame, bits | PROT_WRITE);
		if (!strcmp(algo, "custom"))
			lru_policy(frame);	
	}
	else
	{
		// search free frame
		frame = 0;
		while (frame < nframes && frame_table[frame].page >= 0)
			++frame;
		if (frame == nframes)
		{
			// select a victim
			if (!strcmp(algo, "rand"))
				frame = rand_policy();
			else if (!strcmp(algo, "fifo"))
				frame = fifo_policy();
			else if (!strcmp(algo, "custom"))
				frame = lru_policy(-1);
			int pre_page = frame_table[frame].page;
			disk_write(disk, pre_page, &page_table_get_physmem(pt)[frame * PAGE_SIZE]);
			++disk_writes;
			page_table_set_entry(pt, pre_page, 0, 0);
		}
		else
		{
			if (!strcmp(algo, "custom"))
				lru_policy(frame);	
		}
		disk_read(disk, page, &page_table_get_physmem(pt)[frame * PAGE_SIZE]);
		++disk_reads;
		frame_table[frame].page = page;
		page_table_set_entry(pt, page, frame, PROT_READ);
		// printf("page: %d victim frame: %d \n", page, frame);
	}
}

int main( int argc, char *argv[] )
{
	if(argc!=5) {
		printf("use: virtmem <npages> <nframes> <rand|fifo|custom> <alpha|beta|gamma|delta>\n");
		return 1;
	}

	int npages = atoi(argv[1]);
	nframes = atoi(argv[2]);
	algo = argv[3];
	const char *program = argv[4];

	if (strcmp(algo, "rand") && strcmp(algo, "fifo") && strcmp(algo, "custom"))
	{
		fprintf(stderr,"unknown page replacement algorithm: %s\n",argv[3]);
		return 1;	
	}

	disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}


	struct page_table *pt = page_table_create( npages, nframes, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

	unsigned char *virtmem = page_table_get_virtmem(pt);

	unsigned char *physmem = page_table_get_physmem(pt);

	// initialize frame table
	frame_table = (frame_t *)malloc(nframes * sizeof(frame_t));
	for (int i = 0; i < nframes; ++i)
	{
		frame_table[i].page = -1;
		frame_table[i].age = 0;
	}
		

	if(!strcmp(program,"alpha")) {
		alpha_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"beta")) {
		beta_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"gamma")) {
		gamma_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"delta")) {
		delta_program(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[4]);
		return 1;
	}

	page_table_delete(pt);
	disk_close(disk);

	printf("page faults: %d\ndisk reads: %d\ndisk writes: %d\n", page_faults, disk_reads, disk_writes);

	return 0;
}
