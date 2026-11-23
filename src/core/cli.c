#include "cli.h"
#include "port.h"
#include "algorithms.h"
#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

int port_benchmark(void);
static void output_dir(void) {
#ifdef _WIN32
    (void)_mkdir("outputs");
#else
    (void)mkdir("outputs", 0755);
#endif
}
static int read_line(char *out, size_t size) {
    if (!fgets(out, (int)size, stdin)) return 0;
    size_t len = strlen(out);
    if (len && out[len - 1] == '\n') out[--len] = '\0';
    else if (len == size - 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
        puts("Input too long; try again."); return 0;
    }
    if (len && out[len - 1] == '\r') out[len - 1] = '\0';
    return 1;
}
static int ask_string(const char *prompt, char *out, size_t length) {
    printf("%s",prompt); fflush(stdout);
    return read_line(out,length);
}
static int ask_int(const char *prompt, int *result) {
    char line[96], *end;
    if (!ask_string(prompt, line, sizeof(line))) return 0;
    errno = 0;
    long value = strtol(line, &end, 10);
    if (errno || end == line || *end || value < INT_MIN || value > INT_MAX) {
        puts("Enter a valid whole number."); return 0;
    }
    *result = (int)value;
    return 1;
}
static int ask_vessel(Vessel *v) {
    memset(v, 0, sizeof(*v));
    if (!ask_int("Ship ID (positive and unique): ", &v->id)) return 0;
    if (!ask_string("Ship name (no comma): ", v->name, sizeof(v->name))) return 0;
    if (strchr(v->name, ',')) { puts("CSV names cannot contain commas."); return 0; }
    return ask_int("Arrival hour: ", &v->arrival) &&
        ask_int("Service duration (hours): ", &v->duration) &&
        ask_int("Cranes required simultaneously: ", &v->cranes) &&
        ask_int("Priority (1-10, higher first): ", &v->priority) &&
        ask_int("Containers: ", &v->containers) &&
        ask_int("Deadline (0 = no deadline): ", &v->deadline);
}
static int load_dataset(Port *p, const char *name) {
    Port temp; char err[PORT_ERR_LEN];
    if (!port_load_csv(name, &temp, err, sizeof(err))) {
        printf("Load rejected: %s\n",err); return 0;
    }
    *p = temp;
    printf("Loaded %zu ships, %d berths and %d cranes from %s\n",
          p->count,p->berth_count,p->crane_count,name);
    return 1;
}
static int run_strategy(Port *p, PortStrategy strategy, int display) {
    char err[PORT_ERR_LEN];
    if (!port_validate_input(p, err, sizeof(err))) {
        fprintf(stderr,"Invalid fleet: %s\n",err); return 0;
    }
    if (!port_schedule(p,strategy) || !port_verify_schedule(p,err,sizeof(err))) {
        fprintf(stderr,"Scheduling or verification failed: %s\n",err); return 0;
    }
    if (display) {
        PortStats s=port_stats(p);
        port_print_report(p,&s,strategy==PORT_ARRIVAL_FIRST ? "ARRIVAL-FIRST" : "PRIORITY-FIRST");
        if (p->count > 30) puts("Large scenario: all rows shown above; use output CSV for filtering.");
    }
    return 1;
}
static void print_stats_line(const char *title, const PortStats *s) {
    printf("%-14s served %3zu/%-3zu | avg wait %6.2f h | avg turnaround %6.2f h"
        " | berth use %5.1f%% | crane use %5.1f%%\n",
       title,s->scheduled,s->total,s->avg_wait,s->avg_turnaround,
       100*s->berth_utilization,100*s->crane_utilization);
}
static int compare_and_export(const Port *current, int print_rows) {
    Port arrival = *current, priority = *current;
    if (!run_strategy(&arrival,PORT_ARRIVAL_FIRST,0) ||
        !run_strategy(&priority,PORT_PRIORITY_FIRST,0)) return 0;
    PortStats a=port_stats(&arrival), b=port_stats(&priority);
    puts("\n=== REAL BERTH + CRANE SCHEDULING COMPARISON ===");
    print_stats_line("Arrival-first",&a);
    print_stats_line("Priority-first",&b);
    if (print_rows && current->count <= 30) {
        port_print_report(&arrival,&a,"ARRIVAL-FIRST TIMETABLE");
        port_print_report(&priority,&b,"PRIORITY-FIRST TIMETABLE");
    }
    output_dir();
    int ok = port_export_csv(&arrival,"outputs/arrival_schedule.csv") &&
        port_export_csv(&priority,"outputs/priority_schedule.csv") &&
        port_export_json(&arrival,&a,"arrival-first","outputs/arrival_summary.json") &&
        port_export_json(&priority,&b,"priority-first","outputs/priority_summary.json");
    if (ok) puts("Saved 2 CSV timetables and 2 JSON summaries to outputs/.");
    else puts("Could not write outputs/ files; check permissions.");
    return ok;
}
static void menu_load(Port *p) {
    int choice;
    char name[256];
    puts("1. Normal (9 ships)\n2. Capacity shock (12 ships)\n3. Stress (180 ships)\n4. Adversarial (24 ships)\n5. Custom CSV path");
    if (!ask_int("Dataset choice: ",&choice)) return;
    const char *path = NULL;
    switch(choice) {
        case 1: path="data/normal.csv";break;
        case 2: path="data/capacity_shock.csv";break;
        case 3: path="data/stress.csv";break;
        case 4: path="data/adversarial.csv";break;
        case 5:
            if (!ask_string("CSV file path: ", name, sizeof(name))) return;
            path=name;break;
        default: puts("Unknown dataset.");return;
    }
    load_dataset(p,path);
}
static void menu_ship(Port *p,int action) {
    Vessel v;
    char err[PORT_ERR_LEN];
    int id;
    if (action == 2) {
        if (!ask_vessel(&v)) return;
        if (port_add_vessel(p,v,err,sizeof(err))) puts("Ship added; prior schedule cleared.");
        else printf("Add rejected: %s\n",err);
    } else if (action == 3) {
        if (!ask_int("ID to edit: ",&id)) return;
        if (port_find_vessel(p,id)<0) {puts("Ship not found.");return;}
        puts("Enter ALL replacement ship fields (including ID).");
        if (!ask_vessel(&v)) return;
        if (port_edit_vessel(p,id,v,err,sizeof(err))) puts("Ship updated; prior schedule cleared.");
        else printf("Edit rejected: %s\n",err);
    } else {
        if (!ask_int("ID to delete: ",&id)) return;
        puts(port_delete_vessel(p,id) ? "Ship deleted; prior schedule cleared." : "Ship not found.");
    }
}
static void menu_search(const Port *p) {
    int id;
    if (!ask_int("Vessel ID to find (binary search): ",&id)) return;
    int k=port_find_vessel(p,id);
    if (k<0) puts("Ship not found.");
    else {
        Vessel v=p->vessels[k];
        printf("Found: ID %d | %s | arrival %d | duration %d | cranes %d | priority %d | containers %d | deadline %d\n",
            v.id,v.name,v.arrival,v.duration,v.cranes,v.priority,v.containers,v.deadline);
    }
}
static void menu_planning(const Port *p) {
    int choice;
    puts("1. Interval partitioning (minimum berths if all ships start at arrival)");
    puts("2. 0/1 knapsack (choose ships under total crane-HOUR budget)");
    if (!ask_int("Planning choice: ",&choice)) return;
    if (choice == 1) {
        int berth[PORT_MAX_VESSELS];
        int required=interval_partition_port(p,berth);
        printf("Minimum berths for FIXED immediate-start intervals: %d; current berths: %d\n",
             required,p->berth_count);
        puts("This is NOT the minimum number of berths for a flexible delayed schedule.");
        if(p->count <= 20) for(size_t i=0;i<p->count;i++)
            printf("%s: hypothetical berth %d for [%d,%d)\n",p->vessels[i].name,
                berth[i]+1,p->vessels[i].arrival,p->vessels[i].arrival+p->vessels[i].duration);
    } else if (choice == 2) {
        int budget;
        if (!ask_int("Total crane-hour budget (0..5000): ",&budget)) return;
        unsigned char selected[PORT_MAX_VESSELS];long long best;
        if (!knapsack_crane_hours(p,budget,selected,&best)) {puts("Invalid budget or out of memory.");return;}
        printf("Best container total under %d crane-hours: %lld\n",budget,best);
        long long spent=0;
        for(size_t i=0;i<p->count;i++) if(selected[i]) {
            const Vessel *v=&p->vessels[i];
            printf("Choose ID %d %-18.18s (%d crane-hours, %d containers)\n",
                v->id,v->name,v->duration*v->cranes,v->containers);
            spent+=(long long)v->duration*v->cranes;
        }
        printf("Crane-hours used: %lld/%d\n",spent,budget);
        puts("This selects a portfolio; run the scheduler separately for time-feasible assignments.");
    } else puts("Unknown planning option.");
}
static int run_network(const char *origin, const char *destination) {
    PortNetwork g;char err[PORT_ERR_LEN];
    if (!network_load_csv("data/terminal_network.csv",&g,err,sizeof(err))) {
        fprintf(stderr,"Network: %s\n",err);return 0;
    }
    printf("Available network nodes:");
    for(int i=0;i<g.n;i++) printf(" %s%s",g.names[i],i==g.n-1?"\n":",");
    return network_print_analysis(&g,origin,destination);
}
static void menu_network(void) {
    PortNetwork g;char err[PORT_ERR_LEN],from[NETWORK_NAME_LEN],to[NETWORK_NAME_LEN];
    if(!network_load_csv("data/terminal_network.csv",&g,err,sizeof(err))) {puts(err);return;}
    puts("Nodes:");
    for(int i=0;i<g.n;i++)printf("  %s\n",g.names[i]);
    if(!ask_string("From node (case-sensitive): ",from,sizeof(from)) ||
       !ask_string("To node (case-sensitive): ",to,sizeof(to)))return;
    network_print_analysis(&g,from,to);
}
static void print_menu(void) {
    puts("\n             CONTAINER PORT SCHEDULING");
    puts("FLEET MANAGEMENT");
    puts(" 1 View ships/resources      2 Load scenario/custom CSV");
    puts(" 3 Add ship                  4 Edit ship");
    puts(" 5 Delete ship               6 Change berth/crane capacity");
    puts(" 7 Save current fleet CSV    8 Search ship by ID (binary search)");
    puts("SCHEDULING / ANALYSIS");
    puts(" 9 Run arrival-first        10 Run priority-first");
    puts("11 Compare + export         12 Planning: intervals / knapsack");
    puts("13 Network: paths / MST / flow");
    puts("14 Benchmark normal + shock + stress");
    puts(" 0 Exit");
}
static int interactive(Port *p) {
    for (;;) {
        print_menu();int choice;
        if (!ask_int("Select: ",&choice)) {
            if (feof(stdin)) return 0;
            continue;
        }
        switch(choice) {
            case 0: puts("Goodbye.");return 0;
            case 1: port_list_vessels(p);break;
            case 2: menu_load(p);break;
            case 3: menu_ship(p,2);break;
            case 4: menu_ship(p,3);break;
            case 5: menu_ship(p,4);break;
            case 6: {
                int berths,cranes;char err[PORT_ERR_LEN];
                if(!ask_int("Number of berths (1..16): ",&berths) ||
                   !ask_int("Number of cranes (1..64): ",&cranes))break;
                if(port_set_capacity(p,berths,cranes,err,sizeof(err)))
                    puts("Port capacity updated; previous schedules cleared.");
                else printf("Capacity rejected: %s\n",err);
                break;
            }
            case 7: {
                char path[256];
                if (!ask_string("Save CSV path (e.g., data/my_fleet.csv): ",path,sizeof(path)))break;
                if (p->count==0) {puts("Add at least one ship before saving.");break;}
                puts(port_save_csv(p,path)?"Fleet saved.":"Cannot save file; check path/permissions.");
                break;
            }
            case 8:menu_search(p);break;
            case 9:run_strategy(p,PORT_ARRIVAL_FIRST,1);break;
            case 10:run_strategy(p,PORT_PRIORITY_FIRST,1);break;
            case 11:compare_and_export(p,1);break;
            case 12:menu_planning(p);break;
            case 13:menu_network();break;
            case 14:port_benchmark();break;
            default:puts("Unknown option.");break;
        }
    }
}
static void help(void) {
    puts("Container Port Scheduler (C11), from the project root:");
    puts("  main.exe                 Start interactive menu");
    puts("  main.exe --menu          Same menu explicitly");
    puts("  main.exe --demo          Run normal + capacity shock comparison");
    puts("  main.exe --benchmark     Benchmark normal / shock / stress");
    puts("  main.exe --network       Analyze Gate -> Berth-B");
    puts("  main.exe --run FILE.csv  Run and export BOTH schedules for custom CSV");
    puts("  main.exe --help          Show this help");
}
int port_cli(int argc,char **argv) {
    /* All source files link into this single executable via main(). */
    Port p;port_init(&p);
    if(argc == 2 && strcmp(argv[1],"--help")==0) {help();return 0;}
    if(argc == 2 && strcmp(argv[1],"--benchmark")==0) return port_benchmark()?0:1;
    if(argc == 2 && strcmp(argv[1],"--network")==0)return run_network("Gate","Berth-B")?0:1;
    if(argc == 2 && strcmp(argv[1],"--demo")==0) {
        if(!load_dataset(&p,"data/normal.csv") || !compare_and_export(&p,0))return 1;
        if(!load_dataset(&p,"data/capacity_shock.csv") || !compare_and_export(&p,0))return 1;
        puts("\nTry --benchmark for the 180-ship stress case.");return 0;
    }
    if(argc == 3 && strcmp(argv[1],"--run")==0)
        return load_dataset(&p,argv[2])&&compare_and_export(&p,1)?0:1;
    if(argc==1 || (argc==2 && strcmp(argv[1],"--menu")==0)) {
        if(!load_dataset(&p,"data/normal.csv"))return 1;
        return interactive(&p);
    }
    help();return 2;
}
