#define OLC_PGE_APPLICATION
#define _USE_MATH_DEFINES
#include "olcPixelGameEngine.h"
#include <vector>
#include <cmath>
#include <conio.h>

class FourierApp : public olc::PixelGameEngine
{
public:
	FourierApp(int num_vecs, float rotation_per_sec, float integral_delta_t)
	{
		this->num_vecs = num_vecs;
		this->rotation_per_sec = rotation_per_sec;
		this->integral_delta_t = integral_delta_t;
		sAppName = "Fourier Deconstructor";
	}
	int hover = -1;
	int selected = -1;
	int num_vecs;
	float rotation_per_sec;
	float integral_delta_t;
	std::vector<olc::vi2d> vertices_screen;
	std::vector<olc::vf2d> vertices_world;
	std::map<int, olc::vf2d> c_vals;
	float time = 0;
	olc::vi2d offset = { 0,0 };
	float time_last_updated = 0;

	olc::vf2d ScreenToWorld(olc::vi2d screenPos) {
		return { (float)screenPos.x / ScreenWidth() - 0.5f, (float)screenPos.y / ScreenHeight() - 0.5f };
	}
	olc::vi2d WorldToScreen(olc::vf2d worldPos) {
		return { (int)((worldPos.x + 0.5f) * ScreenWidth()), (int)((worldPos.y + 0.5f) * ScreenHeight()) };
	}
	//(a+b)^2= a^2 + 2ab + b^2
	olc::vf2d CMultiply(olc::vf2d a, olc::vf2d b) {
		return { a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x };
	}
	//Euler, thanks my man
	olc::vf2d Cis(float theta) {
		return { std::cos(theta), std::sin(theta) };
	}
	olc::vf2d Lerp(olc::vf2d a, olc::vf2d b, float partialTime) {
		//Why is this so simple
		return a * (1 - partialTime) + b * partialTime;
	}
	//Epic function defined for [0;1[
	olc::vf2d f(float t) {
		int i = (int)(vertices_world.size() * t);
		float partialTime = t * vertices_world.size() - i;
		return Lerp(vertices_world[i], vertices_world[(i + 1) % vertices_world.size()], partialTime);
	}
	//Maths magic 1
	olc::vf2d Cn(int n) {
		olc::vf2d sum = { 0,0 };
		for (float t = 0; t < 1; t += integral_delta_t) sum += CMultiply(f(t), Cis(-n * 2 * M_PI * t)) * integral_delta_t;
		return sum;
	}
	//Maths magic 2
	olc::vf2d VecN(int n, float t) {
		return CMultiply(c_vals[n], Cis(n * 2 * M_PI * t));
	}

	void UpdateCVals() {
		c_vals.clear();
		for (int i = -(num_vecs - 1) / 2; i <= num_vecs / 2; i++) c_vals.insert(std::pair<int, olc::vf2d>(i, Cn(i)));
	}


public:
	bool OnUserCreate() override
	{
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::VERY_DARK_BLUE);
		auto mouse = GetMousePos();

		//Move selected vertex
		if (selected != -1)
		{
			vertices_screen[selected] = mouse - offset;
			vertices_world[selected] = ScreenToWorld(mouse - offset);
			if (time - time_last_updated > 0.001f)
			{
				UpdateCVals();
				time_last_updated = time;
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
		DrawStringDecal({ 5,5 }, "hover: " + std::to_string(hover), olc::WHITE, { {2.0f},{2.0f} });
		DrawStringDecal({ 5,25 }, "selected: " + std::to_string(selected), olc::WHITE, { {2.0f},{2.0f} });
		DrawStringDecal({ 5,45 }, "num_vecs: " + std::to_string(num_vecs), olc::WHITE, { {2.0f},{2.0f} });
		DrawStringDecal({ 5,65 }, "rotation_per_sec: " + std::to_string(rotation_per_sec), olc::WHITE, { {2.0f},{2.0f} });
		DrawStringDecal({ 5,85 }, "integral_delta_t: " + std::to_string(integral_delta_t), olc::WHITE, { {2.0f},{2.0f} });
		for (int i = 0; i < vertices_screen.size(); i++) DrawLine(vertices_screen[i], vertices_screen[(i + 1) % vertices_screen.size()]);
		for (int i = 0; i < vertices_screen.size(); i++) FillCircle(vertices_screen[i], 10, i == hover ? olc::Pixel(100, 255, 0) : olc::GREEN);
		olc::vf2d sum = { 0,0 };
		int a = 0;
		for (int i = 0; i < num_vecs; i++)
		{
			olc::vf2d pre_sum = sum;
			sum += VecN(a, time);
			DrawLine(WorldToScreen(pre_sum), WorldToScreen(sum), olc::YELLOW);
			a = a <= 0 ? -a + 1 : -a;
		}
		time += fElapsedTime * rotation_per_sec;

		return true;
	}
};

int main()
{
	int num_vecs = 21;
	float rotation_per_sec = 0.1;
	float integral_delta_t = 0.001;
	std::string str;
	std::cout << "Press <c> to open config, press any other key to start!" << std::endl;
	if ('c' == (char)_getch())
	{
		std::cout << "num_vecs (n>0)(preferably odd)(default n=21): ";
		std::getline(std::cin, str);
		try
		{
			num_vecs = std::stoi(str);
		}
		catch (const std::exception&) {}
		if (num_vecs <= 0) num_vecs = 21;
		std::cout << num_vecs << std::endl;


		std::cout << "rotation_per_sec (default n=0.1): ";
		std::getline(std::cin, str);
		try
		{
			rotation_per_sec = std::stof(str);
		}
		catch (const std::exception&) {}
		std::cout << rotation_per_sec << std::endl;

		std::cout << "integral_delta_t (1>=n>0)(default n=0.001): ";
		std::getline(std::cin, str);
		try
		{
			integral_delta_t = std::stof(str);
		}
		catch (const std::exception&) {}
		if (integral_delta_t <= 0 || integral_delta_t > 1) integral_delta_t = 0.001;
		std::cout << integral_delta_t << std::endl;
	}
	//When the easy part is almost as long as the crazy part


	FourierApp demo = FourierApp(num_vecs,rotation_per_sec,integral_delta_t);
	if (demo.Construct(1000, 1000, 1, 1))
		demo.Start();

	return 0;
}