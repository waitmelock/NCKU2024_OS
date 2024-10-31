#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../include/command.h"
#include "../include/builtin.h"

// ======================= requirement 2.3 =======================
/**
 * @brief 
 * Redirect command's stdin and stdout to the specified file descriptor
 * If you want to implement ( < , > ), use "in_file" and "out_file" included the cmd_node structure
 * If you want to implement ( | ), use "in" and "out" included the cmd_node structure.
 *
 * @param p cmd_node structure
 * 
 */
void redirection(struct cmd_node *p) {
    //處理輸入重定向
    if (p->in_file != NULL) {
        int in_fd = open(p->in_file, O_RDONLY);
        if (in_fd == -1) {
            perror("open in_file failed");
            exit(EXIT_FAILURE);
        }
        if (dup2(in_fd, STDIN_FILENO) == -1) {
            perror("dup2 in_file failed");
            exit(EXIT_FAILURE);
        }
    } else if (p->in != -1) {
        if (dup2(p->in, STDIN_FILENO) == -1) {
            perror("dup2 in failed");
            exit(EXIT_FAILURE);
        }
    }

    // 處理輸出重定向
    if (p->out_file != NULL) {
        int out_fd = open(p->out_file, O_WRONLY | O_CREAT|O_TRUNC, 0644);
        if (out_fd == -1) {
            perror("open out_file failed");
            exit(EXIT_FAILURE);
        }
        if (dup2(out_fd, STDOUT_FILENO) == -1) {
            perror("dup2 out_file failed");
            exit(EXIT_FAILURE);
        }
    } else if (p->out != -1) {
        if (dup2(p->out, STDOUT_FILENO) == -1) {
            perror("dup2 out failed");
            exit(EXIT_FAILURE);
        }
    }
}
// ===============================================================

// ======================= requirement 2.2 =======================
/**
 * @brief 
 * Execute external command
 * The external command is mainly divided into the following two steps:
 * 1. Call "fork()" to create child process
 * 2. Call "execvp()" to execute the corresponding executable file
 * @param p cmd_node structure
 * @return int 
 * Return execution status
 */
int spawn_proc(struct cmd_node *p)
{
	pid_t pid = fork();
	pid_t wpid;
    int status;

    if (pid == -1) {
        perror("fork failed");
        return -1;
    } else if (pid == 0) {// 子進程
		redirection(p);
        if (execvp(p->args[0], p->args) == -1) {
            perror("execvp failed");
            _exit(EXIT_FAILURE); // 使用 _exit 以確保子進程立即退出
            return 0;
        }
    } else {// 父進程
        do {
            wpid = waitpid(pid, &status, 0);
            if (wpid == -1) {
                perror("waitpid failed");
                return -1;
            }

        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    if(strcmp(p->args[0],"exit") == 0){
        return 0;
    }

    return 1;
}
// ===============================================================


// ======================= requirement 2.4 =======================
/**
 * @brief 
 * Use "pipe()" to create a communication bridge between processes
 * Call "spawn_proc()" in order according to the number of cmd_node
 * @param cmd Command structure  
 * @return int
 * Return execution status 
 */
int fork_cmd_node(struct cmd *cmd)
{
    struct cmd_node *current = cmd->head;
    int num_cmds = 0;
    // 計算命令數量
    for (; current != NULL; num_cmds++) {
        current = current->next;
    }

    // 創建管道
    int pipefds[2 * (num_cmds - 1)];
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipefds + i * 2) == -1) {
            perror("pipe failed");
            return -1;
        }
    }

    // 存儲所有子進程的 pid
    pid_t *pids = malloc(sizeof(pid_t) * num_cmds);
    if (pids == NULL) {
        perror("malloc failed");
        return -1;
    }

    // 執行每個命令
    current = cmd->head;
    for (int i = 0; i < num_cmds; i++) {
        pids[i] = fork();
        
        if (pids[i] == -1) {
            perror("fork failed");
            free(pids);
            return -1;
        }
        
        if (pids[i] == 0) {  // 子進程
            // 設置輸入端（除了第一個命令）
            if (i > 0) {
                if (dup2(pipefds[(i-1) * 2], STDIN_FILENO) == -1) {
                    perror("dup2 input failed");
                    _exit(EXIT_FAILURE);
                }
            }
            
            // 設置輸出端（除了最後一個命令）
            if (i < num_cmds - 1) {
                if (dup2(pipefds[i * 2 + 1], STDOUT_FILENO) == -1) {
                    perror("dup2 output failed");
                    _exit(EXIT_FAILURE);
                }
            }

            // 處理文件重定向
            if (current->in_file != NULL) {
                int in_fd = open(current->in_file, O_RDONLY);
                if (in_fd == -1) {
                    perror("open input file failed");
                    _exit(EXIT_FAILURE);
                }
                if (dup2(in_fd, STDIN_FILENO) == -1) {
                    perror("dup2 input file failed");
                    _exit(EXIT_FAILURE);
                }
                close(in_fd);
            }
            
            if (current->out_file != NULL) {
                int out_fd = open(current->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (out_fd == -1) {
                    perror("open output file failed");
                    _exit(EXIT_FAILURE);
                }
                if (dup2(out_fd, STDOUT_FILENO) == -1) {
                    perror("dup2 output file failed");
                    _exit(EXIT_FAILURE);
                }
                close(out_fd);
            }

            // 關閉所有管道
            for (int j = 0; j < 2 * (num_cmds - 1); j++) {
                close(pipefds[j]);
            }

            // 執行命令
            if (execvp(current->args[0], current->args) == -1) {
                perror("execvp failed");
                _exit(EXIT_FAILURE);
            }
        }
        
        current = current->next;
    }

    // 父進程關閉所有管道
    for (int i = 0; i < 2 * (num_cmds - 1); i++) {
        close(pipefds[i]);
    }

    // 等待所有子進程結束
    int status;
    int last_status = 1;  // 預設返回1表示成功
    for (int i = 0; i < num_cmds; i++) {
        waitpid(pids[i], &status, 0);
        if (i == num_cmds - 1) {  // 保存最後一個命令的狀態
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == 0) {
                    last_status = 1;  // 成功執行
                } else {
                    last_status = WEXITSTATUS(status);  // 保存錯誤狀態
                }
            }
        }
    }

    free(pids);
    return last_status;
}
// ===============================================================


void shell()
{
	while (1) {
		printf(">>> $ ");
		char *buffer = read_line();
		if (buffer == NULL)
			continue;

		struct cmd *cmd = split_line(buffer);
		
		int status = -1;
		// only a single command
		struct cmd_node *temp = cmd->head;
		
		if(temp->next == NULL){
			status = searchBuiltInCommand(temp);
			if (status != -1){
				int in = dup(STDIN_FILENO), out = dup(STDOUT_FILENO);
				if( in == -1 | out == -1)
					perror("dup");
				redirection(temp);
				status = execBuiltInCommand(status,temp);

				// recover shell stdin and stdout
				if (temp->in_file)  dup2(in, 0);
				if (temp->out_file){
					dup2(out, 1);
				}
				close(in);
				close(out);
			}
			else{
				//external command
				status = spawn_proc(cmd->head);
			}
		}
		// There are multiple commands ( | )
		else{
			
			status = fork_cmd_node(cmd);
		}
		// free space
		while (cmd->head) {
			
			struct cmd_node *temp = cmd->head;
      		cmd->head = cmd->head->next;
			free(temp->args);
   	    	free(temp);
   		}
		free(cmd);
		free(buffer);
		
		if (status == 0)
			break;
	}
}
