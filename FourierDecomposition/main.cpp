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
	FourierApp(int num_vecs, double rotation_per_sec, double integral_delta_t)
	{
		this->num_vecs = num_vecs;
		this->rotation_per_sec = rotation_per_sec;
		this->integral_delta_t = integral_delta_t;
		sAppName = "Fourier Deconstructor";
	}
	int hover = -1;
	int selected = -1;
	int num_vecs;
	double rotation_per_sec;
	double integral_delta_t;
	vector<point> vertices_screen;
	vector<cpx> vertices_world;
	vector<point> drawn_points;
	map<int, cpx> c_vals;
	double curr_time = 0;
	point offset = { 0,0 };
	double time_last_updated = 0;
	bool debug = false;

	cpx ScreenToWorld(point screenPos) {
		return { (double)screenPos.x / ScreenWidth() - 0.5, (double)screenPos.y / ScreenHeight() - 0.5 };
	}
	point WorldToScreen(cpx worldPos) {
		return { (int)((worldPos.real() + 0.5) * ScreenWidth()), (int)((worldPos.imag() + 0.5) * ScreenHeight()) };
	}
	//Euler, thanks my man
	cpx Cis(double theta) {
		return { cos(theta), sin(theta) };
	}
	cpx Lerp(cpx a, cpx b, double partialTime) {
		//Why is this so simple
		return a * (1 - partialTime) + b * partialTime;
	}

	//Epic function defined for [0;1[
	cpx f(double t) {
		int i = (int)(vertices_world.size() * t);
		double partialTime = t * vertices_world.size() - i;
		return Lerp(vertices_world[i], vertices_world[(i + 1) % vertices_world.size()], partialTime);
	}


	//Maths magic 1
	cpx Cn(int n) {
		cpx sum = { 0,0 };
		for (double t = 0; t < 1; t += integral_delta_t) sum += f(t) * Cis(-n * 2 * M_PI * t) * integral_delta_t;
		return sum;
	}

	//Maths magic 2
	cpx VecN(int n, double t) {
		return c_vals[n] * Cis(n * 2 * M_PI * t);
	}

	void UpdateCVals() {
		c_vals.clear();
		for (int i = -(num_vecs - 1) / 2; i <= num_vecs / 2; i++) c_vals.insert({ i, Cn(i) });
		drawn_points.clear();
		time_last_updated = curr_time;
	}


public:
	bool OnUserCreate() override
	{
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::Pixel(0, 0, 31));
		auto mouse = GetMousePos();
		//Handle keys
		if (GetKey(olc::D).bPressed)
		{
			debug = !debug;
		}
		//Move selected vertex
		if (selected != -1)
		{
			vertices_screen[selected] = mouse - offset;
			vertices_world[selected] = ScreenToWorld(mouse - offset);
			if (curr_time - time_last_updated > .00000005 * num_vecs / integral_delta_t)
			{
				UpdateCVals();
			}
		}

		//Handle hover/selection
		if (selected == -1)
		{
			//Find vertex cursor is above
			hover = -1;
			for (int i = 0; i < vertices_screen.size(); i++) if ((mouse - vertices_screen[i]).mag2() <= 120) { hover = i; break; }
		}
		if (GetMouse(0).bPressed)
		{
			if (hover != -1)
			{
				selected = hover;
				offset = mouse - vertices_screen[hover];
			}
			if (selected == -1)
			{
				vertices_screen.push_back(mouse);
				vertices_world.push_back(ScreenToWorld(mouse));
				hover = vertices_screen.size() - 1;
				selected = hover;
				offset = { 0,0 };
				UpdateCVals();
			}
		}
		if (GetMouse(0).bReleased && selected != -1)
		{
			selected = -1;
			UpdateCVals();
		}
		//Render stuff
		DrawString({ 5,975 }, "Press d to toggle debug mode", olc::WHITE, 2);
		if (debug)
		{
			DrawString({ 5,5 }, "curr_time: " + to_string(curr_time), olc::WHITE, 2);
			DrawString({ 5,25 }, "time_last_updated: " + to_string(time_last_updated), olc::WHITE, 2);
			DrawString({ 5,45 }, "hover: " + to_string(hover), olc::WHITE, 2);
			DrawString({ 5,65 }, "selected: " + to_string(selected), olc::WHITE, 2);
			DrawString({ 5,85 }, "num_vecs: " + to_string(num_vecs), olc::WHITE, 2);
			DrawString({ 5,105 }, "rotation_per_sec: " + to_string(rotation_per_sec), olc::WHITE, 2);
			DrawString({ 5,125 }, "integral_delta_t: " + to_string(integral_delta_t), olc::WHITE, 2);
			DrawString({ 5,145 }, "drawn_points.size(): " + to_string(drawn_points.size()), olc::WHITE, 2);
			string a = "";
			for (auto& pair : c_vals) {
				a += to_string(pair.first) + ": " + to_string(pair.second.real()) + " + " + to_string(pair.second.imag()) + "i\n";
			}
			DrawString({ 5,165 }, "c_vals: " + a, olc::WHITE, 2);
		}
		for (int i = 0; i < vertices_screen.size(); i++) DrawLine(vertices_screen[i], vertices_screen[(i + 1) % vertices_screen.size()], olc::Pixel(0, 255, 0, 127));
		for (int i = 0; i < vertices_screen.size(); i++) FillCircle(vertices_screen[i], 10, i == hover ? olc::Pixel(150, 255, 0) : olc::GREEN);
		cpx sum = { 0,0 };
		int a = 0;
		for (int i = 0, a = 0; i < num_vecs; i++)
		{
			cpx pre_sum = sum;
			sum += VecN(a, curr_time);
			DrawLine(WorldToScreen(pre_sum), WorldToScreen(sum), olc::MAGENTA);
			a = a <= 0 ? -a + 1 : -a;
		}
		point last_pos = WorldToScreen(sum);
		int dp_size = drawn_points.size();
		if (curr_time - time_last_updated < 1)
		{
			drawn_points.push_back(last_pos);
		}
		else
		{
			DrawLine(drawn_points[dp_size - 1], drawn_points[0], olc::Pixel(255, 255, 0));
		}
		for (int i = 0; i < dp_size - 1; i++)
		{
			DrawLine(drawn_points[i], drawn_points[(i + 1) % dp_size], olc::Pixel(255, 255, 0));
			//cout << i << " " << drawn_points.size() << endl;
		}
		FillCircle(last_pos, 3, olc::Pixel(255, 0, 0));
		curr_time += fElapsedTime * rotation_per_sec;
		return true;
	}
};

int main()
{
	int num_vecs = 21;
	double rotation_per_sec = 0.1;
	double integral_delta_t = 0.001;
	string str;
	cout << "Press <c> to open config, press any other key to start!" << endl;
	if ('c' == _getch())
	{
		cout << "num_vecs (n>0)(preferably odd)(default n=21): ";
		cin >> num_vecs;
		if (num_vecs <= 0) num_vecs = 21;
		cout << num_vecs << endl;


		cout << "rotation_per_sec (default n=0.1): ";
		cin >> rotation_per_sec;
		cout << rotation_per_sec << endl;

		cout << "integral_delta_t (1>=n>0)(default n=0.001): ";
		cin >> integral_delta_t;
		if (integral_delta_t <= 0 || integral_delta_t > 1) integral_delta_t = 0.001;
		cout << integral_delta_t << endl;
	}

	FourierApp demo = FourierApp(num_vecs, rotation_per_sec, integral_delta_t);
	if (demo.Construct(1000, 1000, 1, 1))
		demo.Start();

	return 0;
}