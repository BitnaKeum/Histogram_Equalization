#include <iostream>
#include <math.h>
#include <windows.h>

#pragma warning(disable : 4996)


void MemoryClear(UCHAR** buf);
UCHAR** memory_alloc2D(int width, int height);
void DrawCDF(float cdf[256], int x_origin, int y_origin);
void DrawHistogram(float histogram[256], int x_origin, int y_origin);


HWND hwnd;
HDC hdc;



int main(void)
{
	system("color F0");
	hwnd = GetForegroundWindow();
	hdc = GetWindowDC(hwnd);

	int width = 512;	// 이미지 파일의 가로 길이
	int height = 512;	// 이미지 파일의 세로 길이

	float Image_Histogram[256] = { 0, };		// 원본 이미지의 히스토그램
	float Image_equal_Histogram[256] = { 0, };	// histogram equalization을 한 히스토그램 (uniform distribution)
	float Image_CDF[256] = { 0, };		  // 원본 이미지의 CDF
	float Image_equal_CDF[256] = { 0, };  // histogram equalization을 한 CDF

	FILE* Input_file = fopen("barbara.raw", "rb");	// Input 이미지
	FILE* Output_file = fopen("output.raw", "wb");  // histogram equalization한 출력 이미지
	UCHAR** Input_data = memory_alloc2D(width, height);  // Input 이미지의 픽셀 값 저장
	UCHAR** Output_data = memory_alloc2D(width, height); // 출력할 이미지의 픽셀 값 저장


	int sum = 0;

	if (!Input_file) {	// 파일이 해당 경로에 없는 경우
		printf("Can not open file.");
		return -1;
	}

	// 이미지를 읽어와 밝기 값 하나하나를 저장
	fread(&Input_data[0][0], sizeof(UCHAR), width * height, Input_file);


	// 원본 이미지 히스토그램 구하기
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int data = Input_data[i][j];
			Image_Histogram[data] += 1;	// 해당 밝기 값의 빈도 수 카운트
		}
	}
	DrawHistogram(Image_Histogram, 30, 400); // 히스토그램 출력
	/*for (int i = 0; i < 256; i++) {
		printf("[%d]: %f\n", i, Image_Histogram[i]);
	}*/


	// 원본 이미지 CDF 구하기 = Histogram Equalization
	sum = 0;
	for (int i = 0; i < 256; i++) {
		sum += (int)Image_Histogram[i];
		Image_CDF[i] = (float)sum / (width * height);	// 전체 크기로 나눠서 확률 구하기
		//printf("[%d]: %f\n", i, Image_CDF[i]);
	}
	DrawCDF(Image_CDF, 30, 400);	// 원본 이미지의 CDF 출력


	// Histogram equalization한 히스토그램 구하기
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < Image_Histogram[i]; j++) {	// 원본 이미지에서 밝기 i의 빈도 수만큼 반복 (총 width*height)
			int index = (int)255 * Image_CDF[i]; // 0~255 범위로 만듦 (Image_CDF는 r을 변환한 s와 같다)
			Image_equal_Histogram[index] += 1;   // histogram equlization한 밝기 값의 빈도수 저장
		}
	}
	DrawHistogram(Image_equal_Histogram, 400, 400); // histogram equlization한 히스토그램 출력
	/*for (int i = 0; i < 256; i++) {
		printf("[%d]: %f\n", i, Image_equal_Histogram[i]);
	}*/


	// histogram equlization한 CDF 구하기
	sum = 0;
	for (int i = 0; i < 256; i++) {
		sum += (int)Image_equal_Histogram[i];
		Image_equal_CDF[i] = (float)sum / (width * height); // 전체 크기로 나눠서 확률 구하기
		//printf("[%d]: %f\n", i, Image_equal_CDF[i]);
	}
	DrawCDF(Image_equal_CDF, 400, 400); // histogram equlization한 CDF 출력




	// 이미지 출력파일 픽셀 값 설정
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int brightness = Input_data[i][j];	// 원본 이미지의 한 픽셀의 밝기 값
			Output_data[i][j] = Image_CDF[brightness] * 255;  // histogram equalization한 값으로 저장
		}
	}
	
	// Output_data의 데이터로 Output_file을 생성
	fwrite(&Output_data[0][0], sizeof(UCHAR), width * height, Output_file);

	

	MemoryClear(Input_data);	// 메모리 해제
	MemoryClear(Output_data);
	fclose(Input_file);		// 파일 close
	fclose(Output_file);

	return 0;
}



// 메모리를 해제하는 함수
void MemoryClear(UCHAR** buf) {
	if (buf) {
		free(buf[0]);
		free(buf);
		buf = NULL;
	}
}

// 2차원 배열에 동적 메모리를 할당하는 함수
UCHAR** memory_alloc2D(int width, int height)
{
	UCHAR** ppMem2D = 0;
	int	i;

	//arrary of pointer
	ppMem2D = (UCHAR**)calloc(sizeof(UCHAR*), height);
	if (ppMem2D == 0) {
		return 0;
	}

	*ppMem2D = (UCHAR*)calloc(sizeof(UCHAR), height * width);
	if ((*ppMem2D) == 0) {//free the memory of array of pointer        
		free(ppMem2D);
		return 0;
	}

	for (i = 1; i < height; i++) {
		ppMem2D[i] = ppMem2D[i - 1] + width;
	}

	return ppMem2D;
}

// CDF를 그리는 함수
void DrawCDF(float cdf[256], int x_origin, int y_origin) {
	for (int CurX = 0; CurX < 256; CurX++) {
		for (int CurY = 0; CurY < cdf[CurX]; CurY++) {
			MoveToEx(hdc, x_origin + CurX, y_origin, 0);
			SetPixel(hdc, x_origin + CurX, y_origin - cdf[CurX] * 100, RGB(0, 0, 255));
		}
	}
}

// Histogram을 출력하는 함수
void DrawHistogram(float histogram[256], int x_origin, int y_origin) {
	MoveToEx(hdc, x_origin, y_origin, 0);
	LineTo(hdc, x_origin + 255, y_origin);

	MoveToEx(hdc, x_origin, 100, 0);
	LineTo(hdc, x_origin, y_origin);

	for (int CurX = 0; CurX < 256; CurX++) {
		for (int CurY = 0; CurY < histogram[CurX]; CurY++) {
			MoveToEx(hdc, x_origin + CurX, y_origin, 0);
			LineTo(hdc, x_origin + CurX, y_origin - histogram[CurX] / 50);
		}
	}
}
