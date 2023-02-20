#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <float.h>
#include <fstream>
#include <Windows.h>


namespace Algorithm {
	class Func
	{
	public:
		virtual std::string name() {
			return  "y = x";
		}

		virtual double fit(std::vector<double> position) {
			double sum = 0;
			for (int i = 0; i < position.size(); i++)
			{
				sum += position[i];
			}
			return sum;
		}

		virtual void setCoeffs(std::vector<std::vector<double>> abcde, int num_special) {}
		virtual void setMinMax(std::vector<std::vector<double>> minmax) {}

		std::vector<double> chosen_values;

		int dimensions = 1;
		std::vector<double> dynamic_dimensions = std::vector<double>(2, -1);
		int num_special_buses = 0;

		std::vector<double> min_vector = std::vector<double>(dimensions, -100);
		std::vector<double> max_vector = std::vector<double>(dimensions, 100);

		std::vector<double> costs_a = std::vector<double>(dimensions, -100);
		std::vector<double> costs_b = std::vector<double>(dimensions, -100);
		std::vector<double> costs_c = std::vector<double>(dimensions, -100);
		std::vector<double> costs_d = std::vector<double>(num_special_buses, -100);
		std::vector<double> costs_e = std::vector<double>(num_special_buses, -100);
	};

	class SquareFunc : public Func {
	public:
		std::string name() {
			return "y = x**2";
		}

		double fit(std::vector<double> position) {
			double sum = 0;
			for (int i = 0; i < position.size(); i++)
			{
				sum += position[i] * position[i];
			}
			return sum;
		}

		int dimensions = 1;

		std::vector<double> min_vector = std::vector<double>(dimensions, -100);
		std::vector<double> max_vector = std::vector<double>(dimensions, 100);
	};

	/// <summary>
	/// A function of costs for power production: quadratic cost curve with valve point loadings
	/// calculated with costs and a special curve on two first buses and costs on the rest.
	/// </summary>
	class PowerFlowFunc :public Func {
	private:
		double costs_with_coefficients(double power, int index) {
			return costs_a[index] + costs_b[index] * power + costs_c[index] * power * power;
		}

		double costs_curve(double power, int index) {
			return std::fabs(costs_d[index] * std::sin(costs_e[index] * (min_vector[index] - power)));
		}

	public:
		std::string name() {
			return "optimal power flow";
		}

		void setCoeffs(std::vector<std::vector<double>> abcde, int num_special) {
			num_special_buses = num_special;
			costs_a = abcde[0];
			costs_b = abcde[1];
			costs_c = abcde[2];
			costs_d = abcde[3];
			costs_e = abcde[4];
		}

		void setMinMax(std::vector<std::vector<double>> minmax) {
			min_vector = minmax[0];
			max_vector = minmax[1];
		}

		double fit(std::vector<double> position) {
			double sum = 0;

			/// basic coefficient quadratic cost for all buses
			for (int i = 0; i < position.size(); i++)
			{
				sum += costs_with_coefficients(position[i], i);
			}

			/// curve for special buses
			for (int i = 0; i < num_special_buses; i++)
			{
				sum += costs_curve(position[i], i);
			}

			return sum;
		}

		std::vector<double> min_vector;
		std::vector<double> max_vector;

		int num_special_buses = 0;
		std::vector<double> costs_a;
		std::vector<double> costs_b;
		std::vector<double> costs_c;
		std::vector<double> costs_d;
		std::vector<double> costs_e;
	};

	class Agent {
	public:
		std::vector<double> position;
		double name = -1;
		double fitness = 0;
		double mass = 0;
		std::vector <double> acceleration;
		std::vector <double> velocity;
		std::vector <double> force;

		std::string positionToString() {
			std::string pos = "";
			for (int i = 0; i < position.size(); i++)
			{
				pos += std::to_string(position[i]);
				if (i < position.size() - 1)
				{
					pos += ", ";
				}
			}

			return pos;
		}

		std::string toString() {
			std::string pos = positionToString();
			return "solution: " + std::to_string(fitness) + " at " + pos;
		}
	};

	class Problem {
	private:
		std::string condition_statement;
		std::string optimum_statement;
	public:
		Func* func_pointer;
		int num_agents = 0;
		int condition = 0;
		int condition_iters = 1000;
		double condition_diff = 0;
		int optimum_type = 0;
		int dimensions = 1;

		std::string conditionStatement() {
			if (condition == 1)
			{
				condition_statement = std::to_string(condition_iters) + " iterations";
			}
			else {
				condition_statement = "difference of " + std::to_string(condition_diff) + " between iterations";
			}
			return condition_statement;
		}

		std::string optimumStatement() {
			if (optimum_type == 1)
			{
				optimum_statement = "minimum";
			}
			else if (optimum_type == 2) { optimum_statement = "maximum"; };
			return optimum_statement;
		}

		std::string toString() {
			std::string result = "We will be looking for a " + optimumStatement() + " in function " + func_pointer->name() + " with condition of " + conditionStatement() + " with " + std::to_string(num_agents) + " agents.\nWe'll start the algorithm now!\n";
			return result;
		}
	};

	class GSAClass {
	public:
		int iteration;
		bool proceed;
		Agent best_agent = Agent();
		int history_iteration = 0;
		std::vector<std::vector<std::vector<double>>> history = std::vector<std::vector<std::vector<double>>>();
		std::vector<std::vector<double>> bests = std::vector< std::vector<double>>();
		std::vector<double> convergence;
		std::vector<double> getDynamic() {
			return std::vector<double>{ current_problem->func_pointer->dynamic_dimensions[0], current_problem->func_pointer->dynamic_dimensions[1] };
		}
	private:
		Problem* current_problem;

		Agent worst_agent = Agent();
		std::vector<Agent> agents;
		std::vector<Agent> k_best_agents;

		double g_const;
		double g_initial;
		double g_deterioration;

		double current_diff;
		double prev_best;

		int k_best;

		void generatePopulation() {
			std::random_device rd;  /// Will be used to obtain a seed for the random number engine
			std::mt19937 gen(rd()); /// Standard mersenne_twister_engine seeded with rd()

			for (int n = 0; n < current_problem->num_agents; ++n) {
				Agent agent = Agent();
				agent.name = n + 1;
				agent.position = std::vector<double>(current_problem->dimensions, 0);
				agent.force = std::vector<double>(current_problem->dimensions, 0);
				agent.acceleration = std::vector<double>(current_problem->dimensions, 0);
				agent.velocity = std::vector<double>(current_problem->dimensions, 0);
				for (int d = 0; d < current_problem->dimensions; d++)
				{
					if (d == current_problem->func_pointer->dynamic_dimensions[0] || d == current_problem->func_pointer->dynamic_dimensions[1])
					{
						std::uniform_real_distribution<> dis_x(current_problem->func_pointer->min_vector[d], current_problem->func_pointer->max_vector[d]);
						double next = dis_x(gen);
						agent.position[d] = next;
					}
					else {
						agent.position[d] = current_problem->func_pointer->chosen_values[d];
					}
				}
				agents.push_back(agent);
			}

			updateFitness();

			//	for (int i = 0; i < current_problem->num_agents; ++i) {
		//			std::cout << agents[i].toString();
		//		}

			++iteration;
		}

		void updateBestAndWorst(int i) {

			if (better(current_problem->optimum_type, agents[i].fitness, best_agent.fitness))
			{
				best_agent = agents[i];
			}

			if (!better(current_problem->optimum_type, agents[i].fitness, worst_agent.fitness))
			{
				worst_agent = agents[i];
			}
		}

		void updateFitness() {
			best_agent.fitness = worst_agent.fitness = current_problem->func_pointer->fit(agents[0].position);
			best_agent.position = worst_agent.position = agents[0].position;

			for (int i = 0; i < current_problem->num_agents; i++)
			{
				/// calculate fitness
				agents[i].fitness = current_problem->func_pointer->fit(agents[i].position);

				updateBestAndWorst(i);
			}
		}

		void updateMass() {
			double sum_mass = 0;
			for (int i = 0; i < current_problem->num_agents; i++)
			{
				if (worst_agent.fitness == best_agent.fitness)
				{
					agents[i].mass = 1;
				}
				else {
					agents[i].mass = (agents[i].fitness - worst_agent.fitness) / (best_agent.fitness - worst_agent.fitness);
				}

				sum_mass += agents[i].mass;
			}

			for (int i = 0; i < current_problem->num_agents; i++)
			{
				agents[i].mass /= sum_mass;
			}
		}

		void updateKBest() {
			/// the percentage starts with almost all and goes down with time, also makes sure not to have all of them or none
			k_best = floor((1 - float(iteration) / current_problem->condition_iters) * current_problem->num_agents);

			if (k_best <= 1)
			{
				k_best = min(2, current_problem->num_agents);
			}
			else if (k_best >= current_problem->num_agents)
			{
				k_best = current_problem->num_agents - 2;
			}

			std::sort(agents.begin(), agents.end(),
				[](Agent const& a, Agent const& b) {
					return    a.fitness < b.fitness;
				});

			for (int i = 0; i < k_best; i++)
			{
				if (current_problem->optimum_type == 1)
				{
					k_best_agents.push_back(agents[i]);/// top k - best for minimum
				}
				else {
					k_best_agents.push_back(agents[current_problem->num_agents - 1 - i]); /// bottom k - best for max
				}
			}
		}

		void calculateForceOnAgent(int i, int j) {
			if (agents[i].position != k_best_agents[j].position)/// != but make it easier
			{
				double sum = 0;
				double R = 0;
				double dist = 0;
				for (int d = 0; d < current_problem->dimensions; d++)
				{
					dist = (agents[i].position[d] - k_best_agents[j].position[d]) * (agents[i].position[d] - k_best_agents[j].position[d]);
					sum += dist;
				}
				R = sqrt(sum);

				for (int d = 0; d < current_problem->dimensions; d++)
				{
					agents[i].force[d] += (((double)rand() / (double)RAND_MAX)) * (k_best_agents[j].mass) * (k_best_agents[j].position[d] - agents[i].position[d]) / (R + DBL_EPSILON);
				}
			}
		}

		void updateVelocities() {
			for (int i = 0; i < current_problem->num_agents; i++)
			{
				for (int d = 0; d < current_problem->dimensions; d++)
				{
					agents[i].force[d] = 0;
				}

				for (int j = 0; j < k_best; j++)
				{
					/// calculate force on object i from object j
					calculateForceOnAgent(i, j);
				}

				for (int d = 0; d < current_problem->dimensions; d++)
				{
					/// calculate acceleration and velocity of i in each dimension
					agents[i].acceleration[d] = agents[i].force[d] * g_const;
					agents[i].velocity[d] = agents[i].acceleration[d] + agents[i].velocity[d] * (((double)rand() / (double)RAND_MAX));
				}
			}
		}

		void updatePositions() {
			/// calculate position of i in each dimension
			for (int i = 0; i < current_problem->num_agents; i++)
			{
				for (int d = 0; d < current_problem->dimensions; d++)
				{
					agents[i].position[d] += agents[i].velocity[d];
					if (agents[i].position[d] > current_problem->func_pointer->max_vector[d])
					{
						agents[i].position[d] = current_problem->func_pointer->max_vector[d];
					}
					if (agents[i].position[d] < current_problem->func_pointer->min_vector[d])
					{
						agents[i].position[d] = current_problem->func_pointer->min_vector[d];
					}
				}
			}
		}

		bool better(int optimum_type, double value1, double value2) {
			bool res = ((optimum_type == 1) && (value1 <= value2)) || ((optimum_type == 2) && (value1 >= value2));
			return res;
		}

		//		void report() {
		//			std::cout << "\n\niteration " << iteration
		//				<< ", current difference " << current_diff << ", g constant " << g_const
		//				<< "\nbest: " << best_agent.fitness << " at " << best_agent.positionToString()
		//				<< "\nworst: " << worst_agent.fitness << " at " << worst_agent.positionToString();
		//			for (int i = 0; i < current_problem->num_agents; i++)
		//			{
		//				std::cout << agents[i].toString();
		//			}
		//		}


		struct less_than_key
		{
			inline bool operator() (const Agent& struct1, const Agent& struct2)
			{
				return (struct1.name < struct2.name);
			}
		};

		std::vector<std::vector<double>> report() {
			std::vector<std::vector<double>> all_positions;
			all_positions = std::vector<std::vector<double>>();
			int dim1 = current_problem->func_pointer->dynamic_dimensions[0];
			int dim2 = current_problem->func_pointer->dynamic_dimensions[1];

			std::vector<Agent> agents_copy = agents;
			std::sort(agents_copy.begin(), agents_copy.end(), less_than_key());

			for (int i = 0; i < current_problem->num_agents; i++)
			{
				double pos1 = agents_copy[i].position[dim1];
				double pos2 = agents_copy[i].position[dim2];
				std::vector<double> agent = std::vector<double>{ pos1, pos2 }; /// 0: dim1 1: dim2 2: mass 3: name, 4: fitness
				//	agent.push_back(agents[i].fitness);
				agent.push_back(agents_copy[i].mass);
				agent.push_back(agents_copy[i].name);
				double fit = std::round(agents_copy[i].fitness * 10.0) / 10.0;
				agent.push_back(fit);
				all_positions.push_back(agent);
			}
			double pos1 = best_agent.position[dim1];
			double pos2 = best_agent.position[dim2];
			std::vector<double> best = std::vector<double>{ pos1, pos2 };
			best.push_back(best_agent.mass);
			best.push_back(best_agent.fitness); /// !!! 3: fitness
			bests.push_back(best);
			history.push_back(all_positions);
			return all_positions;
		}

	public:
		std::vector<std::vector<double>> start(Problem* problem, std::vector<std::vector<double>> minmax, std::vector<double> g_params) {
			current_problem = problem;
			setMinMax(minmax);
			g_initial = g_params[0];
			g_deterioration = g_params[1];

			proceed = true;
			iteration = 0;
			current_diff = 1;

			/// init population
			generatePopulation();
			//			double res = solve();
			return report();
		}

		void setMinMax(std::vector<std::vector<double>> minmax) {
			current_problem->func_pointer->min_vector = minmax[0];
			current_problem->func_pointer->max_vector = minmax[1];
			current_problem->func_pointer->setMinMax(minmax);
		}

		std::vector<std::vector<double>> iter() {
			std::vector<std::vector<double>>	all_positions;
			/// the algorithm cycle
			if (proceed) {
				/// recalculate g const
				g_const = g_initial * exp(-g_deterioration * float(iteration) / current_problem->condition_iters);

				/// calculate mass
				updateMass();

				/// get best agents
				updateKBest();

				/// calculate force field
				updateVelocities();

				/// calculate new positions
				updatePositions();

				/// calculate agents' fitness based on new positions
				updateFitness();

				/// update difference and best solution in this iteration
				if (iteration > 0)
				{
					current_diff = std::fabs(prev_best - best_agent.fitness);
				}
				convergence.push_back(current_diff);
				prev_best = best_agent.fitness;

				/// current state of agents
				all_positions = report();

				/// check exit
				proceed = (current_problem->condition == 1 && iteration < current_problem->condition_iters)
					|| (current_problem->condition == 2 && current_diff > current_problem->condition_diff);

				iteration++;
			}

			return all_positions;
		}

		void writeToFile() {
			char filename_convergence[16] = { 'c','o','n','v','.','t', 'x','t' };
			char filename_bests[16] = { 'b','e','s','t','.','t', 'x','t' };
			char filename_hist[16] = { 'h','i','s','t','.','t', 'x','t' };
			std::fstream fp_convergence;
			std::fstream fp_bests;
			std::fstream fp_hist;

			fp_convergence.open(filename_convergence, std::fstream::out);
			fp_bests.open(filename_bests, std::fstream::out);
			fp_hist.open(filename_hist, std::fstream::out);
			if (!fp_convergence || !fp_bests || !fp_hist)
			{
				std::cout << "\nAn error occured while writing file!";
			}

			for (int i = 0; i < convergence.size(); i++)
			{
				fp_convergence << convergence[i];
				fp_convergence << "\n";
			}

			for (int i = 0; i < bests.size(); i++)
			{
				fp_bests << bests[i][3];
				fp_bests << "\n";
			}

			for (int i = 0; i < history.size(); i++)
			{
				fp_hist << "iteration " << i;
				fp_hist << "\n";
				for (int j = 0; j < this->current_problem->num_agents; j++)
				{ /// 0: dim1 1: dim2 2: mass 3: name, 4: fitness
					fp_hist << "agent_" << history[i][j][3] << " mass " << history[i][j][2] << " fitness " << history[i][j][4] << " at (" << history[i][j][0] << ", " << history[i][j][1] << ")";
					fp_hist << "\n";
				}
			}

			fp_convergence.close();
			fp_bests.close();
			fp_hist.close();
		}

	};

	//	int askCode(std::string message, std::vector<int> possible_values) {
	//		bool proceed = true;
	//		int code = 0;
	//		do {
	//			std::string code_s = "";
	//			std::cout << message;
	//			std::cin >> code_s;
	//			try
	//			{
	//				code = std::stoi(code_s);
	//			}
	//			catch (std::invalid_argument const& ex)
	//			{
	//				code = 0;
	//			}
	//
	//			for (int i = 0; i < possible_values.size(); i++)
	//			{
	//				proceed = proceed && code != possible_values[i];
	//			}
	//
	//		} while (proceed);
	//
	//		return code;
	//	}
	//
	//	double askValue(std::string message, std::vector<double> range_values, std::string type = "double") {
	//		bool proceed = true;
	//		double value = -1;
	//
	//		do {
	//			std::string value_s = "";
	//			std::cout << message;
	//			std::cin >> value_s;
	//			try
	//			{
	//				if (type == "int")
	//				{
	//					value = std::stoi(value_s);
	//				}
	//				else {
	//					value = std::stod(value_s);
	//				}
	//			}
	//			catch (std::invalid_argument const& ex)
	//			{
	//				value = -1;
	//			}
	//
	//			proceed = proceed && (value < range_values[0] || value > range_values[1]);
	//
	//		} while (proceed);
	//
	//		return value;
	//	}
	//
	//	//int main()
	//	//{
	//	//	Func func1 = Func();
	//	//	SquareFunc func2 = SquareFunc();
	//	//	PowerFlowFunc func3 = PowerFlowFunc();
	//	//	Problem problem = Problem();
	//	//	GSA gsa = GSA();
	//	//	Func* funcs[3] = { &func1, &func2 , &func3 };
	//	//	std::vector<std::vector<double>> abcde = std::vector<std::vector<double>>(5, std::vector<double>());
	//
	//	//	std::cout << "Hello, this is the GSA speaking!\n";
	//	//	problem.func_pointer = funcs[askCode("Please, choose a function:\n1: y = x\n2: y = x**2\n3: optimal power flow cost\n", std::vector<int>{1, 2, 3}) - 1];
	//	//	problem.optimum_type = askCode("Please, choose the type of optimum we are looking for:\n1: minimum\n2: maximum\n", std::vector<int>{1, 2});
	//	//	problem.dimensions = problem.func_pointer->dimensions = askValue("Please, choose the number of dimensions between 1 and 10:\n", std::vector<double>{1, 10}, "int");
	//
	//	//	double min_constraint = -DBL_MAX;
	//	//	if (problem.func_pointer->name() == "optimal power flow") {
	//	//		min_constraint = 0;
	//	//		problem.func_pointer->num_special_buses = askValue("Please, choose the number of buses with valve-point effects between 0 and " + std::to_string(problem.dimensions) + "\n", std::vector<double>{0, double(problem.dimensions)}, "int");
	//	//		abcde[0] = abcde[1] = abcde[2] = std::vector<double>(problem.dimensions, 0);
	//	//		abcde[3] = abcde[4] = std::vector<double>(problem.func_pointer->num_special_buses, 0);
	//	//		std::cout << "Please, enter costs a, b and c for each bus:\n";
	//	//		for (int d = 0; d < problem.dimensions; d++)
	//	//		{
	//	//			std::cout << "Bus " << std::to_string(d + 1) << "\n";
	//	//			abcde[0][d] = askValue("Coefficient a:\n", std::vector<double>{0, DBL_MAX});
	//	//			abcde[1][d] = askValue("Coefficient b:\n", std::vector<double>{0, DBL_MAX});
	//	//			abcde[2][d] = askValue("Coefficient c:\n", std::vector<double>{0, DBL_MAX});
	//	//		}
	//	//		std::cout << "Please, enter costs d and e for each of the valve-point buses:\n";
	//	//		for (int d = 0; d < problem.func_pointer->num_special_buses; d++)
	//	//		{
	//	//			std::cout << "Bus " << std::to_string(d + 1) << "\n";
	//	//			abcde[3][d] = askValue("Coefficient d:\n", std::vector<double>{0, DBL_MAX});
	//	//			abcde[4][d] = askValue("Coefficient e:\n", std::vector<double>{0, DBL_MAX});
	//	//		}
	//	//		problem.func_pointer->setCoeffs(abcde, problem.func_pointer->num_special_buses);
	//	//	}
	//
	//	//	problem.func_pointer->min_vector = problem.func_pointer->max_vector = std::vector<double>(problem.dimensions, 0);
	//	//	std::cout << "Please, enter minimum and maximum value for each dimension (" << min_constraint << " <= min < max):\n";
	//
	//	//	for (int d = 0; d < problem.dimensions; d++)
	//	//	{
	//	//		double min = 0;
	//	//		double max = 0;
	//	//		do {
	//	//			std::cout << "Dimension " << std::to_string(d + 1) << "\n";
	//	//			min = askValue("Min:\n", std::vector<double>{min_constraint, DBL_MAX});
	//	//			max = askValue("Max:\n", std::vector<double>{min_constraint, DBL_MAX});
	//	//		} while (max <= min);
	//	//		problem.func_pointer->min_vector[d] = min;
	//	//		problem.func_pointer->max_vector[d] = max;
	//	//	}
	//
	//	//	problem.func_pointer->setMinMax(std::vector<std::vector<double>>{problem.func_pointer->min_vector, problem.func_pointer->max_vector});
	//
	//	//	problem.num_agents = askValue("Please, choose the number of agents between 2 and 2000\n", std::vector<double>{2, 2000}, "int");
	//	//	problem.condition = askCode("Please, choose the type of condition for conversion:\n1: number of iterations reached\n2: difference between iterations\n", std::vector<int>{1, 2});
	//
	//	//	if (problem.condition == 1)		problem.condition_iters = askValue("Please, choose the number of iterations from 1 to 1000\n", std::vector<double>{1, 1000}, "int");
	//	//	else if (problem.condition == 2) problem.condition_diff = askValue("Please, choose the epsilon for difference from 0.0001 to 10\n", std::vector<double>{0.0001, 10});
	//
	//	//	std::cout << "Thank you!\n" + problem.toString();
	//
	//	//	std::string command = "";
	//	//	while (command != "e") {
	//	//		double result = gsa.start(&problem);
	//	//		std::cout << "\n\nThe result: " + std::to_string(result);
	//	//		std::cout << "\n\nIf you wish to exit, press e + enter, if you wish to run the calculation again, press any other letter + enter: ";
	//	//		std::cin >> command;
	//	//	}
	//	//}
}