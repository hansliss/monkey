#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<ctype.h>

#define MaxNomeSize 20

#define LINEBUFSIZE 8192

#define InbufSize 1024
long inbufcurrent=0;
#define ahead 1

#define SRANDOM srandom
#define RANDOM random

long memsize;

typedef char NomeData[MaxNomeSize];

struct nomenode {
  NomeData data;
  unsigned char *clist;
  int *numlist;
  int alloc_len,used_len;
  struct nomenode *left;
  struct nomenode *right;
} *nomes;

/* Protos */
void *MALLOC(int len);
void FREE(void *obj, int len);
void randomize();
int addnome(NomeData nome, struct nomenode **nomes, int nomesize);
char newchar(char *nome,struct nomenode *nomes,int nomesize);
void remnomes(struct nomenode **nomes);
int nextchar(FILE *f);
int main(int argc,char *argv[]);
/* --------- */

void *MALLOC(int len) {
  memsize+=len;
  return malloc(len);
}

void FREE(void *obj, int len) {
  memsize-=len;
  free(obj);
}


void randomize() {
  int seed;
  char *sp=(char *)&seed;
  FILE *infile=fopen("/dev/random","r");
  if (infile) {
    if (fread(sp,sizeof(seed),1,infile)<1) {
      perror("fread()");
    }
    fclose(infile);
  }
  else
    seed=time(NULL);
  SRANDOM(seed);
}

int addnome(NomeData nome, struct nomenode **nomes, int nomesize) {
  int result,i,found;
  unsigned char *tmp;
  int *itmp;
  if (!(*nomes)) {
    if (!((*nomes)=(struct nomenode *)MALLOC(sizeof(struct nomenode))))
      {
	fprintf(stderr,"Out of memory allocing nomenode\n");
	return 0;
      }
    (*nomes)->left=NULL;
    (*nomes)->right=NULL;
    strncpy((*nomes)->data,nome,nomesize-1);
#ifdef DEBUG
    if (nomesize<MaxNomeSize)
      (*nomes)->data[nomesize-1]='\0';
#endif
    if (!((*nomes)->clist=(unsigned char *)MALLOC(ahead*sizeof(unsigned char))))
      {
	fprintf(stderr,"Out of memory allocating clist\n");
	return 0;
      }
    if (!((*nomes)->numlist=(int *)MALLOC(ahead*sizeof(int))))
      {
	fprintf(stderr,"Out of memory allocating numlist\n");
	return 0;
      }
    (*nomes)->alloc_len=ahead;
    (*nomes)->used_len=1;
    (*nomes)->clist[0]=nome[nomesize-1];
    (*nomes)->numlist[0]=1;
    return 1;
  }
  else
    if (!(result=strncmp((*nomes)->data,nome,nomesize-1))) {
      found=0;
      for (i=0;i<(*nomes)->used_len;i++)
	if ((*nomes)->clist[i]==nome[nomesize-1])
	  {
	    found=1;
	    (*nomes)->numlist[i]++;
	    break;
	  }
      if (!found)
	{
	  if ((*nomes)->used_len==(*nomes)->alloc_len)
	    {
	      if (!(tmp=(unsigned char *)MALLOC(
						sizeof(unsigned char)*((*nomes)->alloc_len+ahead))))
		{
		  fprintf(stderr,"Out of memory expanding clist\n");
		  return 0;
		}
	      if (!(itmp=(int *)MALLOC(
				       sizeof(int)*((*nomes)->alloc_len+ahead))))
		{
		  fprintf(stderr,"Out of memory expanding numlist\n");
		  return 0;
		}
	      for (i=0;i<(*nomes)->used_len;i++)
		{
		  tmp[i]=(*nomes)->clist[i];
		  itmp[i]=(*nomes)->numlist[i];
		}
	      FREE((*nomes)->clist,(*nomes)->alloc_len);
	      FREE((*nomes)->numlist,(*nomes)->alloc_len);
	      (*nomes)->alloc_len+=ahead;
	      (*nomes)->clist=tmp;
	      (*nomes)->numlist=itmp;
	    }
	  (*nomes)->clist[(*nomes)->used_len]=nome[nomesize-1];
	  (*nomes)->numlist[(*nomes)->used_len++]=1;
	}
      return 1;
    }
    else
      if (result>0)
	return addnome(nome,&((*nomes)->left),nomesize);
      else
	return addnome(nome,&((*nomes)->right),nomesize);
}

char newchar(char *nome,struct nomenode *nomes,int nomesize) {
  int i,num,k;
  int result;
  if (!nomes)
    return 0;
  if (!(result=strncmp(nome,nomes->data,nomesize-1))) {
#ifdef DEMO2
    nome[nomesize-1]='\0';
    fprintf(stderr,"[/%s/: ",nome);
#endif
    num=0;
    for (i=0;i<nomes->used_len;i++)
      {
#ifdef DEMO2
	fprintf(stderr,"(%c):%d ",nomes->clist[i],nomes->numlist[i]);
#endif
	num+=nomes->numlist[i];
      }
    k=RANDOM()%num;
#ifdef DEMO2
    fprintf(stderr,", num=%d k=%d]",num,k);
#endif
    i=0;
    while(k>=nomes->numlist[i])
      k-=nomes->numlist[i++];
    nome[nomesize-1]=nomes->clist[i];
    for (i=0;i<=nomesize-2;i++)
      nome[i]=nome[i+1];
    return nome[nomesize-2];
  }
  else
    if (result<0)
      return newchar(nome,nomes->left,nomesize);
    else
      return newchar(nome,nomes->right,nomesize);
}

void remnomes(struct nomenode **nomes) {
#ifdef DEBUG
  char nometext[MaxNomeSize+1];
#endif
  if (!(*nomes))
    return;
  else {
    remnomes(&((*nomes)->left));
#ifdef DEBUG
    strncpy(nometext,(*nomes)->data,MaxNomeSize);
    nometext[MaxNomeSize-1]='\0';
    printf("/%s/ ",nometext);
    if (((*nomes)->clist)&&((*nomes)->numlist))
      {
	for (i=0;i<(*nomes)->used_len;i++)
	  printf("(%c):%d ",(*nomes)->clist[i],(*nomes)->numlist[i]);
      }
    printf("\n"); 
#endif
    remnomes(&((*nomes)->right));
    if ((*nomes)->clist)
      FREE((*nomes)->clist,(*nomes)->alloc_len);
    if ((*nomes)->numlist)
      FREE((*nomes)->numlist,(*nomes)->alloc_len);
    FREE((*nomes),sizeof(struct nomenode));
    (*nomes)=NULL;
  }
}

unsigned char inbuf[InbufSize];
int bufcnt;

int nextchar(FILE *f) {
  if ((inbufcurrent==0) || (bufcnt>=inbufcurrent)) {
    inbufcurrent=fread(inbuf,1,InbufSize,f);
    if (inbufcurrent==0)
      return -1;
    bufcnt=0;
  }
  return inbuf[bufcnt++];
}
/*#define nextchar(f) fgetc(f)*/

char mytolower(char c) {
  if (c < 127)
    return tolower(c);
  else {
    if (strchr("ÅÄÖ",c))
      return (c - 'Å' + 'å');
    else
      return c;
  }
}

char mytoupper(char c) {
  if (c < 127)
    return toupper(c);
  else {
    if (strchr("åäö",c))
      return (c - 'å' + 'Å');
    else
      return c;
  }
}

#define vocals "aeiouyåäö"

int count_syllables(char *s) {
  int i;
  int n=0, last_was_vocal=0;
  for (i=0;i<strlen(s);i++)
    if (strchr(vocals, mytolower(s[i]))) {
      if (!last_was_vocal)
	n++;
      last_was_vocal=1;
    }
    else
      last_was_vocal=0;
  return n;
}

int main(int argc,char *argv[]) {
  int nomesize,i,readchars,inchar;
  long count,nn;
  NomeData nome,n;
  unsigned char c;
  FILE *infile;
  char linebuf[LINEBUFSIZE];
  int lidx=1;
  memsize=0;
  nomes=NULL;
  /*  fprintf(stderr,"Monkey (c) 1992,2000 Hans Liss - Text Destructor\n");*/
  if (argc>3) {
    fprintf(stderr,"Usage: monkey <nomesize> <count>\n");
    return(1);
  }
  if (argc>1)
    nomesize=atoi(argv[1]);
  else
    nomesize=5;
  if ((nomesize<=1)||(nomesize>MaxNomeSize)) {
    fprintf(stderr,"Illegal nome size %d\n",nomesize);
    return(1);
  }
  if (argc>2)
    count=atol(argv[2]);
  else
    count=1;
  sprintf(linebuf,"%s.conf",argv[0]);
  if (!(infile=fopen(linebuf,"r"))) {
    sprintf(linebuf,"/usr/local/etc/hankey.conf");
    if (!(infile=fopen(linebuf,"r")))
      {
	printf("Failed to open input file %s.\n",linebuf);
	return(1);
      }
  }
  if (!fgets(linebuf, sizeof(linebuf), infile)) {
    perror("fgets()");
  }
  fclose(infile);
  while (strlen(linebuf) && (linebuf[strlen(linebuf)-1]=='\n'))
    linebuf[strlen(linebuf)-1]='\0';
  if (!(infile=fopen(linebuf,"r"))) {
    printf("Failed to open input file %s.\n",linebuf);
    return(1);
  }
  
    
  bufcnt=0;
  nome[nomesize]='\0';
  randomize();
  readchars=0;
  for (i=0;(i<nomesize)&&(inchar=nextchar(infile))!=-1;i++) {
    nome[i]=inchar;
    readchars++;
  }
  strncpy(n,nome,nomesize-1);
  n[nomesize-1]='\0';
  while((inchar!=-1)&&addnome(nome,&nomes,nomesize)) {
    addnome(nome,&nomes,nomesize);
    for (i=0;i<nomesize-1;i++)
      nome[i]=nome[i+1];
    if ((inchar=nextchar(infile))!=-1)
      {
	nome[nomesize-1]=inchar;
	readchars++;
      }
  }
  /*  fprintf(stderr,"%d characters read, used %ld bytes memory.\n",readchars,memsize);*/
  fclose(infile);
  strcpy(linebuf,n);
  nn=0;
  while (nn<count) {
    c=newchar(n,nomes,nomesize);

    if ((c=='\n') || (strlen(linebuf) >= (LINEBUFSIZE-1)))
      {
	if (count_syllables(linebuf)==((lidx&1)?5:7))
	  {
	    linebuf[0]=mytoupper(linebuf[0]);
	    fprintf(stdout,"%s\n",linebuf);
	    if (lidx==3)
	      {
		lidx=1;
		if (++nn < count)
		  fprintf(stdout,"\n");
	      }
	    else
	      lidx++;
	  }
	linebuf[0]='\0';
      }
    else
      {
	linebuf[strlen(linebuf)+1]='\0';
	linebuf[strlen(linebuf)]=c;
      }
  }
  remnomes(&nomes);
  return 0;
}

