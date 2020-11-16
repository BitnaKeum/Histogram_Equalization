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

	int width = 512;	// �̹��� ������ ���� ����
	int height = 512;	// �̹��� ������ ���� ����

	float Image_Histogram[256] = { 0, };		// ���� �̹����� ������׷�
	float Image_equal_Histogram[256] = { 0, };	// histogram equalization�� �� ������׷� (uniform distribution)
	float Image_CDF[256] = { 0, };		  // ���� �̹����� CDF
	float Image_equal_CDF[256] = { 0, };  // histogram equalization�� �� CDF

	FILE* Input_file = fopen("barbara.raw", "rb");	// Input �̹���
	FILE* Output_file = fopen("output.raw", "wb");  // histogram equalization�� ��� �̹���
	UCHAR** Input_data = memory_alloc2D(width, height);  // Input �̹����� �ȼ� �� ����
	UCHAR** Output_data = memory_alloc2D(width, height); // ����� �̹����� �ȼ� �� ����


	int sum = 0;

	if (!Input_file) {	// ������ �ش� ��ο� ���� ���
		printf("Can not open file.");
		return -1;
	}

	// �̹����� �о�� ��� �� �ϳ��ϳ��� ����
	fread(&Input_data[0][0], sizeof(UCHAR), width * height, Input_file);


	// ���� �̹��� ������׷� ���ϱ�
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int data = Input_data[i][j];
			Image_Histogram[data] += 1;	// �ش� ��� ���� �� �� ī��Ʈ
		}
	}
	DrawHistogram(Image_Histogram, 30, 400); // ������׷� ���
	/*for (int i = 0; i < 256; i++) {
		printf("[%d]: %f\n", i, Image_Histogram[i]);
	}*/


	// ���� �̹��� CDF ���ϱ� = Histogram Equalization
	sum = 0;
	for (int i = 0; i < 256; i++) {
		sum += (int)Image_Histogram[i];
		Image_CDF[i] = (float)sum / (width * height);	// ��ü ũ��� ������ Ȯ�� ���ϱ�
		//printf("[%d]: %f\n", i, Image_CDF[i]);
	}
	DrawCDF(Image_CDF, 30, 400);	// ���� �̹����� CDF ���


	// Histogram equalization�� ������׷� ���ϱ�
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < Image_Histogram[i]; j++) {	// ���� �̹������� ��� i�� �� ����ŭ �ݺ� (�� width*height)
			int index = (int)255 * Image_CDF[i]; // 0~255 ������ ���� (Image_CDF�� r�� ��ȯ�� s�� ����)
			Image_equal_Histogram[index] += 1;   // histogram equlization�� ��� ���� �󵵼� ����
		}
	}
	DrawHistogram(Image_equal_Histogram, 400, 400); // histogram equlization�� ������׷� ���
	/*for (int i = 0; i < 256; i++) {
		printf("[%d]: %f\n", i, Image_equal_Histogram[i]);
	}*/


	// histogram equlization�� CDF ���ϱ�
	sum = 0;
	for (int i = 0; i < 256; i++) {
		sum += (int)Image_equal_Histogram[i];
		Image_equal_CDF[i] = (float)sum / (width * height); // ��ü ũ��� ������ Ȯ�� ���ϱ�
		//printf("[%d]: %f\n", i, Image_equal_CDF[i]);
	}
	DrawCDF(Image_equal_CDF, 400, 400); // histogram equlization�� CDF ���




	// �̹��� ������� �ȼ� �� ����
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int brightness = Input_data[i][j];	// ���� �̹����� �� �ȼ��� ��� ��
			Output_data[i][j] = Image_CDF[brightness] * 255;  // histogram equalization�� ������ ����
		}
	}
	
	// Output_data�� �����ͷ� Output_file�� ����
	fwrite(&Output_data[0][0], sizeof(UCHAR), width * height, Output_file);

	

	MemoryClear(Input_data);	// �޸� ����
	MemoryClear(Output_data);
	fclose(Input_file);		// ���� close
	fclose(Output_file);

	return 0;
}



// �޸𸮸� �����ϴ� �Լ�
void MemoryClear(UCHAR** buf) {
	if (buf) {
		free(buf[0]);
		free(buf);
		buf = NULL;
	}
}

// 2���� �迭�� ���� �޸𸮸� �Ҵ��ϴ� �Լ�
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

// CDF�� �׸��� �Լ�
void DrawCDF(float cdf[256], int x_origin, int y_origin) {
	for (int CurX = 0; CurX < 256; CurX++) {
		for (int CurY = 0; CurY < cdf[CurX]; CurY++) {
			MoveToEx(hdc, x_origin + CurX, y_origin, 0);
			SetPixel(hdc, x_origin + CurX, y_origin - cdf[CurX] * 100, RGB(0, 0, 255));
		}
	}
}

// Histogram�� ����ϴ� �Լ�
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
