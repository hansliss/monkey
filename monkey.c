#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>

#define MaxNomeSize 30

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
  SRANDOM(time(NULL));
}

int addnome(NomeData nome, struct nomenode **nomes, int nomesize) {
  int result,i,found;
  unsigned char *tmp;
  int *itmp;
  if (!(*nomes)) {
    if (!((*nomes)=(struct nomenode *)MALLOC(sizeof(struct nomenode)))) {
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
    if (!((*nomes)->clist=(unsigned char *)MALLOC(ahead*sizeof(unsigned char)))) {
      fprintf(stderr,"Out of memory allocating clist\n");
      return 0;
    }
    if (!((*nomes)->numlist=(int *)MALLOC(ahead*sizeof(int)))) {
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
	if ((*nomes)->clist[i]==nome[nomesize-1]) {
	  found=1;
	  (*nomes)->numlist[i]++;
	  break;
	}
      if (!found) {
	if ((*nomes)->used_len==(*nomes)->alloc_len) {
	  if (!(tmp=(unsigned char *)MALLOC(
					    sizeof(unsigned char)*((*nomes)->alloc_len+ahead)))) {
	    fprintf(stderr,"Out of memory expanding clist\n");
	    return 0;
	  }
	  if (!(itmp=(int *)MALLOC(
				   sizeof(int)*((*nomes)->alloc_len+ahead)))) {
	    fprintf(stderr,"Out of memory expanding numlist\n");
	    return 0;
	  }
	  for (i=0;i<(*nomes)->used_len;i++) {
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
    for (i=0;i<nomes->used_len;i++) {
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
    if (((*nomes)->clist)&&((*nomes)->numlist)) {
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

int main(int argc,char *argv[]) {
  int nomesize,i,readchars,inchar;
  long outchars,nn;
  NomeData nome,n;
  unsigned char c;
  FILE *infile,*outfile;
  memsize=0;
  nomes=NULL;
  fprintf(stderr,"Monkey (c) 1992,2000 Hans Liss - Text Destructor\n");
  if (argc!=5) {
    fprintf(stderr,"Usage: monkey <nomesize> <outchars> <infile> <outfile>\n");
    return(1);
  }
  nomesize=atoi(argv[1]);
  if ((nomesize<=1)||(nomesize>MaxNomeSize)) {
    fprintf(stderr,"Illegal nome size %d\n",nomesize);
    return(1);
  }
  if (!strcmp(argv[3],"-"))
    infile=stdin;
  else
    if (!(infile=fopen(argv[3],"r"))) {
      fprintf(stderr,"Failed to open input file %s.\n",argv[3]);
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
  fprintf(stderr,"%d characters read, used %ld bytes memory.\n",readchars,memsize);
  if (!strcmp(argv[2],"-"))
    outchars=readchars;
  else
    outchars=atol(argv[2]);
  fclose(infile);
  if (!strcmp(argv[4],"-"))
    outfile=stdout;
  else
    if (!(outfile=fopen(argv[4],"w"))) {
      fprintf(stderr,"Failed to open output file %s.\n",argv[4]);
      return(1);
    }
  fprintf(outfile,"%s", n);
  for (nn=0;nn<outchars;nn++) {
    c=newchar(n,nomes,nomesize);
    if (c)
      fputc(c,outfile);
  }
  fclose(outfile);
  remnomes(&nomes);
  return 0;
}

