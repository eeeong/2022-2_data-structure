// 22112072 박예원
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#define MAX_PRIORITY 10

typedef struct job
{
    int arrive_t;   //도착시간
    int quantity_t; //작업분량
    int priority;   //우선순위
} Job;

typedef struct run_job
{
    Job job;
    int remain_t; //남은 작업분량
    int end_t;    //종료시간
} RunJob;

typedef struct End_job
{
    int run_t;  //실행시간 (종료시간 - 도착시간)
    int wait_t; //대기시간 (실행시간 - 작업분량)
} EndJob;

typedef struct priority_list
{
    int job_num;           //작업 수
    double average_wait_t; //평균 대기시간
} PriorityList;

EndJob *get_end_jobs(RunJob *run_jobs, int k);
void set_priority_list(PriorityList priority_list[], RunJob *run_jobs, EndJob *end_jobs, int k);

void fifo_scheduling(RunJob *run_jobs, int k, int all_jobs_end_t);

int compare_priority(RunJob a, RunJob b);
int heap_full(int n, int max_heap_size);
int heap_empty(int n);
void insert_max_heap(int *max_heap, RunJob *run_jobs, int insert_n, int *n, int max_heap_size);
int delete_max_heap(int *max_heap, RunJob *run_jobs, int *n);
void priority_scheduling(RunJob *run_jobs, int k, int all_jobs_end_t);

void run();

int main(void)
{
    run();

    return 0;
}

EndJob *get_end_jobs(RunJob *run_jobs, int k)
{
    EndJob *end_jobs = (EndJob *)malloc(k * sizeof(EndJob));

    for (int i = 0; i < k; i++)
    {
        end_jobs[i].run_t = run_jobs[i].end_t - run_jobs[i].job.arrive_t;
        end_jobs[i].wait_t = end_jobs[i].run_t - run_jobs[i].job.quantity_t;
    }
    return end_jobs;
}

void set_priority_list(PriorityList priority_list[], RunJob *run_jobs, EndJob *end_jobs, int k)
{
    int priority;

    for (int i = 0; i < k; i++)
    {
        priority = run_jobs[i].job.priority - 1;
        priority_list[priority].job_num += 1;
        priority_list[priority].average_wait_t += end_jobs[i].wait_t;
    }
    for (int i = 0; i < MAX_PRIORITY; i++)
        priority_list[i].average_wait_t /= (double)priority_list[i].job_num;
}

void fifo_scheduling(RunJob *run_jobs, int k, int all_jobs_end_t)
{
    EndJob *end_jobs;
    int now_run_t = run_jobs[0].job.arrive_t;
    double average_run_t = 0, average_wait_t = 0;

    for (int i = 0; i < k; i++)
    {
        now_run_t += run_jobs[i].job.quantity_t;
        run_jobs[i].end_t = now_run_t;
        if ((i < k - 1) && run_jobs[i].end_t < run_jobs[i + 1].job.arrive_t)
            now_run_t = run_jobs[i + 1].job.arrive_t;
    }
    end_jobs = get_end_jobs(run_jobs, k);
    for (int i = 0; i < k; i++)
    {
        average_run_t += end_jobs[i].run_t;
        average_wait_t += end_jobs[i].wait_t;
    }
    average_run_t /= (double)k;
    average_wait_t /= (double)k;

    printf("===== FIFO Scheduling의 실행 결과 ===== \n");
    printf("\t작업수 = %d, 종료시간 = %d, 평균 실행시간 = %.2f, 평균 대기시간 = %.2f \n\n", k, all_jobs_end_t, average_run_t, average_wait_t);

    free(end_jobs);
}

int compare_priority(RunJob a, RunJob b)
{ // 1:a가 우선, 0:b가 우선
    if (a.job.priority > b.job.priority)
        return 1;
    else if (a.job.priority == b.job.priority)
    {
        if (a.remain_t < b.remain_t)
            return 1;
        else if (a.remain_t == b.remain_t)
        {
            if (a.job.arrive_t < b.job.arrive_t)
                return 1;
        }
    }
    return 0;
}

int heap_full(int n, int max_heap_size)
{ // 1 : heap full
    if (n == max_heap_size - 1)
        return 1;
    return 0;
}

int heap_empty(int n)
{
    if (n == 0)
        return 1; // 1 : heap empty;
    return 0;
}

void insert_max_heap(int *max_heap, RunJob *run_jobs, int insert_n, int *n, int max_heap_size)
{
    int i;
    if (heap_full(*n, max_heap_size))
    {
        printf("힙이 꽉 찼습니다! \n");
        return;
    }
    i = ++(*n);
    while ((i != 1) && compare_priority(run_jobs[insert_n], run_jobs[max_heap[i / 2]]))
    {
        max_heap[i] = max_heap[i / 2];
        i /= 2;
    }
    max_heap[i] = insert_n;
}

int delete_max_heap(int *max_heap, RunJob *run_jobs, int *n)
{
    int parent_i = 1, child_i = 2, delete_n, tmp;
    if (heap_empty(*n)) //힙이 비어있을 때
        return -1;
    delete_n = max_heap[1];
    tmp = max_heap[(*n)--];
    while (child_i <= *n)
    {
        if ((child_i < *n) && compare_priority(run_jobs[max_heap[child_i + 1]], run_jobs[max_heap[child_i]]))
            child_i++;
        if (compare_priority(run_jobs[tmp], run_jobs[max_heap[child_i]]))
            break;
        max_heap[parent_i] = max_heap[child_i];
        parent_i = child_i;
        child_i *= 2;
    }
    max_heap[parent_i] = tmp;
    return delete_n;
}

void priority_scheduling(RunJob *run_jobs, int k, int all_jobs_end_t)
{
    int now_run_t = run_jobs[0].job.arrive_t, now_job_n = 0, come_job_n = 1;
    int *max_heap = (int *)malloc(k * sizeof(int)), heap_n = 0;
    double average_run_t = 0.0;
    EndJob *end_jobs;
    PriorityList priority_list[MAX_PRIORITY] = {
        0,
    };

    while (now_run_t < all_jobs_end_t)
    {
        if (come_job_n > 0)
        {
            int remain_t = run_jobs[now_job_n].remain_t;
            int come_run_t = run_jobs[come_job_n].job.arrive_t - now_run_t;
            if (remain_t < come_run_t)
            {
                run_jobs[now_job_n].end_t = now_run_t + run_jobs[now_job_n].remain_t;
                run_jobs[now_job_n].remain_t = 0;
                now_run_t = run_jobs[now_job_n].end_t;
                now_job_n = delete_max_heap(max_heap, run_jobs, &heap_n);
                if (now_job_n < 0)
                {
                    now_run_t = run_jobs[come_job_n].job.arrive_t;
                    now_job_n = come_job_n;
                    come_job_n += 1;
                    if (come_job_n >= k)
                        come_job_n = -1;
                }
            }
            else if (remain_t == come_run_t)
            {
                run_jobs[now_job_n].end_t = now_run_t + run_jobs[now_job_n].remain_t;
                run_jobs[now_job_n].remain_t = 0;
                now_run_t = run_jobs[now_job_n].end_t;
                now_job_n = delete_max_heap(max_heap, run_jobs, &heap_n);
                if (now_job_n < 0)
                    now_job_n = come_job_n;
                else if (compare_priority(run_jobs[now_job_n], run_jobs[come_job_n]))
                {
                    insert_max_heap(max_heap, run_jobs, come_job_n, &heap_n, k);
                }
                else
                {
                    insert_max_heap(max_heap, run_jobs, now_job_n, &heap_n, k);
                    now_job_n = come_job_n;
                }
                come_job_n += 1;
                if (come_job_n >= k)
                    come_job_n = -1;
            }
            else
            {
                run_jobs[now_job_n].remain_t -= (run_jobs[come_job_n].job.arrive_t - now_run_t);
                now_run_t = run_jobs[come_job_n].job.arrive_t;
                if (compare_priority(run_jobs[now_job_n], run_jobs[come_job_n]))
                {
                    insert_max_heap(max_heap, run_jobs, come_job_n, &heap_n, k);
                }
                else
                {
                    insert_max_heap(max_heap, run_jobs, now_job_n, &heap_n, k);
                    now_job_n = come_job_n;
                }
                come_job_n += 1;
                if (come_job_n >= k)
                    come_job_n = -1;
            }
        }
        else
        {
            run_jobs[now_job_n].end_t = now_run_t + run_jobs[now_job_n].remain_t;
            run_jobs[now_job_n].remain_t = 0;
            now_run_t = run_jobs[now_job_n].end_t;
            if (heap_empty(heap_n))
                break;
            now_job_n = delete_max_heap(max_heap, run_jobs, &heap_n);
        }
    }
    end_jobs = get_end_jobs(run_jobs, k);
    for (int i = 0; i < k; i++)
        average_run_t += end_jobs[i].run_t;
    average_run_t /= (double)k;

    set_priority_list(priority_list, run_jobs, end_jobs, k);
    printf("===== Priority Scheduling의 실행 결과 ===== \n");
    printf("\t작업수 = %d, 종료시간 = %d, 평균 실행시간 = %.2f \n\n", k, all_jobs_end_t, average_run_t);
    for (int i = MAX_PRIORITY - 1; i >= 0; i--)
    {
        if (priority_list[i].job_num > 0)
            printf("\t우선순위 %2d : 작업수 = %d, 평균 대기시간 = %.2f \n", i + 1, priority_list[i].job_num, priority_list[i].average_wait_t);
    }
    free(max_heap);
    free(end_jobs);
}

void run()
{
    FILE *file = NULL;
    char f_name[32];
    int k, all_jobs_end_t = 0;
    RunJob *run_jobs;

    printf("입력파일 이름을 입력하세요 >> ");
    scanf("%s", f_name);
    file = fopen(f_name, "r");
    if (file == NULL)
    {
        printf("파일 열기 실패\n");
        return;
    }

    fscanf(file, "%d", &k);
    run_jobs = (RunJob *)malloc(k * sizeof(RunJob));
    for (int i = 0; i < k; i++)
    {
        fscanf(file, "%d %d %d", &run_jobs[i].job.arrive_t, &run_jobs[i].job.quantity_t, &run_jobs[i].job.priority);
        run_jobs[i].remain_t = run_jobs[i].job.quantity_t;
        all_jobs_end_t += run_jobs[i].job.quantity_t;
    }
    all_jobs_end_t += run_jobs[0].job.arrive_t;
    fclose(file);

    fifo_scheduling(run_jobs, k, all_jobs_end_t);
    priority_scheduling(run_jobs, k, all_jobs_end_t);

    free(run_jobs);
}