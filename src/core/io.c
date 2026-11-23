#include "port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

void port_init(Port *p) {
    memset(p,0,sizeof(*p));
    p->berth_count=3;
    p->crane_count=6;
}
static char *trim(char *s) {
    while(isspace((unsigned char)*s)) s++;
    size_t n=strlen(s);
    while(n && isspace((unsigned char)s[n-1])) s[--n]='\0';
    return s;
}
static int number(const char *s, int *out) {
    char *end; long x;
    if(!s || !*s)return 0;
    errno=0; x=strtol(s,&end,10);
    if(errno || end==s || *trim(end)!='\0' || x<INT_MIN || x>INT_MAX)return 0;
    *out=(int)x; return 1;
}
static int split_csv(char *line, char **fields, int limit) {
    int n=0; char *start=line;
    for(char *q=line;;q++) {
        if(*q==',' || *q=='\0' || *q=='\r' || *q=='\n') {
            char end=*q; *q='\0';
            if(n<limit) fields[n]=trim(start);
            n++;
            if(end!=',')break;
            start=q+1;
        }
    }
    return n;
}
int port_load_csv(const char *path, Port *p, char *err, size_t errsz) {
    FILE *f=fopen(path,"r"); char line[512]; int line_no=0,header=0;
    if(!f){snprintf(err,errsz,"Cannot open '%s'",path);return 0;}
    port_init(p);
    while(fgets(line,sizeof(line),f)) {
        line_no++;
        if(!strchr(line,'\n') && !feof(f)){
            snprintf(err,errsz,"CSV line %d exceeds 511 characters",line_no);fclose(f);return 0;
        }
        char *s=trim(line);
        if(!*s)continue;
        if(*s=='#') {
            if(strncmp(s,"#berths=",8)==0 && !number(s+8,&p->berth_count)) {
                snprintf(err,errsz,"Invalid berth capacity on line %d",line_no);fclose(f);return 0;
            }
            if(strncmp(s,"#cranes=",8)==0 && !number(s+8,&p->crane_count)) {
                snprintf(err,errsz,"Invalid crane capacity on line %d",line_no);fclose(f);return 0;
            }
            continue;
        }
        if(!header) {
            char hdr[512];snprintf(hdr,sizeof(hdr),"%s",s);
            char *cols[12]; int n=split_csv(hdr,cols,12);
            if(n!=7 && n!=8) {
                snprintf(err,errsz,"Header must have 7 or 8 data columns (line %d)",line_no);fclose(f);return 0;
            }
            if(strcmp(cols[0],"id")!=0 || strcmp(cols[1],"name")!=0) {
                snprintf(err,errsz,"Expected id,name CSV header (line %d)",line_no);fclose(f);return 0;
            }
            header=1;continue;
        }
        if(p->count==PORT_MAX_VESSELS){
            snprintf(err,errsz,"Maximum %d vessels exceeded",PORT_MAX_VESSELS);fclose(f);return 0;
        }
        char *cols[12];int n=split_csv(s,cols,12);
        if(n!=7 && n!=8) {
            snprintf(err,errsz,"Line %d: expected 7 or 8 columns, received %d",line_no,n);fclose(f);return 0;
        }
        Vessel v={0};
        if(strlen(cols[1])>=PORT_NAME_LEN || !*cols[1]) {
            snprintf(err,errsz,"Line %d: vessel name empty/too long",line_no);fclose(f);return 0;
        }
        snprintf(v.name,sizeof(v.name),"%s",cols[1]);
        int *fields[]={&v.id,&v.arrival,&v.duration,&v.cranes,&v.priority,&v.containers};
        int pos[]={0,2,3,4,5,6};
        for(int i=0;i<6;i++)if(!number(cols[pos[i]],fields[i])){
            snprintf(err,errsz,"Line %d: invalid integer column %d",line_no,pos[i]+1);
            fclose(f);return 0;
        }
        if(n==8 && !number(cols[7],&v.deadline)){
            snprintf(err,errsz,"Line %d: invalid deadline",line_no);fclose(f);return 0;
        }
        p->vessels[p->count++]=v;
    }
    fclose(f);
    if(!header || !p->count){snprintf(err,errsz,"Missing header or vessel records");return 0;}
    return port_validate_input(p,err,errsz);
}
int port_validate_input(const Port *p, char *err, size_t errsz) {
    if(p->berth_count<1 || p->berth_count>16 || p->crane_count<1 || p->crane_count>64) {
        snprintf(err,errsz,"Berths must be 1..16 and cranes 1..64");return 0;
    }
    if(p->count>PORT_MAX_VESSELS){snprintf(err,errsz,"Too many vessels");return 0;}
    for(size_t i=0;i<p->count;i++){
        const Vessel *v=&p->vessels[i];
        if(v->id<=0 || !v->name[0] || v->arrival<0 || v->arrival>1000000 ||
            v->duration<1 || v->duration>100000 ||
            v->cranes<1 || v->cranes>p->crane_count ||
            v->priority<1 || v->priority>10 || v->containers<0 ||
            v->deadline<0 || (v->deadline && (long long)v->deadline<(long long)v->arrival+v->duration) ||
            (long long)v->arrival+v->duration>INT_MAX) {
            snprintf(err,errsz,"Vessel row %zu (id=%d) violates constraints",i+1,v->id);return 0;
        }
        for(size_t j=0;j<i;j++)if(p->vessels[j].id==v->id){
            snprintf(err,errsz,"Duplicate vessel ID %d",v->id);return 0;
        }
    }
    return 1;
}
