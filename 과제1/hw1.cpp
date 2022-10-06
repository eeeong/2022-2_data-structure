// K-Means Algorythm
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_REPEAT 30 //초기 클러스터 반복 가능 단계 수

typedef struct Point //(x,y)
{
    double x;
    double y;
} POINT;

typedef struct Cluster //클러스터
{
    POINT p;
    int ci; //클러스터 번호
} CLUSTER;

typedef struct CenterPoint //중심점
{
    POINT p;
    int ci_p_count;
} CENTERPOINT;

void PrintCenterPoint(CENTERPOINT *centers, int k, int repeat)
{
    for (int i = 0; i < k; i++)
        printf("\t클러스터 %d: 중심점 = (%f, %f)\n", i, centers[k * repeat + i].p.x, centers[k * repeat + i].p.y);
}

double Distance(POINT p1, POINT p2)
{
    double distance, x_d, y_d;
    x_d = p1.x - p2.x;
    y_d = p1.y - p2.y;
    distance = sqrt(x_d * x_d + y_d * y_d); //제곱근(루트)
    return distance;
}

double ShortDistance(POINT p, CENTERPOINT *centers, int center_count)
{
    double short_distance, distance;
    short_distance = Distance(p, centers[0].p);
    for (int i = 1; i < center_count; i++)
    {
        distance = Distance(p, centers[i].p);
        if (distance < short_distance)
            short_distance = distance;
    }
    return short_distance;
}

int ShortDistanceIdx(POINT p, CENTERPOINT *centers, int k, int repeat)
{
    int short_idx = 0;
    for (int i = 1; i < k; i++)
    {
        if (Distance(p, centers[k * repeat + i].p) < Distance(p, centers[k * repeat + short_idx].p))
            short_idx = i;
    }
    return short_idx;
}

double LongDistance(CENTERPOINT *centers, CLUSTER *points, int k, int point_count, int repeat, int ci)
{
    double long_distance = -1, distance;
    for (int i = 0; i < point_count; i++)
    {
        if (points[i].ci == ci)
        {
            distance = Distance(points[i].p, centers[k * repeat + ci].p);
            if (distance > long_distance)
                long_distance = distance;
        }
    }
    return long_distance;
}

void AllocFirstCenter(CENTERPOINT *centers, CLUSTER *points, int k, int point_count)
{
    int max_idx;
    //첫번째 좌표
    centers[0].p = points[0].p;

    //두번째 좌표
    max_idx = 1;
    for (int i = 2; i < point_count; i++)
    {
        if (Distance(centers[0].p, points[i].p) > Distance(centers[0].p, points[max_idx].p))
            max_idx = i;
    }

    centers[1].p = points[max_idx].p;

    //세번째 이상 좌표 (중심점 여러개)
    for (int i = 2; i < k; i++)
    {
        max_idx = 1; //항상 첫번째 좌표를 제외한 점들을 조사한다.
        for (int j = 2; j < point_count; j++)
        {
            if (ShortDistance(points[j].p, centers, i) > ShortDistance(points[max_idx].p, centers, i))
                max_idx = j;

            centers[i].p = points[max_idx].p;
        }
    }
}

void AllocCenter(CLUSTER *points, CENTERPOINT *centers, int k, int point_count, int repeat)
{
    for (int i = 0; i < point_count; i++)
    {
        centers[k * repeat + points[i].ci].p.x += points[i].p.x;
        centers[k * repeat + points[i].ci].p.y += points[i].p.y;
    }
    for (int i = 0; i < k; i++)
    {
        centers[k * repeat + i].p.x /= centers[k * (repeat - 1) + i].ci_p_count;
        centers[k * repeat + i].p.y /= centers[k * (repeat - 1) + i].ci_p_count;
    }
}

void AllocCluster(CLUSTER *points, CENTERPOINT *centers, int k, int point_count, int repeat)
{
    for (int i = 0; i < point_count; i++)
    {
        points[i].ci = ShortDistanceIdx(points[i].p, centers, k, repeat);
        // centers[k * repeat + i].ci_p_count++; // Ci에 속하는 점들의 개수 추가
        centers[k * repeat + points[i].ci].ci_p_count++; // Ci에 속하는 점들의 개수 추가
    }
}

void PrintLastCluster(CENTERPOINT *centers, CLUSTER *points, int k, int point_count, int repeat)
{
    double long_distance;
    printf("### 클러스터 구성 완료!! : 반복횟수 = %d\n", repeat);
    for (int i = 0; i < k; i++)
    {
        long_distance = LongDistance(centers, points, k, point_count, repeat, i);
        printf("\t클러스터 %d: 중심점 = (%f, %f), point 수 = %d, 최장 거리 = %f\n", i,
               centers[k * repeat + i].p.x, centers[k * repeat + i].p.y,
               centers[k * (repeat - 1) + i].ci_p_count, long_distance);
    }
}

bool SameCenter(CENTERPOINT *center, int k, int repeat)
{
    for (int i = 0; i < k; i++)
    {
        if (center[k * repeat + i].p.x == center[k * (repeat - 1) + i].p.x && center[k * repeat + i].p.y == center[k * (repeat - 1) + i].p.y)
            continue;
        else
            return false;
    }
    return true;
}

void RunKMeans()
{
    FILE *file = NULL;
    char file_name[16];
    int k, point_count, repeat_count = 0;
    CLUSTER *points = NULL;      //점들
    CENTERPOINT *centers = NULL; //중심점들

    printf("파일 이름과 K 값을 입력하시오 >> ");
    scanf("%s %d", file_name, &k);

    file = fopen(file_name, "r"); //읽기 모드로 파일 열기
    if (file == NULL)
    {
        printf("파일 열 수 없음\n");
        return; //프로그램 종료
    }
    // points 배열에 파일에서 읽어온 점들의 (x,y)좌표를 저장한다.
    fscanf(file, "%d", &point_count);
    points = (CLUSTER *)malloc(point_count * sizeof(CLUSTER));
    for (int i = 0; i < point_count; i++)
        fscanf(file, "%lf %lf", &points[i].p.x, &points[i].p.y);

    centers = (CENTERPOINT *)calloc(MAX_REPEAT * k, sizeof(CENTERPOINT)); // 0으로 초기화

    AllocFirstCenter(centers, points, k, point_count);
    AllocCluster(points, centers, k, point_count, repeat_count);
    printf("초기 클러스터 구성 : \n");
    PrintCenterPoint(centers, k, 0);

    while (1) //반복횟수 2회 이상
    {
        repeat_count++;
        AllocCenter(points, centers, k, point_count, repeat_count);
        if (SameCenter(centers, k, repeat_count))
        {
            PrintLastCluster(centers, points, k, point_count, repeat_count);
            break;
        }
        AllocCluster(points, centers, k, point_count, repeat_count);
        printf("%d번째 클러스터 구성 : \n", repeat_count + 1);
        PrintCenterPoint(centers, k, repeat_count);
    }

    free(points);
    free(centers);
}

int main(void)
{
    RunKMeans();

    return 0;
}