#define OLC_PGE_APPLICATION
#define _USE_MATH_DEFINES
#include "olcPixelGameEngine.h"
#include <complex>
#include <vector>
#include <cmath>
#include <conio.h>
#define cpx complex<double>
#define point olc::vi2d

using namespace std;

class FourierApp : public olc::PixelGameEngine
{
public:
	FourierApp(double rotation_per_sec)
	{
		this->rotation_per_sec = rotation_per_sec;
		sAppName = "Fourier Deconstructor";
	}
	int hover = -1;
	int selected = -1;
	double rotation_per_sec;
	vector<point> screen_vertices;
	vector<cpx> complex_vertices;
	double curr_time = 0;
	point offset = { 0,0 };
	double time_last_updated = 0;
	bool debug = false;
	vector<cpx> idft;

	cpx ScreenToWorld(point screenPos) {
		return { (double)screenPos.x / ScreenWidth() - 0.5, (double)screenPos.y / ScreenHeight() - 0.5 };
	}
	point WorldToScreen(cpx worldPos) {
		return { (int)((worldPos.real() + 0.5) * ScreenWidth()), (int)((worldPos.imag() + 0.5) * ScreenHeight()) };
	}
	cpx Lerp(cpx a, cpx b, double partialTime) {
		//Why is this so simple
		return a * (1 - partialTime) + b * partialTime;
	}
	cpx Cis(double theta) {
		return { cos(theta), sin(theta) };
	}
	int wrapIndex(int i) {
		int n = complex_vertices.size();
		if (n % 2 == 0) {
			return i >= n / 2 ? i - n : i;
		}
		else {
			return i >= (n - 1) / 2 ? i - n : i;
		}
	}

	//Epic function defined for [0;1[
	cpx f(double t) {
		int i = (int)(complex_vertices.size() * t);
		double partialTime = t * complex_vertices.size() - i;
		return Lerp(complex_vertices[i], complex_vertices[(i + 1) % complex_vertices.size()], partialTime);
	}




	void UpdateIDFT() {
		int n = complex_vertices.size();
		double invs = 1.0 / n;
		idft.resize(n);
		for (int i = 0; i < n; i++) {
			idft[i] = 0;
			for (int j = 0; j < n; j++) {
				idft[i] += f(j * invs) * Cis(-2 * M_PI * wrapIndex(i) * j * invs);
			}
			idft[i] *= invs;
		}
		time_last_updated = curr_time;
	}
	cpx iVec(int i, double t) {
		double invs = 1.0 / (complex_vertices.size());
		return idft[i] * Cis(2 * M_PI * wrapIndex(i) * t * invs);

	}

public:
	bool OnUserCreate() override { return true; }
	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear({ 0, 0, 31 });
		auto& mouse = GetMousePos();
		//Handle keys
		if (GetKey(olc::D).bPressed) debug = !debug;

		//Move selected vertex
		if (selected != -1)
		{
			screen_vertices[selected] = mouse - offset;
			complex_vertices[selected] = ScreenToWorld(mouse - offset);
			if (curr_time - time_last_updated > .005 * rotation_per_sec) UpdateIDFT();

		}

		//Handle hover/selection
		if (selected == -1)
		{
			//Find vertex cursor is above
			hover = -1;
			for (int i = 0; i < screen_vertices.size(); i++)
			{
				if ((mouse - screen_vertices[i]).mag2() <= 120) {
					hover = i;
					break;
				}
			}
		}
		if (GetMouse(0).bPressed)
		{
			if (hover != -1)
			{
				selected = hover;
				offset = mouse - screen_vertices[hover];
			}
			if (selected == -1)
			{
				screen_vertices.push_back(mouse);
				complex_vertices.push_back(ScreenToWorld(mouse));
				hover = screen_vertices.size() - 1;
				selected = hover;
				offset = { 0,0 };
				UpdateIDFT();
			}
		}
		if (GetMouse(0).bReleased && selected != -1)
		{
			selected = -1;
			UpdateIDFT();
		}
		//Render stuff
		DrawString({ 5,975 }, "Press d to toggle debug mode", olc::WHITE, 2);
		if (debug)
		{
			DrawString({ 5,5 }, "curr_time: " + to_string(curr_time), olc::WHITE, 2);
			DrawString({ 5,25 }, "time_last_updated: " + to_string(time_last_updated), olc::WHITE, 2);
			DrawString({ 5,45 }, "hover: " + to_string(hover), olc::WHITE, 2);
			DrawString({ 5,65 }, "selected: " + to_string(selected), olc::WHITE, 2);
			DrawString({ 5,85 }, "rotation_per_sec: " + to_string(rotation_per_sec), olc::WHITE, 2);

		}
		for (int i = 0; i < screen_vertices.size(); i++) DrawLine(screen_vertices[i], screen_vertices[(i + 1) % screen_vertices.size()], olc::Pixel(0, 255, 0, 127));
		for (int i = 0; i < screen_vertices.size(); i++) FillCircle(screen_vertices[i], 10, i == hover ? olc::Pixel(150, 255, 0) : olc::GREEN);
		cpx sum = { 0,0 };
		int a = 0;

		for (int i = 0; i < idft.size(); i++)
		{
			cpx pre_sum = sum;
			sum += iVec(i, curr_time);
			DrawLine(WorldToScreen(pre_sum), WorldToScreen(sum), olc::MAGENTA);
		}

		curr_time += fElapsedTime * rotation_per_sec;
		return true;
	}
};

int main()
{
	double rotation_per_sec = 1;
	cout << "Press <c> to open config, press any other key to start!" << endl;
	if (_getch() == 'c')
	{
		cout << "rotation_per_sec (default n=0.1): ";
		cin >> rotation_per_sec;
	}

	FourierApp demo = FourierApp(rotation_per_sec);
	if (demo.Construct(1000, 1000, 1, 1))
		demo.Start();

	return 0;
}