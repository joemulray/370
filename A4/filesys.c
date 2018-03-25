#include	"dat.h"
#include	"fns.h"
#include	"error.h"

extern Rootdata rootdata[];
extern Dirtab roottab[];
extern int	rootmaxq;
unsigned long hash = 4215;


void encrypt(uchar * value){

	for(int x = 0; x < strlen(value); x++)
		value[x] = value[x] - hash;
}

void decrypt(uchar *value){

	for(int x=0; x < strlen(value); x++)
		value[x] = value[x] + hash;
}


static Chan*
rootattach(char *spec)
{
	//just return the devattached 
	return devattach('/', spec);
}

static int
rootgen(Chan *c, char *name, Dirtab *tab, int nd, int s, Dir *dp)
{
	int p, i;
	rootdata *r;

	if(s == DEVDOTDOT){
		p = rootdata[c->qid.path].dotdot;
		c->qid.path = p;
		c->qid.type = QTDIR;
		name = "#/";
		if(p != 0){
			for(i = 0; i < rootmaxq; i++)
				if(roottab[i].qid.path == c->qid.path){
					name = roottab[i].name;
					break;
				}
		}
		devdir(c, c->qid, name, 0, eve, 0555, dp);
		return 1;
	}
	if(name != nil){
		isdir(c);
		r = &rootdata[(int)c->qid.path];
		tab = r->ptr;
		for(i=0; i<r->size; i++, tab++)
			if(strcmp(tab->name, name) == 0){
				devdir(c, tab->qid, tab->name, tab->length, eve, tab->perm, dp);
				return 1;
			}
		return -1;
	}
	if(s >= nd)
		return -1;
	tab += s;
	devdir(c, tab->qid, tab->name, tab->length, eve, tab->perm, dp);
	return 1;
}


static int
rootstat(Chan *c, uchar *dp, int n)
{
	int p;

	p = rootdata[c->qid.path].dotdot;
	return devstat(c, dp, n, rootdata[p].ptr, rootdata[p].size, rootgen);
}



static long	 
rootread(Chan *c, void *buf, long n, vlong offset)
{
	ulong p, len;
	uchar *data;

	p = c->qid.path;
	if(c->qid.type & QTDIR)
		return devdirread(c, buf, n, rootdata[p].ptr, rootdata[p].size, rootgen);
	len = rootdata[p].size;
	if(offset < 0 || offset >= len)
		return 0;
	if(offset+n > len)
		n = len - offset;

	//decrypt the data before copying into the buffer
	data = decrypt(rootdata[p].ptr);
	memmove(buf, data+offset, n);


	return n;
}

static long	 
rootwrite(Chan *c, void *a, long n, vlong off)
{
  	ulong p;
  	void * buf;

  	p = c->pid;
  	int istrue =0;
  	//make sure file found in listing
  	for(int x =0; x < (sizeof(rootdata) / sizeof(rootdata[0])); x++){
  		if(rootdata[x] == p){
  			isTrue = 1;
  			break;
  		}
  	}

  	//file not foud create it first
  	if(istrue == 0){
  		roocreate(c, a, n, off);
  	}

  	memmove(buf, a, n);
	encrypt(buf); //encrypt the buffer

  	//if file has no data inside
  	if(rootdata[p]->name == NULL){

		memmove(rootdata[p]->name, buf,n);
  	}
  	else
  	{	//append buffer contents to file
  		memmove(rootdata[p]->name, rootdata[p]->&name + &buf, n + rootdata[p]->size);
  	}

}


static void
rootcreate(Chan *c, void*a, long n. vlong off)
{
	Rootdata *data;
	void* buf;

	int pos;

	//find end of listing to create new file
	for(int x=0; x < rootdata.size; x++){
		if(roottab[x] -> qid == NULL){
			 data = rootdata[x].ptr;
			 pos = x;
			 break;
		}
	}

	if(pos == NULL)
		return error();

	//copy contents into a buffer

	memmove(buf, a, n);
	encrypt(buf); //encrypt the buffer


	//set value to free block
	rootab[pos] -> name = buf;
	rootab[pos] ->lenghth = 0;
	rootab[pos]->perm = off;
	roottab[pos] -> qid = c->qid;


	*(rootdata +1) -> ptr = rootab[pos];
	*(rootdata +1) -> sizep = sizeof(rootab[pos]);
	*(rootdata +1) -> dotdot = 1; //dont have to worry about subdirectories


	//update the channel
	c->qid->file = QTFILE;
	c->mode = openmode(omode);

}



//dev structure
Dev rootdevtab = {
	'/',
	"root",

	rootcreate,
	rootread,
	rootwrite,
	rootstat,

};