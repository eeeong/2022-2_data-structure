// K-Means Algorythm
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

void PrintCenterPoint(CENTERPOINT *centers, int k)
{
    for (int i = 0; i < k; i++)
        printf("\t클러스터 %d: 중심점 = (%f, %f)\n", i, centers[i].p.x, centers[i].p.y);
}

//두 점의 거리를 리턴하는 함수
double Distance(POINT p1, POINT p2)
{
    double distance, x_d, y_d;
    x_d = p1.x - p2.x;
    y_d = p1.y - p2.y;
    distance = sqrt(x_d * x_d + y_d * y_d); //제곱근(루트)
    return distance;
}

// 모든 중심점과 비교하여 가장 짧은 거리를 리턴하는 함수
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

// 모든 중심점과 비교하여 가장 가까운 중심점의 인덱스를 리턴하는 함수
int ShortDistanceIdx(POINT p, CENTERPOINT *centers, int k)
{
    int short_idx = 0;
    for (int i = 1; i < k; i++)
    {
        if (Distance(p, centers[i].p) < Distance(p, centers[short_idx].p))
            short_idx = i;
    }
    return short_idx;
}

// 군집 내의 점들 중 중심점과의 거리가 가장 긴 거리를 리턴
double LongDistance(CENTERPOINT *centers, CLUSTER *points, int point_count, int ci)
{
    double long_distance = -1, distance;
    for (int i = 0; i < point_count; i++)
    {
        if (points[i].ci == ci)
        {
            distance = Distance(points[i].p, centers[ci].p);
            if (distance > long_distance)
                long_distance = distance;
        }
    }
    return long_distance;
}

// 초기 중심점 설정 함수
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

// 중심점 설정 함수
void AllocCenter(CLUSTER *points, CENTERPOINT *centers, CENTERPOINT *centers_before, int k, int point_count)
{
    for (int i = 0; i < point_count; i++)
    {
        centers[points[i].ci].p.x += points[i].p.x;
        centers[points[i].ci].p.y += points[i].p.y;
    }
    for (int i = 0; i < k; i++)
    {
        centers[i].p.x /= centers_before[i].ci_p_count;
        centers[i].p.y /= centers_before[i].ci_p_count;
    }
}

// 클러스터의 번호를 배정하는 함수
void AllocCluster(CLUSTER *points, CENTERPOINT *centers, int k, int point_count)
{
    for (int i = 0; i < point_count; i++)
    {
        points[i].ci = ShortDistanceIdx(points[i].p, centers, k);
        centers[points[i].ci].ci_p_count++; // Ci에 속하는 점들의 개수 추가
    }
}

// 마지막 클러스터 출력 함수
void PrintLastCluster(CENTERPOINT *centers, CENTERPOINT *centers_before, CLUSTER *points, int k, int point_count)
{
    double long_distance;
    for (int i = 0; i < k; i++)
    {
        long_distance = LongDistance(centers, points, point_count, i);
        printf("\t클러스터 %d: 중심점 = (%f, %f), point 수 = %d, 최장 거리 = %f\n", i,
               centers[i].p.x, centers[i].p.y, centers_before[i].ci_p_count, long_distance);
    }
}

// 중심점이 이전 중심점과 같은지 비교하는 함수
bool SameCenter(CENTERPOINT *centers, CENTERPOINT *centers_before, int k)
{
    for (int i = 0; i < k; i++)
    {
        if (centers[i].p.x == centers_before[i].p.x && centers[i].p.y == centers_before[i].p.y)
            continue;
        else
            return false; //중심점이 다르다
    }
    return true; //중심점이 같다
}

// centers_before은 center으로 복사, centers는 0으로 초기화하는 함수
void SetCenters(CENTERPOINT *centers, CENTERPOINT *centers_before, int k)
{
    for (int i = 0; i < k; i++)
    {
        centers_before[i].p = centers[i].p;
        centers_before[i].ci_p_count = centers[i].ci_p_count;

        centers[i].p.x = 0;
        centers[i].p.y = 0;
        centers[i].ci_p_count = 0;
    }
}

// K-Means 알고리즘 실행 함수
void RunKMeans()
{
    FILE *file = NULL;
    char file_name[16];
    int k, point_count, repeat_count = 0;
    CLUSTER *points = NULL;                              //점들
    CENTERPOINT *centers = NULL, *centers_before = NULL; //중심점들

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

    centers = (CENTERPOINT *)calloc(k, sizeof(CENTERPOINT));        // 0으로 초기화
    centers_before = (CENTERPOINT *)calloc(k, sizeof(CENTERPOINT)); // 0으로 초기화

    AllocFirstCenter(centers, points, k, point_count);
    AllocCluster(points, centers, k, point_count);
    printf("초기 클러스터 구성 : \n");
    PrintCenterPoint(centers, k);

    while (1) //반복횟수 2회 이상
    {
        repeat_count++;

        SetCenters(centers, centers_before, k);
        AllocCenter(points, centers, centers_before, k, point_count);
        if (SameCenter(centers, centers_before, k))
        {
            printf("### 클러스터 구성 완료!! : 반복횟수 = %d\n", repeat_count);
            PrintLastCluster(centers, centers_before, points, k, point_count);
            break;
        }
        AllocCluster(points, centers, k, point_count);
        printf("%d번째 클러스터 구성 : \n", repeat_count + 1);
        PrintCenterPoint(centers, k);
    }

    free(points);
    free(centers);
    free(centers_before);
}

int main(void)
{
    RunKMeans();

    return 0;
}