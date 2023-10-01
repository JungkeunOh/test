/*********************************************************************
*
* ANSI C Example program:
*    Acq-IntClk.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to acquire a finite amount of data
*    using the DAQ device's internal clock.
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       signal is input on the DAQ device.
*    2. Enter the minimum and maximum voltages.
*    Note: For better accuracy try to match the input range to the
*          expected voltage level of the measured signal.
*    3. Select the number of samples to acquire.
*    4. Set the rate of the acquisition.
*    Note: The rate should be AT LEAST twice as fast as the maximum
*          frequency component of the signal being acquired.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input voltage channel.
*    3. Set the rate for the sample clock. Additionally, define the
*       sample mode to be finite and set the number of samples to be
*       acquired per channel.
*    4. Call the Start function to start the acquisition.
*    5. Read all of the waveform data.
*    6. Call the Clear Task function to clear the task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O Control. For further connection information, refer
*    to your hardware reference manual.
*
*********************************************************************/


#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <string.h>
#include <iomanip>
#include <sstream>
#include "NIDAQmx.h"

using namespace std;

#define DAQmxErrChk1(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error1; else
#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else
#define V_MAX 10.0
#define V_MIN -10.0
#define Hz 100 // sampling rate 0.01sec 
// #define nSample 200 // nSample/Hz = time(sec) 
//#define del_Time 0 // main안에 int형식으로 재정의 함. 총 계측시간 중에서 앞부분을 제외하기 위하여 설정, 테스트를 위해 1초만, 나중에서는 넉넉하게 60초 정도로~~~

/* 초기값을 잡고 그 평균값을 초기값으로 설정하는 작업이 필요*/
#define cal_factor 5.014 // calibration factor !! 캘리브레이션 data
#define ini_time 5.0 // 초기값 잡는 시간

int main(int argc, char* argv[]) // operation_Time(sec) 에 따라 nSample 개수가 정해짐
{
	int32       error = 0;
	TaskHandle  taskHandle = 0;
	int32       read; // read의 값은 nSample의 값과 동일
	double Period, operation_Time, Amp;
	char* pEnd;

	bool ini_Mode = true; // 0: 초기값 안잡고 곧장 data 받기, 1: 초기값 잡고 이것을 반영한 data 받기 // 나중에 버린 data를 이용하여 초기값을 받도록 수정!!!
	int del_Time = 0;

	if (argc < 4)
	{
		Period = 0.75; // 임의로 설정
		operation_Time = 5; //임의로 설정
		Amp = 0.025 * 100.0; // 임의로 설정함 cm로 변환
	}
	else
	{
		Period = strtod(argv[1], &pEnd);
		operation_Time = strtod(argv[2], &pEnd);
		Amp = strtod(argv[3], &pEnd);
		Amp = Amp * 100.0; //cm 단위로 변경
	}
	//printf("%f\n", Amp);
	
	int iniSample = int(ini_time * Hz); // 초기값을 잡기 위한 sampling data 수
	
	//float64     data[nSample];
	double* ini_data = new double[iniSample];
	for (int i = 0; i < iniSample; i++)
	{
		ini_data[i] = 0.0;
	}
	char        errBuff1[2048] = { '\0' };
	double ini_val = 0.0; // 초기값 세팅
	double ini_tmp = 0.0;

	if (ini_Mode)
	{
		/*    calibration 데이터 계측하는 과정 시작     */
		/*********************************************/
		// DAQmx Configure Code
		/*********************************************/
		DAQmxErrChk1(DAQmxCreateTask("", &taskHandle));
		DAQmxErrChk1(DAQmxCreateAIVoltageChan(taskHandle, "Dev1/ai0", "", DAQmx_Val_Cfg_Default, V_MIN, V_MAX, DAQmx_Val_Volts, NULL));
		DAQmxErrChk1(DAQmxCfgSampClkTiming(taskHandle, "", Hz, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, iniSample));

		/*********************************************/
		// DAQmx Start Code
		/*********************************************/
		DAQmxErrChk1(DAQmxStartTask(taskHandle));

		/*********************************************/
		// DAQmx Read Code
		/*********************************************/
		//DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 1000, 10.0, DAQmx_Val_GroupByChannel, data, nSample, &read, NULL));
		//DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, nSample, 10.0, DAQmx_Val_GroupByChannel, data, nSample, &read, NULL)); // channel ai0만 읽을 예정이므로, 채널별 sample수도 nSample로, 지정함. timeout이 10초로 설정되어 있음, 즉 10초만 동작
		DAQmxErrChk1(DAQmxReadAnalogF64(taskHandle, iniSample, -1, DAQmx_Val_GroupByChannel, ini_data, iniSample, &read, NULL)); // channel ai0만 읽을 예정이므로, 채널별 sample수도 nSample로, 지정함. timeout을 무한대로 설정해 놓음(-1)
	//	printf("Acquired %d points\n", (int)read);


	Error1:
		if (DAQmxFailed(error))
			DAQmxGetExtendedErrorInfo(errBuff1, 2048);
		if (taskHandle != 0) {
			/*********************************************/
			// DAQmx Stop Code
			/*********************************************/

			DAQmxStopTask(taskHandle);
			DAQmxClearTask(taskHandle);
		}
		if (DAQmxFailed(error))
			printf("DAQmx Error: %s\n", errBuff1);
		/*    calibration 데이터 계측하는 과정 끝     */

		/*double ini_val = 0.0;
		double ini_tmp = 0.0;*/
		for (int i = 0; i < iniSample; i++) //
		{
			ini_tmp += ini_data[i];
		}
		delete[] ini_data;
		ini_val = double(ini_tmp / iniSample); // cal_factor는 아래 data 배열에서 곱해줌
		//printf("ini_val = %.3f\n", ini_val);
		// ini_val ==> 초기값 //
	}
	
	printf("ini_val = %.3f\n", ini_val);
	int nSample = int(operation_Time * Hz); // 총 sampling data 수
	int nSamData = nSample;//int((operation_Time - del_Time) * Hz); // amp를 계산하기 위하여 실제로 사용하는 sampling data 수
	int nDelData = 0;//int(del_Time * Hz + 0.001); // 앞의 제외되는 데이터 개수
	int nData_in_a_wave = int(Period * Hz + 0.001); //wave 하나에 들어가는 데이터의 개수 (정수)
	int nWave = int(nSamData / nData_in_a_wave + 0.001); // amp 계산에 실제 사용되는 wave의 개수 (정수)
	printf("nSample = %d\n", nSample);
	printf("nSamData = %d\n", nSamData);
	
	printf("nData_in_a_wave = %d\n", nData_in_a_wave);
	printf("nWave = %d\n", nWave);

	//float64     data[nSample];
	double* data = new double[nSample];
	for (int i = 0; i < nSample; i++)
	{
		data[i] = 0.0;
	}
	//double data[10000];
	char        errBuff[2048] = { '\0' };

	/*    실제 데이터 계측하는 과정 시작     */
	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk(DAQmxCreateTask("", &taskHandle));
	DAQmxErrChk(DAQmxCreateAIVoltageChan(taskHandle, "Dev1/ai0", "", DAQmx_Val_Cfg_Default, V_MIN, V_MAX, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", Hz, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, nSample));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk(DAQmxStartTask(taskHandle));

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	//DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 1000, 10.0, DAQmx_Val_GroupByChannel, data, nSample, &read, NULL));
	//DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, nSample, 10.0, DAQmx_Val_GroupByChannel, data, nSample, &read, NULL)); // channel ai0만 읽을 예정이므로, 채널별 sample수도 nSample로, 지정함. timeout이 10초로 설정되어 있음, 즉 10초만 동작
	DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, nSample, -1, DAQmx_Val_GroupByChannel, data, nSample, &read, NULL)); // channel ai0만 읽을 예정이므로, 채널별 sample수도 nSample로, 지정함. timeout을 무한대로 설정해 놓음(-1)
//	printf("Acquired %d points\n", (int)read);


Error:
	if (DAQmxFailed(error))
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
	if (taskHandle != 0) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/

		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if (DAQmxFailed(error))
		printf("DAQmx Error: %s\n", errBuff);
	/*    실제 데이터 계측하는 과정 끝     */

	//ofstream withoutIni("raw_wo_ini.dat");

	//for (int i = 0; i < nSample; i++) //
	//{
	//	withoutIni << data[i] * cal_factor << endl;
	//}
	//
	//withoutIni.close();

	// data에서 ini_val를 빼고 calibration factor를 곱하는 과정 =>>> 0점을 잡고 calibration factor를 곱함
	for (int i = 0; i < nSample; i++)
	{
		data[i] = (data[i] - ini_val) * cal_factor;
	}

	//data가 입력된 Amp의 50% 이상이면 그 때의 index를 nDelData에 넣고 끝냄.
	for (int i = 0; i < nSample; i++)
	{
		if (abs(data[i]) > Amp * 0.5)
		{
			nDelData = i + Period*Hz*20; // 50%이상인 index부터 20주기의 data를 버리도록~! ***********************************
			break;
		}
	}
//	printf("OK\n");
	printf("nDelData = %d\n", nDelData);
	if (nDelData >= nSample) return 0; // 버리는 data가 전체 sample 수보다 크거나 같으면 종료

	// Amplitude 구하기
	double sum = 0.0;
	
	//20개의 주기만 취하여 계산. 단, 23개 주기 이하로 남은 경우에는 일단 (남은 주기- 3) 만큼만...???
	if ((nSample - nDelData) > nData_in_a_wave * 23) nWave = 20;
	else if ((nSample - nDelData) > nData_in_a_wave * 7) nWave = int((nSamData - nDelData) / nData_in_a_wave + 0.001) - 3; // 7~23주기 사이로 남으면. -3은 감속 구간의 data로 가정하고 빼줌
	else return 0; // 10 주기 미만으로 남으면 계측하지 않고 종료

	double* amp = new double [nWave];
	for (int i = 0; i < nWave; i++)
	{
		sum = 0.0;
		for (int j = 0; j < nData_in_a_wave; j++)
		{
			sum = sum + 2.0 * data[j + i * nData_in_a_wave + nDelData] * data[j + i * nData_in_a_wave + nDelData]; //앞부분의 데이터를 제외하고 계산하기 위하여 nDelData 만큼 shift한 데이터를 사용
			
		}
		amp[i] = sqrt(sum / nData_in_a_wave);
	}
	double avg_Amp = 0.0;
	for (int i = 0; i < nWave; i++)
	{
		avg_Amp = avg_Amp + amp[i];
	}
	avg_Amp = avg_Amp / nWave;
	
	//ofstream 또는 FILE을 위에서 정의하면 문제가 발생함 (원인은 모름)
	//ofstream wRaw("wave_raw.dat"); // wave_raw.dat를 생성하려면 nSample 또는 nSamData만큼 data[i]를 기록하면 됨.
	
	//파일 구분을 위해 파일이름에 들어갈 날짜 및 시간 데이터 생성
	time_t now;
	time(&now);
	struct tm newtime;
	localtime_s(&newtime, &now);
	ostringstream ossYear, ossMonth, ossDay;
	ossYear << setw(4) << setfill('0') << newtime.tm_yday + 1900;
	ossMonth << setw(2) << setfill('0') << newtime.tm_mon + 1;
	ossDay << setw(2) << setfill('0') << newtime.tm_mday;

	//average amplitude 파일 생성, 계측된 amp를 조파기 PC에서 읽기위한 파일
	ofstream wAvgAmp("average_amplitude.dat");
	
	//raw data 기록 파일 생성.
	string Path = "D:\\SSMB\\Auto_Wave_Calibration\\rawdata\\rawdata_";
	Path += ossYear.str() + "_" + ossMonth.str() + "_" + ossDay.str();
	char buffer[100];
	sprintf_s(buffer, "_Set_Amp_%f.dat", Amp);
	string tmp1 = buffer;
	Path += tmp1;
	ofstream wRaw(Path);

	//for (int i = 0; i < nWave; i++) //
	//{
	//	wRaw << amp[i] << endl;
	//}
	wRaw << "평균 진폭(amp)을 계산을 시작하는 시간(s): " << (double) nDelData/Hz << endl;
	wRaw << "Time(s)" << '\t' << "Elevation(cm)" << endl;
	for (int i = 0; i < nSample; i++) //
	{
		wRaw << (double)i/Hz << '\t' << data[i] << endl;
	}
	wAvgAmp << avg_Amp/100 << endl; // 다시 m단위로 환산하여 저장, data 자체에 ini_val을 적용하고, cal_factor를 곱했으니 avg_Amp는 이대로....
//	wRaw << Period << " " <<  operation_Time <<endl; // 확인용이고 나중에 지울것임*********************************************** <= 삭제!!!!!!!
	wRaw.close();
	wAvgAmp.close();

	delete[] data;
	delete[] amp;

	// 창닫기를 키 안누르고 종료하려고 아래 두 줄 주석처리
	//printf("End of program, press Enter key to quit\n");
	//getchar();
	return 0;
}
// 추가해야할 사항들
// 1. calibration factor를 곱해야 함
// 2. 원하는 만큼의 데이터를 이용하여 amplitude 계산 후 저장