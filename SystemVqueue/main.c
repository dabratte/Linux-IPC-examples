#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/msg.h>

/*   !!!!!!!!!!!!!!!!!!  */
#define QUEUEPATH "" /* <----- add path to file used to represent queue(must be an existing file) */

#define MSGTYPE   22

/* custom message */
struct usrmsg {
        int id;
        char info;
};


struct msgbuf {
        long mtype; /* mandatory queue message member*/
        struct usrmsg msg;
};

int main(){

        struct msgbuf parent_msg = {MSGTYPE, {10,'A'}};

        key_t qkey = ftok(QUEUEPATH, 66);

        if (qkey == -1){
                perror("Obtaining token");
                exit(EXIT_FAILURE);
        }

        /* queue creation */
        int   qid = msgget(qkey, 0666 | IPC_CREAT); 
        if(qid == -1){
                perror("Creating queue");
                exit(EXIT_FAILURE);
        }

        pid_t chpid = fork();
        if (chpid == -1){
                perror("Creating child process");
                goto delete;
        }
        else if (chpid){
                if (msgsnd(qid, &parent_msg, sizeof parent_msg.msg, 0 ) == -1){
                        perror("Sending message");
                        goto delete;
                }
                printf("Press enter to terminate process\n");
                getchar();
        }
        else {
                /* We already have the queue identifier, but if you fork + execv child process should 
                 * call ftok() + msgget() without IPC_CREAT flag to obtain the identifier
                 */
                struct msgbuf child_msg = {MSGTYPE,{0}};
                if (msgrcv(qid, &child_msg, sizeof child_msg.msg, MSGTYPE, 0) == -1){
                        perror("Receiving message");
                        exit(EXIT_FAILURE);
                }
                printf("Mensaje received in child process! \n");
                printf("Id    : %d\n", child_msg.msg.id);
                printf("Info  : %c\n", child_msg.msg.info);
                exit(EXIT_SUCCESS);
        }

delete:
        if (msgctl(qid, IPC_RMID, NULL) == -1)
                perror("Eliminando cola : ");
        return 0;

}

