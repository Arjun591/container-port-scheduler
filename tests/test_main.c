#include "port.h"
#include "algorithms.h"
#include "network.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int passed=0, failed=0;
#define CHECK(COND, MSG) do { \
    if (COND) { passed++; } else { failed++; fprintf(stderr,"FAIL: %s (%s:%d)\n",MSG,__FILE__,__LINE__); } \
} while(0)

static Vessel vessel(int id,int arr,int dur,int cranes,int priority,int containers,int deadline) {
    Vessel v={0};
    v.id=id;snprintf(v.name,sizeof(v.name),"Ship-%d",id);
    v.arrival=arr;v.duration=dur;v.cranes=cranes;v.priority=priority;
    v.containers=containers;v.deadline=deadline;
    return v;
}
static void tests_data_and_schedules(void) {
    const char *file[]={"data/normal.csv","data/capacity_shock.csv","data/stress.csv","data/adversarial.csv"};
    const size_t counts[]={9,12,180,24};
    const size_t served_arrival[]={9,7,84,15};
    const size_t served_priority[]={9,5,77,16};
    for(int scenario=0;scenario<4;scenario++) {
        Port base;char err[PORT_ERR_LEN];
        CHECK(port_load_csv(file[scenario],&base,err,sizeof(err)),"Scenario CSV loads");
        CHECK(base.count==counts[scenario],"Scenario ship count");
        for(int mode=1;mode<=2;mode++) {
            Port p=base;
            CHECK(port_schedule(&p,(PortStrategy)mode),"Scheduler returns success");
            CHECK(port_verify_schedule(&p,err,sizeof(err)),"Independent schedule verification");
            PortStats s=port_stats(&p);
            CHECK(s.scheduled==(mode==1?served_arrival[scenario]:served_priority[scenario]),
                  "Expected number of ships scheduled");
            CHECK(s.scheduled+s.unscheduled==p.count,"No ship is lost");
        }
    }
    Port invalid;char err[PORT_ERR_LEN];
    CHECK(!port_load_csv("data/does_not_exist.csv",&invalid,err,sizeof(err)),"Missing file rejected");
}
static void tests_fleet_and_search(void) {
    Port p;port_init(&p);p.berth_count=2;p.crane_count=4;
    char err[PORT_ERR_LEN];
    Vessel a=vessel(11,0,3,2,5,200,12);
    Vessel b=vessel(17,2,4,1,3,150,16);
    CHECK(port_add_vessel(&p,a,err,sizeof(err)),"Add ship A");
    CHECK(port_add_vessel(&p,b,err,sizeof(err)),"Add ship B");
    CHECK(p.count==2,"Two ships stored");
    CHECK(port_find_vessel(&p,17)==1,"Binary ID search finds second ship");
    CHECK(port_find_vessel(&p,999)==-1,"Binary search reports absence");
    CHECK(!port_add_vessel(&p,a,err,sizeof(err)),"Duplicate ID rejected");
    CHECK(p.count==2,"Failed add is atomic");
    Vessel changed=b;changed.priority=9;changed.id=18;
    CHECK(port_edit_vessel(&p,17,changed,err,sizeof(err)),"Edit and rename ship ID");
    CHECK(port_find_vessel(&p,17)==-1&&port_find_vessel(&p,18)==1,"Edit searchable by new ID");
    CHECK(!port_set_capacity(&p,1,1,err,sizeof(err)),"Refuse capacity smaller than ship demand");
    CHECK(p.crane_count==4,"Failed capacity edit is atomic");
    CHECK(port_set_capacity(&p,3,5,err,sizeof(err)),"Set valid berth and crane capacity");
    CHECK(port_delete_vessel(&p,11),"Delete ship by ID");
    CHECK(p.count==1&&p.vessels[0].id==18,"Array compaction after deletion");
    CHECK(!port_delete_vessel(&p,11),"Refuse second deletion");
    CHECK(port_save_csv(&p,"test_roundtrip.csv"),"Save custom CSV");
    Port copy;
    CHECK(port_load_csv("test_roundtrip.csv",&copy,err,sizeof(err)),"Reload custom CSV");
    CHECK(copy.count==1&&copy.vessels[0].priority==9&&copy.berth_count==3&&copy.crane_count==5,
        "CSV round trip preserves fleet and capacities");
    remove("test_roundtrip.csv");
}
static void tests_merge_and_intervals(void) {
    Port p;port_init(&p);p.berth_count=3;p.crane_count=5;
    p.vessels[0]=vessel(11,0,3,1,2,20,0);
    p.vessels[1]=vessel(12,0,2,1,8,40,0);
    p.vessels[2]=vessel(13,2,2,1,5,50,0);
    p.count=3;
    int order[PORT_MAX_VESSELS];
    CHECK(merge_sort_vessel_order(&p,order,PORT_PRIORITY_FIRST),"Merge sort succeeds");
    CHECK(order[0]==1&&order[1]==2&&order[2]==0,"Priority sort order");
    CHECK(merge_sort_vessel_order(&p,order,PORT_ARRIVAL_FIRST),"Arrival merge sort succeeds");
    CHECK(order[0]==1&&order[1]==0&&order[2]==2,"Arrival sort with duration ties");
    int berths[PORT_MAX_VESSELS];
    CHECK(interval_partition_port(&p,berths)==2,"Minimum 2 fixed-arrival berths");
    CHECK(berths[0]!=berths[1],"Overlapping intervals use different berths");
    CHECK(berths[2]==berths[1],"Reuse berth on matching end/start");
}
static void tests_knapsack(void) {
    Port p;port_init(&p);p.crane_count=5;
    p.vessels[0]=vessel(1,0,2,1,5,6,0);
    p.vessels[1]=vessel(2,0,3,1,5,10,0);
    p.vessels[2]=vessel(3,0,4,1,5,12,0);
    p.count=3;
    unsigned char chosen[PORT_MAX_VESSELS]={0};long long best=-1;
    CHECK(knapsack_crane_hours(&p,5,chosen,&best),"DP knapsack succeeds");
    CHECK(best==16,"DP knapsack exact optimum");
    CHECK(chosen[0]&&chosen[1]&&!chosen[2],"Knapsack recovers chosen ships");
    CHECK(!knapsack_crane_hours(&p,5001,chosen,&best),"Knapsack capacity upper bound enforced");
}
static void tests_graph(void) {
    int w[]={0,2,10,-1, -1,0,3,9, -1,-1,0,1, -1,-1,-1,0};
    int d[4],b[4];
    CHECK(dijkstra(4,w,0,d),"Dijkstra runs");
    CHECK(bellman_ford(4,w,0,b)==1,"Bellman Ford runs");
    CHECK(d[3]==6&&b[3]==6,"Both shortest paths match expected 6");
    int neg[]={0,4,10, -1,0,-2, -1,-1,0};
    CHECK(!dijkstra(3,neg,0,d),"Dijkstra refuses negative weight");
    CHECK(bellman_ford(3,neg,0,b)==1&&b[2]==2,"Bellman Ford handles -2 edge");
    int u[]={0,1,0},v[]={1,2,2},weights[]={1,2,10},tree=0;
    CHECK(kruskal_mst(3,3,u,v,weights,&tree)&&tree==3,"Kruskal MST known example");
    int capacity[16][16]={{0}};
    capacity[0][1]=3;capacity[0][2]=2;capacity[1][2]=1;
    capacity[1][3]=2;capacity[2][3]=3;
    CHECK(edmonds_karp(4,&capacity[0][0],0,3)==5,"Edmonds-Karp max flow=5");
    CHECK(capacity[0][1]==3,"Max flow does not mutate original graph");
    PortNetwork g;char err[PORT_ERR_LEN];
    CHECK(network_load_csv("data/terminal_network.csv",&g,err,sizeof(err)),"Load port network CSV");
    CHECK(g.n==6,"Port network has six nodes");
    CHECK(network_print_analysis(&g,"Gate","Berth-B"),"Integrated network comparisons");
}
static unsigned int seed=123456789U;
static unsigned int random_u32(void) {
    seed ^= seed << 13;seed ^= seed >> 17;seed ^= seed << 5;
    return seed;
}
static void tests_random_scheduling(void) {
    /* Repeat 100 independent randomly generated valid fleets and both heuristics.
       These verify constraints, not global optimality. */
    for(int trial=0;trial<100;trial++) {
        Port p;port_init(&p);p.berth_count=1+(int)(random_u32()%4);
        p.crane_count=2+(int)(random_u32()%8);
        p.count=10+(size_t)(random_u32()%7);
        for(size_t i=0;i<p.count;i++) {
            int arrival=(int)(random_u32()%30);
            int duration=1+(int)(random_u32()%8);
            int crane=1+(int)(random_u32()%(unsigned)p.crane_count);
            int deadline=arrival+duration+1+(int)(random_u32()%20);
            p.vessels[i]=vessel((int)i+1,arrival,duration,crane,
                     1+(int)(random_u32()%10),100+(int)(random_u32()%300),deadline);
        }
        char err[PORT_ERR_LEN];
        CHECK(port_validate_input(&p,err,sizeof(err)),"Random fleet valid");
        for(int strategy=1;strategy<=2;strategy++) {
            CHECK(port_schedule(&p,(PortStrategy)strategy),"Random scheduler completes");
            CHECK(port_verify_schedule(&p,err,sizeof(err)),"Random schedule satisfies every constraint");
        }
    }
}
int main(void) {
    tests_data_and_schedules();
    tests_fleet_and_search();
    tests_merge_and_intervals();
    tests_knapsack();
    tests_graph();
    tests_random_scheduling();
    printf("\nTEST RESULTS: passed=%d, failed=%d\n",passed,failed);
    return failed?1:0;
}
