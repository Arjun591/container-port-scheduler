#include "port.h"
#include <stdio.h>
PortStats port_stats(const Port *p) {
    PortStats s={0};s.total=p->count;
    long long waits=0,turns=0,berth_hours=0,crane_hours=0;
    for(size_t i=0;i<p->count;i++){
        const Vessel *v=&p->vessels[i];const Assignment *a=&p->assignments[i];
        if(!a->scheduled)continue;
        s.scheduled++;s.containers_served+=v->containers;
        waits+=a->start-v->arrival;turns+=a->finish-v->arrival;
        berth_hours+=v->duration;crane_hours+=(long long)v->duration*v->cranes;
        if(a->finish>s.makespan)s.makespan=a->finish;
    }
    s.unscheduled=s.total-s.scheduled;
    if(s.scheduled){s.avg_wait=(double)waits/s.scheduled;s.avg_turnaround=(double)turns/s.scheduled;}
    if(s.makespan){s.berth_utilization=(double)berth_hours/(p->berth_count*s.makespan);
        s.crane_utilization=(double)crane_hours/(p->crane_count*s.makespan);}
    return s;
}
void port_print_report(const Port *p,const PortStats *s,const char *strategy){
    printf("\n=== %s ===\n",strategy);
    printf("Berths: %d | Cranes: %d | Scheduled: %zu/%zu | Containers: %lld\n",
            p->berth_count,p->crane_count,s->scheduled,s->total,s->containers_served);
    printf("Avg wait: %.2f h | Avg turnaround: %.2f h | Makespan: %d h\n",
            s->avg_wait,s->avg_turnaround,s->makespan);
    printf("Berth use: %.1f%% | Crane use: %.1f%% | Unscheduled: %zu\n",
            100*s->berth_utilization,100*s->crane_utilization,s->unscheduled);
    printf("%-5s %-18s %-6s %-7s %-7s %-8s\n",
            "ID","Vessel","Berth","Start","Finish","Status");
    for(size_t i=0;i<p->count;i++){
        const Vessel*v=&p->vessels[i];const Assignment*a=&p->assignments[i];
        if(a->scheduled)printf("%-5d %-18.18s %-6d %-7d %-7d %-8s\n",
             v->id,v->name,a->berth+1,a->start,a->finish,"OK");
        else printf("%-5d %-18.18s %-6s %-7s %-7s %-8s\n",
             v->id,v->name,"-","-","-","MISSED");
    }
}
int port_export_csv(const Port *p,const char *path){
    FILE*f=fopen(path,"w");if(!f)return 0;
    fputs("id,name,arrival,duration,cranes,priority,containers,deadline,berth,start,finish,status\n",f);
    for(size_t i=0;i<p->count;i++){
        const Vessel*v=&p->vessels[i];const Assignment*a=&p->assignments[i];
        fprintf(f,"%d,%s,%d,%d,%d,%d,%d,%d,",v->id,v->name,v->arrival,v->duration,v->cranes,v->priority,v->containers,v->deadline);
        if(a->scheduled)fprintf(f,"%d,%d,%d,SCHEDULED\n",a->berth+1,a->start,a->finish);
        else fputs(",,,UNSCHEDULED\n",f);
    }
    return fclose(f)==0;
}
int port_export_json(const Port *p,const PortStats *s,const char *strategy,const char *path){
    FILE*f=fopen(path,"w");if(!f)return 0;
    fprintf(f,"{\n  \"strategy\": \"%s\",\n  \"berths\": %d,\n  \"cranes\": %d,\n",
          strategy,p->berth_count,p->crane_count);
    fprintf(f,"  \"total\": %zu,\n  \"scheduled\": %zu,\n  \"unscheduled\": %zu,\n",
          s->total,s->scheduled,s->unscheduled);
    fprintf(f,"  \"containers_served\": %lld,\n  \"average_wait_hours\": %.6f,\n",
          s->containers_served,s->avg_wait);
    fprintf(f,"  \"average_turnaround_hours\": %.6f,\n  \"makespan_hours\": %d,\n",
          s->avg_turnaround,s->makespan);
    fprintf(f,"  \"berth_utilization\": %.6f,\n  \"crane_utilization\": %.6f\n}\n",
          s->berth_utilization,s->crane_utilization);
    return fclose(f)==0;
}
