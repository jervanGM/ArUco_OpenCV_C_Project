#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif
using namespace cv;
using namespace std;

void figures(Mat& frame, vector<Point3d>objectPoints2, Mat rvec_front, Mat tvec_front, Mat cameraMatrix, Mat distorsion_vector, vector<Point2d> projectedPoints, int figure);

int main(int argc, char* argv[])
{
	/////////////////////////////////////////!!Lectura y escritura de video!!///////////////////////////////////////////////////////
	VideoCapture capture(0);
	int frame_width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	int frame_height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	VideoWriter output("output_aruco.avi", CV_FOURCC('M', 'J', 'P', 'G'), 10, Size(frame_width, frame_height));
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//**********************************************Declaración de variables*****************************************************//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Mat frame, frameg, cannyimg, thresimg, drawingg, frame2;
	vector<vector<Point>> contours;
	vector<vector<Point>> contours_ARC;
	vector<vector<Point>> approx;
	vector<vector<Point>> approx_ARC;
	vector<Point2d> imagePoints;
	vector<Point3d>	objectPoints;
	vector<Point3d>	objectPoints2;
	vector<Point3d>	objectPoints5;
	vector<Point3d> imagePointsH;
	vector<Point3d>pointID;
	vector<Point2d> projectedPoints;
	vector<Mat>aruco_database;
	Mat rvec_front;
	Mat tvec_front;
	RNG rng;
	bool ret = true;
	time_t start, end;
	double epsilon;
	char c = 0;
	int fig = 0;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (!capture.isOpened()) {
		cout << "No se ha podido abrir el archivo" << endl;
	}
	////////////////////////////!!Insertar en caso de utilizar otra camara!!///////////////////////////////////////////////////////

	float cam[3][3] = { 1.0603358680554393e+03, 0, 320, 0, 1.0603358680554393e+03, 240, 0, 0,1 };
	float dist[1][5] = { 1.4303235808757833e-01, -1.9588829186853554e-01, 0, 0,3.6016645843108015e+00 };
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Dadabase de arucos en forma matricial
	float ar0[4][4] = { 255,0,255,255,
						0,255,0,255,
						0,0,255,255,
						0,0,255,0 };
	float ar1[4][4] = { 0,0,0,0,
						255,255,255,255,
						255,0,0,255,
						255,0,255,0 };
	float ar2[4][4] = { 0,0,255,255,
						0,0,255,255,
						0,0,255,0,
						255,255,0,255 };
	float ar3[4][4] = { 255,0,0,255,
						255,0,0,255,
						0,255,0,0,
						0,255,255,0 };
	float ar4[4][4] = { 0,255,0,255,
						0,255,0,0,
						255,0,0,255,
						255,255,255,0 };
	float ar5[4][4] = { 0,255,255,255,
						255,0,0,255,
						255,255,0,0,
						255,255,0,255 };
	float ar6[4][4] = { 255,0,0,255,
						255,255,255,0,
						0,0,255,0,
						255,255,255,0 };
	float ar7[4][4] = { 255,255,0,0,
						0,255,0,0,
						255,255,255,255,
						0,0,255,0 };
	float ar8[4][4] = { 255,255,255,255,
						255,255,255,0,
						255,255,0,255,
						255,0,255,0 };
	float ar9[4][4] = { 255,255,0,0,
						255,255,255,255,
						0,255,0,255,
						0,255,255,0 };
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//******************************************************SETUP****************************************************************//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	aruco_database.push_back(Mat(4, 4, CV_32F, ar0));
	aruco_database.push_back(Mat(4, 4, CV_32F, ar1));
	aruco_database.push_back(Mat(4, 4, CV_32F, ar2));
	aruco_database.push_back(Mat(4, 4, CV_32F, ar3));
	aruco_database.push_back(Mat(4, 4, CV_32F, ar4));
	aruco_database.push_back(Mat(4, 4, CV_32F, ar5));
	aruco_database.push_back(Mat(4, 4, CV_32F, ar6));
	aruco_database.push_back(Mat(4, 4, CV_32F, ar7));
	aruco_database.push_back(Mat(4, 4, CV_32F, ar8));
	aruco_database.push_back(Mat(4, 4, CV_32F, ar9));
	//Puntos utiliados en findHomography
	imagePointsH.push_back(Point3d(50, 50, 0));
	imagePointsH.push_back(Point3d(50, 150, 0));
	imagePointsH.push_back(Point3d(150, 150, 0));
	imagePointsH.push_back(Point3d(150, 50, 0));
	//Puntos del objeto en solvePnP
	objectPoints.push_back(Point3d(5, 5, 0));
	objectPoints.push_back(Point3d(5, -5, 0));
	objectPoints.push_back(Point3d(-5, -5, 0));
	objectPoints.push_back(Point3d(-5, 5, 0));
	//Punto de localización de la id del aruco para solvePnP
	pointID.push_back(Point3d(0, 0, 0));
	Mat cameraMatrix(3, 3, CV_32F, cam);
	Mat distorsion_vector = Mat(1, 5, CV_32F, dist);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//************************************************INICIO DEL PROGRAMA********************************************************//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	while (c != 27) { //Si la tecla es ESCAPE, finaliza el programa
		int64 start = getTickCount();//Conteo inicial del reloj para fps
		ret = capture.read(frame);	 //Lectura del video
		if (ret == true) {
			//Preprocesado
			cvtColor(frame, frameg, CV_BGR2GRAY);//Cambio a blanco y negro
			threshold(frameg, thresimg, 150, 255, CV_THRESH_OTSU);//Binarización de la imagen
			GaussianBlur(thresimg, thresimg, Size(3, 3), 0);//Suavizado de imagen
			Canny(thresimg, cannyimg, 400, 500);//Detección de bordes Canny
			//Detección de contornos de ArUco
			findContours(cannyimg, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));//Detección de todos los contornos de la imagen
			Mat im_out = Mat::zeros(Size(10, 10), CV_8UC3); //Imagen de compribación del database
			approx.resize(contours.size());				    //Asignación de tamaño de los contornos cerrados
			for (int i = 0; i < approx.size(); i++)
			{
				//Inicialización o limpieza de variables usadas
				bool correct = false;
				int n = 0;
				int k = 0;
				int id = 0;
				Mat diff;
				imagePoints.clear();
				approxPolyDP(Mat(contours[i]), approx[i], 20, true); //Expresión para unir contornos cercanos
				//Clasificación de contornos
				if (approx[i].size() == 4 && contourArea(approx[i]) > 5000 && contourArea(approx[i]) < 100000) {

					copy(approx[i].begin(), approx[i].end(), back_inserter(imagePoints));//Copia de puntos de las esquinas del ArUco
					Mat H = findHomography(imagePoints, imagePointsH, RANSAC); //Función de devuelve la matriz de proyección de la imagen al mundo real
					warpPerspective(thresimg, im_out, H, frame.size());		   //Prouección de la imagen usando la matriz H
					getRectSubPix(im_out, Size(65, 65), Point2f(100, 100), im_out); //Recorte de la imagen a un cuadrado con tamaño deseado en un punto inicial donde se sabe que se encuentra el ArUco		
					resize(im_out, im_out, Size(), 0.06, 0.06, INTER_LANCZOS4);	    //Reescala a 4x4 pixeles con facot 0.06 en X e Y
					im_out.convertTo(im_out, CV_32F);//Conversión de uchar a float

					//Comparación con la database, sale cuando existe match
					for (int j = 0; j < aruco_database.size() && n != 16; j++) {
						k = 0;//Inicialización de las rotaciones en caso de no encajar con un ArUco
						for (int i = 0; i < 4 && n != 16; i++)
						{
							compare(im_out, aruco_database[j], diff, CMP_EQ); //Comparación del aruco (0= match ,1= no match)
							n = countNonZero(diff);							  //Cuenta cuantos 0 hay en la comparación
							if (n != 16) {//Existe al menos un pixel que no es igual a la database
								rotate(im_out, im_out, ROTATE_90_CLOCKWISE);//Rota la imagen
								k++;
							}
							else correct = true;//Comprueba que existe match
						}
						id = j;//Guarda el ArUco que encaja con la imagen
					}
					//Proyección de puntos en caso de haber match
					if (correct == true) {
						rotate(imagePoints.begin(), imagePoints.begin() + k, imagePoints.end()); //Seguimiento de la orientación del ArUco
						solvePnP(objectPoints, imagePoints, cameraMatrix, distorsion_vector, rvec_front, tvec_front, false, CV_ITERATIVE);//Devuelve los vectores de rotación(rvec) y traslación(tvec) de la proyección
						projectPoints(pointID, rvec_front, tvec_front, cameraMatrix, distorsion_vector, projectedPoints); //Proyección de la id en el centro del ArUco
						putText(frame, "ID=" + to_string(id), projectedPoints[0], CV_FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 1); //Texto de la id en el punto proyectado
						imagePoints.clear();//Limpieza de puntos para proxima iteración
						char a = waitKey(1);//Comprueba la tecla
						//Al pulsar la tecla a, aumenta de valor fig y esta se utilizará en la función figures
						if (a == 97) fig++;
						if (fig > 2) fig = 0;
						figures(frame, objectPoints2, rvec_front, tvec_front, cameraMatrix, distorsion_vector, projectedPoints, fig);
					}
				}
			}
			int fps = getTickFrequency() / (getTickCount() - start); //Expreción para mostrar los fps

			putText(frame, "FPS:" + to_string(fps), Point(20, 20), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2); //Muestra los fps en la esquina superior izquierda
			imshow("OUTPUT IMAGE", frame); //Muestra la imagen de salida
			output.write(frame);		   //Añade el frame al video de salida
		}
		c = waitKey(0); //Espera a pulsar una tecla
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//****************************************FIN DE PROGRAMA Y LIMPIEZA DE MEMORIA**********************************************//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	frame.release();
	frameg.release();
	cannyimg.release();
	thresimg.release();
	capture.release();
	output.release();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//*********************************************FUNCIÓN DE PROYECCIÓN DE FIGURAS**********************************************//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void figures(Mat& frame, vector<Point3d>objectPoints2, Mat rvec_front, Mat tvec_front, Mat cameraMatrix, Mat distorsion_vector, vector<Point2d> projectedPoints, int figure) {

	if (figure == 0) {//Proyección de cubo
		//Puntos del cubo en el mundo real con respecto al centro del Aruco( en cm)
		objectPoints2.push_back(Point3d(5, 5, 0));
		objectPoints2.push_back(Point3d(5, -5, 0));
		objectPoints2.push_back(Point3d(-5, -5, 0));
		objectPoints2.push_back(Point3d(-5, 5, 0));
		objectPoints2.push_back(Point3d(5, 5, -10));
		objectPoints2.push_back(Point3d(5, -5, -10));
		objectPoints2.push_back(Point3d(-5, -5, -10));
		objectPoints2.push_back(Point3d(-5, 5, -10));

		//Proyección de puntos
		projectPoints(objectPoints2, rvec_front, tvec_front, cameraMatrix, distorsion_vector, projectedPoints);

		//Unión de lineas en un orden concreto
		line(frame, projectedPoints[0], projectedPoints[4], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[1], projectedPoints[5], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[2], projectedPoints[6], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[3], projectedPoints[7], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[4], projectedPoints[5], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[5], projectedPoints[6], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[6], projectedPoints[7], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[7], projectedPoints[4], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[0], projectedPoints[1], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[1], projectedPoints[2], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[2], projectedPoints[3], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[3], projectedPoints[0], Scalar(0, 255, 0), 2);
	}
	else if (figure == 1) {//Proyección de priámide con mismo procedimiento
		objectPoints2.push_back(Point3d(5, 5, 0));
		objectPoints2.push_back(Point3d(5, -5, 0));
		objectPoints2.push_back(Point3d(5, -5, -10));
		objectPoints2.push_back(Point3d(5, 5, -10));
		objectPoints2.push_back(Point3d(-5, 0, -5));

		projectPoints(objectPoints2, rvec_front, tvec_front, cameraMatrix, distorsion_vector, projectedPoints);

		line(frame, projectedPoints[0], projectedPoints[1], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[1], projectedPoints[2], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[2], projectedPoints[3], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[3], projectedPoints[0], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[0], projectedPoints[4], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[1], projectedPoints[4], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[2], projectedPoints[4], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[3], projectedPoints[4], Scalar(0, 255, 0), 2);
	}
	else if (figure == 2) {//Proyección de diamante con mismo procedimiento
		objectPoints2.push_back(Point3d(5, 5, -5));
		objectPoints2.push_back(Point3d(5, -5, -5));
		objectPoints2.push_back(Point3d(-5, -5, -5));
		objectPoints2.push_back(Point3d(-5, 5, -5));
		objectPoints2.push_back(Point3d(0, 0, -10));
		objectPoints2.push_back(Point3d(0, 0, 0));

		projectPoints(objectPoints2, rvec_front, tvec_front, cameraMatrix, distorsion_vector, projectedPoints);

		line(frame, projectedPoints[0], projectedPoints[1], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[1], projectedPoints[2], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[2], projectedPoints[3], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[3], projectedPoints[0], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[0], projectedPoints[4], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[1], projectedPoints[4], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[2], projectedPoints[4], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[3], projectedPoints[4], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[0], projectedPoints[5], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[1], projectedPoints[5], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[2], projectedPoints[5], Scalar(0, 255, 0), 2);
		line(frame, projectedPoints[3], projectedPoints[5], Scalar(0, 255, 0), 2);
	}
	objectPoints2.clear(); //Limpieza para próxima iteración
}
