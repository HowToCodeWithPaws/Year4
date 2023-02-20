#pragma once
#include <string>
#include <vector>
#include <float.h>

namespace GSA {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Plot
	/// </summary>
	public ref class Plot : public System::Windows::Forms::Form
	{
	public:
		Plot(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

		void setSeries(int code, std::vector<double> points) {
			this->Text = code ? "Convergence plot" : "Best values plot";
			this->series1->Points->Clear();
			System::Windows::Forms::DataVisualization::Charting::Title^ title1 = (gcnew System::Windows::Forms::DataVisualization::Charting::Title());
			title1->Name = L"Title1";
			title1->Text = code ? L"Convergence plot" : L"Best values plot";
			this->chart1->Titles->Add(title1);
			chartArea1->AxisY->Title = code ? L"difference between iterations" : L"best function value in iteration";
			for (int i = 0; i < points.size(); i++)
			{
				double point = points[i];
				System::Windows::Forms::DataVisualization::Charting::DataPoint^ dataPoint = (gcnew System::Windows::Forms::DataVisualization::Charting::DataPoint(i, round(point)));
				this->series1->Points->Add(dataPoint);
			}
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Plot()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::DataVisualization::Charting::Chart^ chart1;
		   System::Windows::Forms::DataVisualization::Charting::Series^ series1;
		   System::Windows::Forms::DataVisualization::Charting::ChartArea^ chartArea1;

	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->chartArea1 = (gcnew System::Windows::Forms::DataVisualization::Charting::ChartArea());
			this-> series1 = (gcnew System::Windows::Forms::DataVisualization::Charting::Series());
			this->chart1 = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chart1))->BeginInit();
			this->SuspendLayout();
			// 
			// chart1
			// 
			this->chartArea1->Name = L"ChartArea1";
			this->chartArea1->AxisX->Title = L"iterations";
			this->chart1->ChartAreas->Add(chartArea1);
			this->chart1->Location = System::Drawing::Point(12, 12);
			this->chart1->Name = L"chart1";
			series1->ChartArea = L"ChartArea1";
			series1->ChartType = System::Windows::Forms::DataVisualization::Charting::SeriesChartType::Line;
			series1->Legend = L"Legend1";
			series1->Name = L"Series1";
			series1->Color = Color::Red;
			this->chart1->Series->Add(series1);
			this->chart1->Size = System::Drawing::Size(697, 405);
			this->chart1->TabIndex = 0;
			this->chart1->Text = L"chart1";
			// 
			// Plot
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(9, 20);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(721, 429);
			this->Controls->Add(this->chart1);
			this->Name = L"Plot";
			this->Text = L"Plot";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chart1))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion

	};
}
