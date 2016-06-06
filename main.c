#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>
#include <getopt.h>
#include "filap.h"
#include "smpl.h"

#define EV_REQUEST 1
#define EV_RECV 2
#define EV_REPLY 3
#define EV_RELEASE 4

#define ST_RELEASED 5
#define ST_WANTED 6
#define ST_HELD 7

#define MSG_REQUEST 8
#define MSG_REPLY 9

struct process_t {
    int state;
    int timestamp;
    int request_timestamp;
    int recv_queue;
    filap recvd_from;
    int nreplies;
    int *pending;
};
typedef struct process_t *process;

process init_processes(int pnum)
{
    process plist = malloc(sizeof(struct process_t) * pnum);

    if (plist == NULL) return NULL;

    for(int i = 0; i < pnum; ++i) {
        plist[i].state = ST_RELEASED;
        plist[i].timestamp = 0;
        plist[i].request_timestamp = 0;
        plist[i].recv_queue = facility("recvq", pnum);
        plist[i].recvd_from = cria_filap(pnum);
        plist[i].nreplies = 0;
        plist[i].pending = malloc(sizeof(int) * pnum);
        memset(plist[i].pending, 0, sizeof(int) * pnum);
    }

    return plist;
}

int destroy_processes(process plist, int pnum)
{
    if (plist == NULL) return 0;

    for(int i = 0; i < pnum; ++i) {
        destroi_filap(plist[i].recvd_from);
        free(plist[i].pending);
    }
    free(plist);

    return 1;
}

void send(process plist, int pnum, int pid, int dst_pid, int type)
{
    int recv_at, recv_at_pri;

    recv_at = random(1, 5);
    recv_at_pri = 5 - recv_at;

    if (request(plist[dst_pid].recv_queue, pid, recv_at_pri) == 0) {
        // schedule receive on receiving process
        insere_filap(plist[dst_pid].recvd_from, pid, plist[pid].timestamp, INT_MAX - time(), type);

        printf("  |- %s scheduled to occur at time %g\n", type == MSG_REQUEST ? "REQUEST" : "REPLY", time() + recv_at);

        schedule(EV_RECV, recv_at, dst_pid);
    } else {
        printf("\tError: Process %d tried to send a message to %d.\n", pid, dst_pid);
        printf("\tCause: Receive queue of process %d is full!\n", dst_pid);
        exit(1);
    }
}

void broadcast(process plist, int pnum, int pid)
{
    int recv_at, recv_at_pri;

    for(int dst_pid = 0; dst_pid < pnum; ++dst_pid) {
        if (dst_pid == pid) continue;

        send(plist, pnum, pid, dst_pid, MSG_REQUEST);
    }
}

void recv(process plist, int pnum, int pid, int pid_from, int timestamp_from, int type)
{
    int lower_pri;

    if (type == MSG_REQUEST) {
        if (plist[pid].state == ST_WANTED) {
            if (plist[pid].request_timestamp < timestamp_from) lower_pri = 0;
            else if (plist[pid].request_timestamp == timestamp_from && pid < pid_from) lower_pri = 0;
            else lower_pri = 1;
        } else
            lower_pri = 1;

        if (plist[pid].state == ST_HELD || (plist[pid].state == ST_WANTED && !lower_pri)) {
            plist[pid].pending[pid_from] = 1;

            printf("  |- REQUEST %d was enqueued\n", pid_from);
            printf("  |- pending(%d) = ", pid);
            for(int i = 0; i < pnum; ++i)
                printf("%d ", plist[pid].pending[i]);
            printf("\n");
        } else {
            send(plist, pnum, pid, pid_from, MSG_REPLY);
        }
    } else {
        if (plist[pid].state != ST_WANTED) {
            printf("\tError: unexpected REPLY. Message type = %d\n", type);
            exit(1);
        }

        plist[pid].nreplies++;
        if (plist[pid].nreplies == pnum-1) {
            printf("Process %d entered the critical region at time %g\n", pid, time());
            plist[pid].state = ST_HELD;
            schedule(EV_RELEASE, 5.0, pid);
        }
    }
}

void releasecr(process plist, int pnum, int pid)
{
    for(int dst_pid = 0; dst_pid < pnum; ++dst_pid) {
        if (plist[pid].pending[dst_pid]) {
            printf("Process %d sent *pending* REPLY to process %d\n", pid, dst_pid);
            send(plist, pnum, pid, dst_pid, MSG_REPLY);
        }
    }
    plist[pid].state = ST_RELEASED;
}

void print_usage(const char *program_name) {
    printf("Usage: %s --nproc <N> [--rc0 <request time>] [--rc1 <request time>] [--rc2 <request time>]\n\n", program_name);
    printf("Notes on the parameters:\n");
    printf(" - The flag --nproc sets the number of processes in the simulation\n");
    printf(" - N is the number of processes in the system\n");
    printf(" - Processes are numbered from 0 to N-1.\n");
    printf(" - Optional parameters --rck specify the time at which process k will request the critical region.\n");
    printf(" - at least one process must request the critical region in the simulation\n");
    printf(" - current implementation was tested with at most 3 processes\n");
}

void print_header() {
    printf("-- BEGIN HEADER --\n");
    printf("Ricart-Agrawala algorithm simulation using SMPL\n");
    printf("Course: Fault-Tolerant Distributed Systems 2016/1\n");
    printf("Professor: Elias P Duarte Jr\n");
    printf("Institution: Universidade Federal do Paraná\n");
    printf("Student: João Victor Tozatti Risso - GRR Nr.: GRR20120726\n");
    printf("-- END HEADER --\n\n");
}

int main(int argc, char **argv)
{
    const double sim_time = 100;
    static int show_usage = 0;
    static int pnum = 0;

    int event, src_pid, dst_pid, req, pid, num_requests = 0,
        p0_time = 0, p1_time = 0, p2_time = 0;
    struct timeval tp;
    item_fila recvd_msg;

    print_header();

    if (argc < 3) {
        print_usage(argv[0]);
        exit(1);
    }

    int c;
    while (1) {
        static struct option long_options[] = {
            {"help",  no_argument, &show_usage, 1},
            {"nproc", required_argument, 0, 'n'},
            {"rc0",   required_argument, 0, 'p'},
            {"rc1",   required_argument, 0, 'q'},
            {"rc2",   required_argument, 0, 'r'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "n:p:q:r:", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1) break;

        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;
            case 'n':
                pnum = (int) strtol(optarg, NULL, 10);
                break;
            case 'p':
                p0_time = (int) strtol(optarg, NULL, 10);
                ++num_requests;
                break;
            case 'q':
                p1_time = (int) strtol(optarg, NULL, 10);
                ++num_requests;
                break;
            case 'r':
                p2_time = (int) strtol(optarg, NULL, 10);
                ++num_requests;
                break;
            case '?':
                /* getopt_long already printed an error message. */
                break;
            default:
                abort();
        }
    }

    if (show_usage) {
        print_usage(argv[0]);
        exit(0);
    }

    if (pnum < 2) {
        printf("Error: Invalid number of processes! Please, specify at least 2 processes using the --nproc option.\n\n");
        print_usage(argv[0]);
        exit(1);
    } else if (pnum < 2 && p2_time != 0) {
        printf("Error: You have specified a time for process 2 (starting from 0) on a simulation with only 2 processes!\n\n");
        print_usage(argv[0]);
        exit(1);
    } else if (p0_time == 0 && p1_time == 0 && p2_time == 0) {
        printf("You have not specified any critical region requests in your simulation!\n\n");
        print_usage(argv[0]);
        exit(1);
    }

    // initialize PRNG
    gettimeofday(&tp, NULL);
    srand(tp.tv_sec + tp.tv_usec / 1000);
    seed(tp.tv_sec + tp.tv_usec / 1000, 1);

    smpl(0, "Ricart-Agrawala");
    stream(1);

    process plist = init_processes(pnum);

    printf("-- BEGIN PARAMETERS --\n");
    printf("Number of processes: %d\n", pnum);
    printf("Number of critical region requests: %d\n", num_requests);
    printf("Starting events:\n");
    if (p0_time != 0) {
        printf("  - Process 0 will request the critical region at time %d\n", p0_time);
        schedule(EV_REQUEST, p0_time, 0);
    }
    if (p1_time != 0) {
        printf("  - Process 1 will request the critical region at time %d\n", p1_time);
        schedule(EV_REQUEST, p1_time, 1);
    }
    if (p2_time != 0) {
        printf("  - Process 2 will request the critical region at time %d\n", p2_time);
        schedule(EV_REQUEST, p2_time, 2);
    }
    printf("-- END PARAMETERS --\n\n");

    printf("-- SIMULATION BEGIN --\n");
    while(time() < sim_time) {
        cause(&event, &pid);

        switch(event) {
            case EV_REQUEST:  // request
                printf("Process %d has executed a critical region REQUEST at time %g\n", pid, time());

                // muda estado do processo para WANTED
                plist[pid].state = ST_WANTED;

                // update logical clock for process pid
                plist[pid].timestamp++;
                plist[pid].request_timestamp = plist[pid].timestamp;

                // broadcast to all processes in the system
                broadcast(plist, pnum, pid);
                break;

            case EV_RECV:
                // remove mensagem da fila com maior prioridade
                recvd_msg = remove_max_filap(plist[pid].recvd_from);
                // sincroniza relógio lógico no recebimento
                plist[pid].timestamp = (plist[pid].timestamp > recvd_msg->timestamp) ?
                    plist[pid].timestamp : recvd_msg->timestamp;
                plist[pid].timestamp++;

                printf("Process %d received %s from %d at time %g\n", pid, (recvd_msg->tipo == MSG_REQUEST) ? "REQUEST" : "REPLY", recvd_msg->pid, time());

                recv(plist, pnum, pid, recvd_msg->pid, recvd_msg->timestamp, recvd_msg->tipo);
                break;
            case EV_RELEASE:  // release critical region

                printf("Process %d released the critical region at time %g\n", pid, time());

                releasecr(plist, pnum, pid);
                num_requests--;
                break;
        }

        if (num_requests == 0)
            break;
    }
    printf("-- SIMULATION END --\n");

    destroy_processes(plist, pnum);

    return 0;
}
